import { describe, expect, it } from 'vitest';
import { cellAt } from './board';

// 10x10 board drawn on 500 px: intersections every 50 px, the first one at 25 px.
describe('cellAt', () => {
  it('maps a click to the nearest intersection', () => {
    expect(cellAt(25, 25, 500, 10)).toEqual({ row: 0, col: 0 });
    expect(cellAt(240, 70, 500, 10)).toEqual({ row: 1, col: 4 });
    expect(cellAt(499, 499, 500, 10)).toEqual({ row: 9, col: 9 });
  });

  it('ignores clicks outside the grid', () => {
    expect(cellAt(-10, 25, 500, 10)).toBeNull();
    expect(cellAt(25, 530, 500, 10)).toBeNull();
  });
});
