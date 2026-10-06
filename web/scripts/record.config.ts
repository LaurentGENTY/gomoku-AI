import { defineConfig, devices } from '@playwright/test';

const viewport = { width: 600, height: 860 };

export default defineConfig({
  testDir: '.',
  testMatch: 'record.spec.ts',
  timeout: 15 * 60_000,
  workers: 1,
  use: {
    ...devices['Desktop Chrome'],
    baseURL: 'http://localhost:5199',
    viewport,
    video: { mode: 'on', size: viewport },
  },
  webServer: {
    command: 'npx vite --port 5199 --strictPort',
    cwd: '..',
    url: 'http://localhost:5199',
    reuseExistingServer: true,
  },
});
