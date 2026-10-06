import { defineConfig } from 'vitest/config';

export default defineConfig({
  // Relative asset URLs so the build works under /gomoku-AI/ on GitHub Pages.
  base: './',
  worker: { format: 'es' },
  build: {
    rollupOptions: {
      // Node-only branches of the Emscripten loader; never executed in the browser.
      external: ['module', 'fs', 'path', 'url', 'node:module', 'node:fs', 'node:path', 'node:url'],
    },
  },
  test: {
    include: ['src/**/*.test.ts', 'test/**/*.test.ts'],
    testTimeout: 30_000,
  },
});
