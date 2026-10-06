import type { MatchState } from '../game/match';
import { BLACK, type Color } from '../game/types';

const name = (color: Color) => (color === BLACK ? 'Black' : 'White');

export function statusText(state: MatchState): string {
  switch (state.phase) {
    case 'idle':
      return 'Loading…';
    case 'humanTurn':
      return `Your turn (${name(state.color)})`;
    case 'aiThinking':
      return `AI is thinking… (${name(state.color)})`;
    case 'over':
      return state.winner === null ? 'Draw — the board is full' : `${name(state.winner)} wins`;
    case 'error':
      return state.message;
  }
}
