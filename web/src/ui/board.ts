import { BLACK, type Cell, type Move } from '../game/types';

const COLORS = {
  board: '#dcb35c',
  line: '#6b4f1d',
  black: '#141414',
  white: '#f4f1ea',
  whiteEdge: '#8a8170',
  lastMove: '#e0452b',
  winLine: 'rgba(224, 69, 43, 0.85)',
};

export interface BoardView {
  render(moves: Move[], winLine: Cell[]): void;
  onCellClick(handler: (cell: Cell) => void): void;
  /** Viewport coordinates of an intersection (used by the Playwright scripts). */
  cellCenter(cell: Cell): { x: number; y: number };
}

/** Stones sit on intersections; the first one is half a step from the edge. */
export function cellAt(x: number, y: number, width: number, size: number): Cell | null {
  const step = width / size;
  const col = Math.round((x - step / 2) / step);
  const row = Math.round((y - step / 2) / step);
  if (row < 0 || col < 0 || row >= size || col >= size) return null;
  return { row, col };
}

export function createBoardView(canvas: HTMLCanvasElement, size: number): BoardView {
  const ctx = canvas.getContext('2d')!;
  let current: { moves: Move[]; line: Cell[] } = { moves: [], line: [] };

  const step = () => canvas.clientWidth / size;
  const toPixel = (index: number) => step() / 2 + index * step();

  function draw(): void {
    const width = canvas.clientWidth;
    const dpr = window.devicePixelRatio || 1;
    if (canvas.width !== Math.round(width * dpr)) {
      canvas.width = Math.round(width * dpr);
      canvas.height = Math.round(width * dpr);
    }
    ctx.setTransform(dpr, 0, 0, dpr, 0, 0);
    ctx.fillStyle = COLORS.board;
    ctx.fillRect(0, 0, width, width);

    ctx.strokeStyle = COLORS.line;
    ctx.lineWidth = 1;
    for (let i = 0; i < size; i++) {
      ctx.beginPath();
      ctx.moveTo(toPixel(0), toPixel(i));
      ctx.lineTo(toPixel(size - 1), toPixel(i));
      ctx.moveTo(toPixel(i), toPixel(0));
      ctx.lineTo(toPixel(i), toPixel(size - 1));
      ctx.stroke();
    }

    const radius = step() * 0.42;
    for (const move of current.moves) {
      ctx.beginPath();
      ctx.arc(toPixel(move.col), toPixel(move.row), radius, 0, Math.PI * 2);
      ctx.fillStyle = move.color === BLACK ? COLORS.black : COLORS.white;
      ctx.fill();
      if (move.color !== BLACK) {
        ctx.strokeStyle = COLORS.whiteEdge;
        ctx.stroke();
      }
    }

    const last = current.moves.at(-1);
    if (last) {
      ctx.beginPath();
      ctx.arc(toPixel(last.col), toPixel(last.row), radius * 0.28, 0, Math.PI * 2);
      ctx.fillStyle = COLORS.lastMove;
      ctx.fill();
    }

    if (current.line.length >= 2) {
      const sorted = [...current.line].sort((a, b) => a.row - b.row || a.col - b.col);
      const [first, end] = [sorted[0], sorted[sorted.length - 1]];
      ctx.beginPath();
      ctx.moveTo(toPixel(first.col), toPixel(first.row));
      ctx.lineTo(toPixel(end.col), toPixel(end.row));
      ctx.strokeStyle = COLORS.winLine;
      ctx.lineWidth = step() * 0.18;
      ctx.lineCap = 'round';
      ctx.stroke();
    }
  }

  window.addEventListener('resize', draw);

  return {
    render(moves, line) {
      current = { moves, line };
      draw();
    },
    onCellClick(handler) {
      canvas.addEventListener('click', (event) => {
        const rect = canvas.getBoundingClientRect();
        const cell = cellAt(event.clientX - rect.left, event.clientY - rect.top, rect.width, size);
        if (cell) handler(cell);
      });
    },
    cellCenter(cell) {
      const rect = canvas.getBoundingClientRect();
      return { x: rect.left + toPixel(cell.col), y: rect.top + toPixel(cell.row) };
    },
  };
}
