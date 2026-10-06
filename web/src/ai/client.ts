import type { AiMove, AiPlayer, Color, Level, Move } from '../game/types';
import type { WorkerRequest, WorkerResponse } from './protocol';

interface Pending {
  resolve(response: WorkerResponse): void;
  reject(err: Error): void;
}

/** AiPlayer backed by a dedicated Web Worker; terminate() is the only way to stop a search. */
export class WorkerAi implements AiPlayer {
  private readonly worker: Worker;
  private pending: Pending | null = null;

  constructor(private readonly level: Level) {
    this.worker = new Worker(new URL('./ai.worker.ts', import.meta.url), { type: 'module' });
    this.worker.onmessage = (event: MessageEvent<WorkerResponse>) => this.settle(event.data);
    this.worker.onerror = (event) => this.fail(new Error(event.message || 'AI worker crashed'));
  }

  async init(size: number, color: Color): Promise<void> {
    await this.request({ type: 'init', level: this.level, size, color });
  }

  async play(moves: Move[]): Promise<AiMove> {
    const response = await this.request({ type: 'play', moves });
    if (response.type !== 'move') throw new Error(`Unexpected AI response: ${response.type}`);
    return { row: response.row, col: response.col, ms: response.ms };
  }

  dispose(): void {
    this.worker.terminate();
    this.fail(new Error('AI disposed'));
  }

  private request(message: WorkerRequest): Promise<WorkerResponse> {
    if (this.pending) return Promise.reject(new Error('AI is busy'));
    return new Promise((resolve, reject) => {
      this.pending = { resolve, reject };
      this.worker.postMessage(message);
    });
  }

  private settle(response: WorkerResponse): void {
    const pending = this.pending;
    this.pending = null;
    if (!pending) return;
    if (response.type === 'error') pending.reject(new Error(response.message));
    else pending.resolve(response);
  }

  private fail(err: Error): void {
    const pending = this.pending;
    this.pending = null;
    pending?.reject(err);
  }
}
