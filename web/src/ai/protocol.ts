import type { Color, Level, Move } from '../game/types';

export type WorkerRequest =
  | { type: 'init'; level: Level; size: number; color: Color }
  | { type: 'play'; moves: Move[] };

export type WorkerResponse =
  | { type: 'ready' }
  | { type: 'move'; row: number; col: number; ms: number }
  | { type: 'error'; message: string };

export const LEVELS: Record<Level, { module: 'player41' | 'player44'; depth: number }> = {
  easy: { module: 'player41', depth: 0 },
  medium: { module: 'player44', depth: 2 },
  hard: { module: 'player44', depth: 4 },
};
