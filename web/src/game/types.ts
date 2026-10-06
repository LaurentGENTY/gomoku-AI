/** Stone colors, matching `enum color_t` in src/server/move.h. */
export type Color = 0 | 1;
export const BLACK: Color = 0;
export const WHITE: Color = 1;
export const other = (color: Color): Color => (color === BLACK ? WHITE : BLACK);

export interface Cell {
  row: number;
  col: number;
}

export interface Move extends Cell {
  color: Color;
}

export type Level = 'easy' | 'medium' | 'hard';

export type PlayResult = 'invalid' | 'ok' | 'won' | 'full';

/** Game rules, implemented in C (referee.wasm). */
export interface Referee {
  reset(size: number): void;
  play(row: number, col: number, color: Color): PlayResult;
  winningLine(): Cell[];
}

export interface AiMove extends Cell {
  ms: number;
}

export interface AiPlayer {
  init(size: number, color: Color): Promise<void>;
  /** `moves`: every move this AI has not seen yet, its own previous move included. */
  play(moves: Move[]): Promise<AiMove>;
  dispose(): void;
}

export type Side = { kind: 'human' } | { kind: 'ai'; level: Level };
