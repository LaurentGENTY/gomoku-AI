import { describe, expect, it } from 'vitest';
import { settingsFromQuery, sidesFor } from './controls';
import { BLACK, WHITE } from '../game/types';

describe('settingsFromQuery', () => {
  it('uses defaults for an empty or invalid query', () => {
    const defaults = { mode: 'human', humanColor: BLACK, levelA: 'medium', levelB: 'hard' };
    expect(settingsFromQuery('')).toEqual(defaults);
    expect(settingsFromQuery('?mode=chess&a=godlike')).toEqual(defaults);
  });

  it('reads every parameter', () => {
    expect(settingsFromQuery('?mode=aivsai&color=white&a=easy&b=medium')).toEqual({
      mode: 'aivsai', humanColor: WHITE, levelA: 'easy', levelB: 'medium',
    });
  });
});

describe('sidesFor', () => {
  it('puts the human on the chosen color', () => {
    expect(sidesFor({ mode: 'human', humanColor: WHITE, levelA: 'hard', levelB: 'easy' })).toEqual({
      black: { kind: 'ai', level: 'hard' }, white: { kind: 'human' },
    });
  });

  it('uses both levels in AI vs AI', () => {
    expect(sidesFor({ mode: 'aivsai', humanColor: BLACK, levelA: 'easy', levelB: 'hard' })).toEqual({
      black: { kind: 'ai', level: 'easy' }, white: { kind: 'ai', level: 'hard' },
    });
  });
});
