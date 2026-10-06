import { defineConfig, devices } from '@playwright/test';

export default defineConfig({
  testDir: 'e2e',
  timeout: 60_000,
  projects: [
    { name: 'dev', use: { ...devices['Desktop Chrome'], baseURL: 'http://localhost:5199/' } },
    // The deployed build, served under the same sub-path as GitHub Pages.
    { name: 'prod', use: { ...devices['Desktop Chrome'], baseURL: 'http://localhost:5197/gomoku-AI/' } },
  ],
  webServer: [
    {
      command: 'npx vite --port 5199 --strictPort',
      url: 'http://localhost:5199',
      reuseExistingServer: !process.env.CI,
    },
    {
      command: 'npx vite preview --port 5197 --strictPort --base /gomoku-AI/',
      url: 'http://localhost:5197/gomoku-AI/',
      reuseExistingServer: !process.env.CI,
    },
  ],
});
