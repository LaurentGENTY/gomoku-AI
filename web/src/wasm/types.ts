/** Exports of referee.wasm (src/wasm/referee_api.c). */
export interface RefereeModule {
  _ref_new(size: number): number;
  _ref_free(): void;
  /** 0 invalid, 1 ok, 2 won, 3 full */
  _ref_play(row: number, col: number, color: number): number;
  _ref_win_count(): number;
  _ref_win_cell(index: number): number;
}

/** Exports of player41.wasm / player44.wasm (src/wasm/player_api.c). */
export interface PlayerModule {
  _ai_init(size: number, color: number, depth: number): void;
  _ai_push(row: number, col: number, color: number): void;
  /** @returns row * size + col */
  _ai_play(): number;
  _ai_finalize(): void;
}

export interface ModuleOptions {
  wasmBinary?: Uint8Array;
}
