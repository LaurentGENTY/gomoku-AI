import createReferee from '../wasm/generated/referee.js';
import type { ModuleOptions, RefereeModule } from '../wasm/types';
import type { Cell, Color, PlayResult, Referee } from './types';

const RESULTS: readonly PlayResult[] = ['invalid', 'ok', 'won', 'full'];

/** Typed view of referee.wasm. */
export class WasmReferee implements Referee {
  private size = 0;

  private constructor(private readonly mod: RefereeModule) {}

  static async load(options?: ModuleOptions): Promise<WasmReferee> {
    return new WasmReferee(await createReferee(options));
  }

  reset(size: number): void {
    if (!this.mod._ref_new(size)) throw new Error(`Unsupported board size ${size}`);
    this.size = size;
  }

  play(row: number, col: number, color: Color): PlayResult {
    return RESULTS[this.mod._ref_play(row, col, color)] ?? 'invalid';
  }

  winningLine(): Cell[] {
    const count = this.mod._ref_win_count();
    return Array.from({ length: count }, (_, i) => {
      const index = this.mod._ref_win_cell(i);
      return { row: Math.floor(index / this.size), col: index % this.size };
    });
  }
}
