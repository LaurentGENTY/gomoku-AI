import {
  BLACK, WHITE, other,
  type AiMove, type AiPlayer, type Cell, type Color, type Level, type Move, type PlayResult, type Referee, type Side,
} from './types';

export type MatchState =
  | { phase: 'idle' }
  | { phase: 'humanTurn'; color: Color }
  | { phase: 'aiThinking'; color: Color }
  | { phase: 'over'; winner: Color | null; line: Cell[] }
  | { phase: 'error'; message: string };

export interface MatchDeps {
  referee: Referee;
  createAi(level: Level): AiPlayer;
  delay(ms: number): Promise<void>;
  /** Minimum time per AI move, so AI-vs-AI games stay watchable. */
  minAiDelayMs: number;
}

interface AiSlot {
  player: AiPlayer;
  /** Number of moves of `Match.moves` already sent to this AI */
  sent: number;
}

/** Turn order of one game. Rules come from the referee; this only sequences turns. */
export class Match {
  moves: Move[] = [];
  state: MatchState = { phase: 'idle' };
  lastAiMs: number | null = null;
  private sides: Record<Color, Side> = { 0: { kind: 'human' }, 1: { kind: 'human' } };
  private readonly ais = new Map<Color, AiSlot>();
  // Bumped on every new game so that answers from a cancelled game are dropped.
  private generation = 0;

  constructor(private readonly deps: MatchDeps, private readonly onChange: () => void) {}

  async start(size: number, black: Side, white: Side): Promise<void> {
    this.disposeAis();
    const generation = ++this.generation;
    this.moves = [];
    this.lastAiMs = null;
    this.sides = { 0: black, 1: white };
    this.deps.referee.reset(size);
    this.setState({ phase: 'idle' });
    try {
      for (const color of [BLACK, WHITE]) {
        const side = this.sides[color];
        if (side.kind === 'ai') {
          const player = this.deps.createAi(side.level);
          this.ais.set(color, { player, sent: 0 });
          await player.init(size, color);
        }
      }
    } catch (err) {
      if (generation === this.generation) this.fail('Could not load the AI', err);
      return;
    }
    if (generation !== this.generation) return;
    this.turn(BLACK);
  }

  humanPlay(row: number, col: number): boolean {
    if (this.state.phase !== 'humanTurn') return false;
    return this.apply(row, col, this.state.color) !== 'invalid';
  }

  dispose(): void {
    this.generation++;
    this.disposeAis();
  }

  private turn(color: Color): void {
    if (this.sides[color].kind === 'human') this.setState({ phase: 'humanTurn', color });
    else void this.aiTurn(color);
  }

  private async aiTurn(color: Color): Promise<void> {
    const generation = this.generation;
    const slot = this.ais.get(color)!;
    // player.h contract: the AI gets every move it has not seen, its own last one included.
    const pending = this.moves.slice(slot.sent);
    slot.sent = this.moves.length;
    this.setState({ phase: 'aiThinking', color });
    let move: AiMove;
    try {
      [move] = await Promise.all([slot.player.play(pending), this.deps.delay(this.deps.minAiDelayMs)]);
    } catch (err) {
      if (generation === this.generation) this.fail('The AI crashed', err);
      return;
    }
    if (generation !== this.generation) return;
    this.lastAiMs = move.ms;
    if (this.apply(move.row, move.col, color) === 'invalid') {
      this.fail('The AI crashed', new Error(`illegal AI move ${move.row},${move.col}`));
    }
  }

  private apply(row: number, col: number, color: Color): PlayResult {
    const result = this.deps.referee.play(row, col, color);
    if (result === 'invalid') return result;
    this.moves.push({ row, col, color });
    if (result === 'won') this.setState({ phase: 'over', winner: color, line: this.deps.referee.winningLine() });
    else if (result === 'full') this.setState({ phase: 'over', winner: null, line: [] });
    else this.turn(other(color));
    return result;
  }

  private fail(message: string, err: unknown): void {
    console.error(message, err);
    this.disposeAis();
    this.setState({ phase: 'error', message });
  }

  private disposeAis(): void {
    for (const slot of this.ais.values()) slot.player.dispose();
    this.ais.clear();
  }

  private setState(state: MatchState): void {
    this.state = state;
    this.onChange();
  }
}
