// One worker = one player instance: the C players keep their state in globals.
import type { PlayerModule } from '../wasm/types';
import { LEVELS, type WorkerRequest, type WorkerResponse } from './protocol';

const scope = self as unknown as {
  postMessage(message: WorkerResponse): void;
  onmessage: ((event: MessageEvent<WorkerRequest>) => void) | null;
};

let player: PlayerModule | null = null;
let size = 0;

async function loadModule(name: 'player41' | 'player44'): Promise<PlayerModule> {
  if (name === 'player41') return (await import('../wasm/generated/player41.js')).default();
  return (await import('../wasm/generated/player44.js')).default();
}

async function handle(request: WorkerRequest): Promise<WorkerResponse> {
  if (request.type === 'init') {
    const level = LEVELS[request.level];
    player = await loadModule(level.module);
    size = request.size;
    player._ai_init(request.size, request.color, level.depth);
    return { type: 'ready' };
  }
  if (!player) throw new Error('AI not initialized');
  for (const move of request.moves) player._ai_push(move.row, move.col, move.color);
  const started = performance.now();
  const index = player._ai_play();
  return { type: 'move', row: Math.floor(index / size), col: index % size, ms: Math.round(performance.now() - started) };
}

scope.onmessage = (event) => {
  handle(event.data).then(
    (response) => scope.postMessage(response),
    (err: unknown) => scope.postMessage({ type: 'error', message: String(err) }),
  );
};
