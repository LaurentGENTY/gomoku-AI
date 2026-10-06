import { readFileSync } from 'node:fs';
import { describe, expect, it } from 'vitest';
import createReferee from '../src/wasm/generated/referee.js';
import createPlayer41 from '../src/wasm/generated/player41.js';
import createPlayer44 from '../src/wasm/generated/player44.js';
import type { ModuleOptions, PlayerModule, RefereeModule } from '../src/wasm/types';

const SIZE = 10;
const BLACK = 0;
const WHITE = 1;
const INVALID = 0;
const OK = 1;
const WON = 2;
const FULL = 3;

type Factory<T> = (options?: ModuleOptions) => Promise<T>;
type Stone = [row: number, col: number, color: number];

// Feed the binary directly: no reliance on the loader finding the file on disk.
const wasmBinary = (name: string) =>
  readFileSync(new URL(`../src/wasm/generated/${name}.wasm`, import.meta.url));

async function newReferee(): Promise<RefereeModule> {
  const ref = await createReferee({ wasmBinary: wasmBinary('referee') });
  expect(ref._ref_new(SIZE)).toBe(1);
  return ref;
}

async function aiMove(
  factory: Factory<PlayerModule>, name: string, depth: number, color: number, stones: Stone[],
): Promise<number> {
  const ai = await factory({ wasmBinary: wasmBinary(name) });
  ai._ai_init(SIZE, color, depth);
  for (const [row, col, c] of stones) ai._ai_push(row, col, c);
  const index = ai._ai_play();
  ai._ai_finalize();
  return index;
}

describe('referee.wasm', () => {
  it('rejects occupied and out-of-board cells', async () => {
    const ref = await newReferee();
    expect(ref._ref_play(4, 4, BLACK)).toBe(OK);
    expect(ref._ref_play(4, 4, WHITE)).toBe(INVALID);
    expect(ref._ref_play(-1, 0, WHITE)).toBe(INVALID);
    expect(ref._ref_play(0, SIZE, WHITE)).toBe(INVALID);
  });

  const lines: Array<[name: string, dr: number, dc: number]> = [
    ['row', 0, 1],
    ['column', 1, 0],
    ['diagonal', 1, 1],
    ['anti-diagonal', 1, -1],
  ];
  it.each(lines)('detects a five in a %s and reports its cells', async (_name, dr, dc) => {
    const ref = await newReferee();
    const whiteReplies = [[9, 0], [9, 1], [9, 2], [9, 3]];
    for (let k = 0; k < 4; k++) {
      expect(ref._ref_play(2 + k * dr, 5 + k * dc, BLACK)).toBe(OK);
      expect(ref._ref_play(whiteReplies[k][0], whiteReplies[k][1], WHITE)).toBe(OK);
    }
    expect(ref._ref_play(2 + 4 * dr, 5 + 4 * dc, BLACK)).toBe(WON);
    const cells = Array.from({ length: ref._ref_win_count() }, (_, i) => ref._ref_win_cell(i));
    const expected = Array.from({ length: 5 }, (_, k) => (2 + k * dr) * SIZE + 5 + k * dc);
    expect([...cells].sort((a, b) => a - b)).toEqual([...expected].sort((a, b) => a - b));
  });

  it('reports a full board without a five as full', async () => {
    const ref = await newReferee();
    // (col + floor(row / 2)) % 2 fills a 10x10 board without any five in a row.
    for (let row = 0; row < SIZE; row++) {
      for (let col = 0; col < SIZE; col++) {
        const last = row === SIZE - 1 && col === SIZE - 1;
        expect(ref._ref_play(row, col, (col + Math.floor(row / 2)) % 2)).toBe(last ? FULL : OK);
      }
    }
  });
});

describe('player modules', () => {
  const midGame: Stone[] = [[4, 4, BLACK], [5, 5, WHITE], [4, 5, BLACK], [3, 3, WHITE], [5, 4, BLACK], [6, 6, WHITE]];

  it.each([
    ['player41', createPlayer41, 0],
    ['player44', createPlayer44, 2],
    ['player44', createPlayer44, 4],
  ] as const)('%s (depth %i) returns a legal move in under 10 s', async (name, factory, depth) => {
    const started = performance.now();
    const index = await aiMove(factory, name, depth, BLACK, midGame);
    expect(performance.now() - started).toBeLessThan(10_000);
    expect(index).toBeGreaterThanOrEqual(0);
    expect(index).toBeLessThan(SIZE * SIZE);
    const occupied = midGame.map(([r, c]) => r * SIZE + c);
    expect(occupied).not.toContain(index);
  });

  it('player44 blocks an open four', async () => {
    const stones: Stone[] = [[4, 2, BLACK], [4, 1, WHITE], [4, 3, BLACK], [0, 0, WHITE], [4, 4, BLACK], [9, 9, WHITE], [4, 5, BLACK]];
    expect(await aiMove(createPlayer44, 'player44', 4, WHITE, stones)).toBe(4 * SIZE + 6);
  });

  it('player44 completes its own five', async () => {
    const stones: Stone[] = [[4, 2, BLACK], [0, 0, WHITE], [4, 3, BLACK], [0, 9, WHITE], [4, 4, BLACK], [9, 0, WHITE], [4, 5, BLACK], [9, 9, WHITE]];
    expect([4 * SIZE + 1, 4 * SIZE + 6]).toContain(await aiMove(createPlayer44, 'player44', 4, BLACK, stones));
  });
});
