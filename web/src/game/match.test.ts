import { describe, expect, it, vi } from 'vitest';
import { Match, type MatchDeps } from './match';
import { BLACK, WHITE, type AiMove, type AiPlayer, type Cell, type Move, type PlayResult, type Referee } from './types';

class FakeReferee implements Referee {
  occupied = new Set<string>();
  /** "row,col" of the move that completes a five */
  winOn: string | null = null;
  fullAfter = Infinity;
  reset(): void {
    this.occupied.clear();
  }
  play(row: number, col: number): PlayResult {
    const key = `${row},${col}`;
    if (this.occupied.has(key) || row < 0 || col < 0 || row >= 10 || col >= 10) return 'invalid';
    this.occupied.add(key);
    if (key === this.winOn) return 'won';
    if (this.occupied.size >= this.fullAfter) return 'full';
    return 'ok';
  }
  winningLine(): Cell[] {
    return [{ row: 0, col: 0 }];
  }
}

class FakeAi implements AiPlayer {
  received: Move[][] = [];
  disposed = false;
  constructor(private readonly script: Array<AiMove | Promise<AiMove>> = []) {}
  async init(): Promise<void> {}
  play(moves: Move[]): Promise<AiMove> {
    this.received.push(moves);
    const next = this.script.shift();
    return next ? Promise.resolve(next) : Promise.reject(new Error('script exhausted'));
  }
  dispose(): void {
    this.disposed = true;
  }
}

class FailingAi extends FakeAi {
  async init(): Promise<void> {
    throw new Error('wasm failed to load');
  }
}

const at = (row: number, col: number): AiMove => ({ row, col, ms: 1 });
const flush = () => new Promise((resolve) => setTimeout(resolve, 0));
const human = { kind: 'human' } as const;
const ai = { kind: 'ai', level: 'easy' } as const;

function setup(ais: FakeAi[]) {
  const referee = new FakeReferee();
  const queue = [...ais];
  const deps: MatchDeps = {
    referee,
    createAi: () => queue.shift()!,
    delay: () => Promise.resolve(),
    minAiDelayMs: 0,
  };
  return { match: new Match(deps, () => {}), referee };
}

describe('Match', () => {
  it('starts on the human turn when the human plays black', async () => {
    const { match } = setup([new FakeAi()]);
    await match.start(10, human, ai);
    expect(match.state).toEqual({ phase: 'humanTurn', color: BLACK });
  });

  it('sends the AI every move it has not seen, including its own last move', async () => {
    const bot = new FakeAi([at(5, 5), at(6, 6)]);
    const { match } = setup([bot]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await flush();
    match.humanPlay(4, 5);
    await flush();
    expect(bot.received).toEqual([
      [{ row: 4, col: 4, color: BLACK }],
      [{ row: 5, col: 5, color: WHITE }, { row: 4, col: 5, color: BLACK }],
    ]);
    expect(match.moves).toHaveLength(4);
    expect(match.state).toEqual({ phase: 'humanTurn', color: BLACK });
  });

  it('lets the AI open as black', async () => {
    const bot = new FakeAi([at(4, 4)]);
    const { match } = setup([bot]);
    await match.start(10, ai, human);
    await flush();
    expect(bot.received).toEqual([[]]);
    expect(match.state).toEqual({ phase: 'humanTurn', color: WHITE });
  });

  it('rejects a move on an occupied cell', async () => {
    const { match } = setup([new FakeAi([at(5, 5)])]);
    await match.start(10, human, ai);
    expect(match.humanPlay(4, 4)).toBe(true);
    await flush();
    expect(match.humanPlay(5, 5)).toBe(false);
    expect(match.state).toEqual({ phase: 'humanTurn', color: BLACK });
  });

  it('ignores clicks while the AI is thinking', async () => {
    const { match } = setup([new FakeAi([new Promise<AiMove>(() => {})])]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    expect(match.state.phase).toBe('aiThinking');
    expect(match.humanPlay(0, 0)).toBe(false);
    expect(match.moves).toHaveLength(1);
  });

  it('ends the game on a win', async () => {
    const { match, referee } = setup([new FakeAi()]);
    referee.winOn = '0,4';
    await match.start(10, human, ai);
    match.humanPlay(0, 4);
    expect(match.state).toEqual({ phase: 'over', winner: BLACK, line: [{ row: 0, col: 0 }] });
  });

  it('ends the game in a draw when the board is full', async () => {
    const { match, referee } = setup([new FakeAi([at(5, 5)])]);
    referee.fullAfter = 2;
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await flush();
    expect(match.state).toEqual({ phase: 'over', winner: null, line: [] });
  });

  it('plays AI against AI until the end', async () => {
    const blackAi = new FakeAi([at(0, 0), at(0, 1), at(0, 2)]);
    const whiteAi = new FakeAi([at(9, 0), at(9, 1)]);
    const { match, referee } = setup([blackAi, whiteAi]);
    referee.winOn = '0,2';
    await match.start(10, ai, { kind: 'ai', level: 'hard' });
    await vi.waitFor(() => expect(match.state.phase).toBe('over'));
    expect(match.state).toMatchObject({ phase: 'over', winner: BLACK });
    expect(match.moves).toHaveLength(5);
  });

  it('reports a crash when the AI throws', async () => {
    vi.spyOn(console, 'error').mockImplementation(() => {});
    const bot = new FakeAi([]);
    const { match } = setup([bot]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await flush();
    expect(match.state).toEqual({ phase: 'error', message: 'The AI crashed' });
    expect(bot.disposed).toBe(true);
  });

  it('reports a crash when the AI plays an illegal move', async () => {
    vi.spyOn(console, 'error').mockImplementation(() => {});
    const { match } = setup([new FakeAi([at(4, 4)])]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await flush();
    expect(match.state).toEqual({ phase: 'error', message: 'The AI crashed' });
  });

  it('reports an AI that fails to load', async () => {
    vi.spyOn(console, 'error').mockImplementation(() => {});
    const { match } = setup([new FailingAi()]);
    await match.start(10, human, ai);
    expect(match.state).toEqual({ phase: 'error', message: 'Could not load the AI' });
  });

  it('ignores a move from a cancelled game', async () => {
    let resolveOld!: (move: AiMove) => void;
    const oldAi = new FakeAi([new Promise<AiMove>((resolve) => (resolveOld = resolve))]);
    const { match } = setup([oldAi, new FakeAi()]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await match.start(10, human, ai);
    expect(oldAi.disposed).toBe(true);
    resolveOld(at(5, 5));
    await flush();
    expect(match.moves).toEqual([]);
    expect(match.state).toEqual({ phase: 'humanTurn', color: BLACK });
  });
});
