import { expect, test } from '@playwright/test';

test('a human move gets an AI answer', async ({ page }) => {
  await page.goto('/?mode=human&color=black&a=easy');
  await page.waitForFunction(() => window.__gomoku?.state().phase === 'humanTurn');
  const point = await page.evaluate(() => window.__gomoku!.cellCenter({ row: 4, col: 4 }));
  await page.mouse.click(point.x, point.y);
  await page.waitForFunction(
    () => window.__gomoku!.moves().length === 2 && window.__gomoku!.state().phase === 'humanTurn',
    null,
    { timeout: 30_000 },
  );
  await expect(page.locator('#status')).toHaveText('Your turn (Black)');
  await expect(page.locator('#timing')).toContainText('Last AI move:');
});
