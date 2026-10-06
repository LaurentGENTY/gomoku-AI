import { readFileSync } from 'node:fs';
import { expect, it } from 'vitest';
import { WasmReferee } from '../src/game/referee';
import { BLACK, WHITE } from '../src/game/types';

const wasmBinary = readFileSync(new URL('../src/wasm/generated/referee.wasm', import.meta.url));

it('maps referee codes and the winning line to typed values', async () => {
  const referee = await WasmReferee.load({ wasmBinary });
  referee.reset(10);
  expect(referee.play(4, 4, BLACK)).toBe('ok');
  expect(referee.play(4, 4, WHITE)).toBe('invalid');
  for (let col = 5; col < 8; col++) referee.play(4, col, BLACK);
  expect(referee.play(4, 8, BLACK)).toBe('won');
  expect(referee.winningLine()).toEqual([4, 5, 6, 7, 8].map((col) => ({ row: 4, col })));
  referee.reset(10);
  expect(referee.play(4, 4, WHITE)).toBe('ok');
});
