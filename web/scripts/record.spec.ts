import { mkdirSync, writeFileSync } from 'node:fs';
import path from 'node:path';
import { test, type Page } from '@playwright/test';

// Run from web/: raw videos go to ../docs/media/raw (git-ignored).
const RAW_DIR = path.resolve('..', 'docs', 'media', 'raw');

// Human moves tried in order; cells already taken are skipped.
const HUMAN_SCRIPT: Array<[number, number]> = [
  [4, 4], [4, 5], [5, 4], [3, 3], [5, 5], [3, 5], [6, 3], [2, 6],
  [6, 6], [2, 2], [5, 3], [3, 4], [6, 4], [4, 2], [7, 2], [7, 7],
];

async function waitForPhase(page: Page, phases: string[], timeout = 120_000): Promise<string> {
  const handle = await page.waitForFunction(
    (wanted) => {
      const phase = window.__gomoku?.state().phase ?? '';
      return wanted.includes(phase) ? phase : null;
    },
    phases,
    { timeout },
  );
  return (await handle.jsonValue()) as string;
}

/** Plays the scripted human moves; returns the AI move durations seen. */
async function playScript(page: Page, maxMoves: number): Promise<number[]> {
  const aiTimes: number[] = [];
  let played = 0;
  for (const [row, col] of HUMAN_SCRIPT) {
    if (played >= maxMoves) break;
    if ((await waitForPhase(page, ['humanTurn', 'over', 'error'])) !== 'humanTurn') break;
    const ms = await page.evaluate(() => window.__gomoku!.lastAiMs());
    if (ms !== null) aiTimes.push(ms);
    const taken = await page.evaluate(
      ([r, c]) => window.__gomoku!.moves().some((m) => m.row === r && m.col === c),
      [row, col],
    );
    if (taken) continue;
    await page.waitForTimeout(600);
    const point = await page.evaluate((cell) => window.__gomoku!.cellCenter(cell), { row, col });
    await page.mouse.click(point.x, point.y);
    played++;
  }
  return aiTimes;
}

async function saveVideo(page: Page, name: string): Promise<void> {
  const video = page.video()!;
  await page.close();
  mkdirSync(RAW_DIR, { recursive: true });
  await video.saveAs(path.join(RAW_DIR, `${name}.webm`));
}

test('human-vs-hard', async ({ page }) => {
  await page.goto('/?mode=human&color=black&a=hard');
  const aiTimes = await playScript(page, HUMAN_SCRIPT.length);
  await waitForPhase(page, ['humanTurn', 'over', 'error']);
  await page.waitForTimeout(2000);
  mkdirSync(RAW_DIR, { recursive: true });
  writeFileSync(path.join(RAW_DIR, 'hard-timings.json'), JSON.stringify({ maxMs: Math.max(...aiTimes), aiTimes }, null, 2));
  await saveVideo(page, 'human-vs-hard');
});

test('ai-vs-ai', async ({ page }) => {
  await page.goto('/?mode=aivsai&a=easy&b=hard');
  await waitForPhase(page, ['over', 'error'], 10 * 60_000);
  await page.waitForTimeout(2000);
  await saveVideo(page, 'ai-vs-ai');
});

test('how-it-thinks', async ({ page }) => {
  await page.goto('/?mode=human&color=black&a=hard');
  await playScript(page, 3);
  await waitForPhase(page, ['humanTurn', 'over', 'error']);
  await page.waitForTimeout(1500);
  await saveVideo(page, 'how-it-thinks');
});
