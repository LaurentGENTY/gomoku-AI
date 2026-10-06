import './style.css';
import { WorkerAi } from './ai/client';
import { Match, type MatchState } from './game/match';
import { WasmReferee } from './game/referee';
import type { Cell, Move } from './game/types';
import { createBoardView } from './ui/board';
import { bindControls, settingsFromQuery, sidesFor } from './ui/controls';
import { statusText } from './ui/status';

const SIZE = 10;
// Keeps AI-vs-AI games watchable (and recordable) when the AI answers instantly.
const MIN_AI_DELAY_MS = 400;

declare global {
  interface Window {
    __gomoku?: {
      state(): MatchState;
      moves(): Move[];
      lastAiMs(): number | null;
      cellCenter(cell: Cell): { x: number; y: number };
    };
  }
}

async function main(): Promise<void> {
  const canvas = document.querySelector<HTMLCanvasElement>('#board')!;
  const status = document.querySelector<HTMLElement>('#status')!;
  const timing = document.querySelector<HTMLElement>('#timing')!;

  let referee: WasmReferee;
  try {
    referee = await WasmReferee.load();
  } catch (err) {
    console.error(err);
    document.querySelector('#board-area')!.textContent =
      'Could not load the game engine (WebAssembly). Please try a recent browser.';
    return;
  }

  const view = createBoardView(canvas, SIZE);
  const match = new Match(
    {
      referee,
      createAi: (level) => new WorkerAi(level),
      delay: (ms) => new Promise((resolve) => setTimeout(resolve, ms)),
      minAiDelayMs: MIN_AI_DELAY_MS,
    },
    render,
  );

  function render(): void {
    const state = match.state;
    view.render(match.moves, state.phase === 'over' ? state.line : []);
    status.textContent = statusText(state);
    timing.textContent = match.lastAiMs === null ? '' : `Last AI move: ${match.lastAiMs} ms`;
    canvas.classList.toggle('waiting', state.phase !== 'humanTurn');
  }

  const controls = bindControls(document.querySelector<HTMLElement>('#controls')!, settingsFromQuery(location.search));
  const newGame = () => {
    const { black, white } = sidesFor(controls.read());
    void match.start(SIZE, black, white);
  };
  controls.onNewGame(newGame);
  view.onCellClick(({ row, col }) => match.humanPlay(row, col));

  window.__gomoku = {
    state: () => match.state,
    moves: () => match.moves,
    lastAiMs: () => match.lastAiMs,
    cellCenter: (cell) => view.cellCenter(cell),
  };
  newGame();
}

void main();
