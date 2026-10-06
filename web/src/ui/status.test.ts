import { expect, it } from 'vitest';
import { statusText } from './status';
import { BLACK, WHITE } from '../game/types';

it('describes every phase', () => {
  expect(statusText({ phase: 'idle' })).toBe('Loading…');
  expect(statusText({ phase: 'humanTurn', color: BLACK })).toBe('Your turn (Black)');
  expect(statusText({ phase: 'aiThinking', color: WHITE })).toBe('AI is thinking… (White)');
  expect(statusText({ phase: 'over', winner: WHITE, line: [] })).toBe('White wins');
  expect(statusText({ phase: 'over', winner: null, line: [] })).toBe('Draw — the board is full');
  expect(statusText({ phase: 'error', message: 'The AI crashed' })).toBe('The AI crashed');
});
