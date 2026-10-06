# Gomoku AI — WASM port + web UI Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Rebuild the missing C server layer of the semester-6 Gomoku school project, compile the original C AI to WebAssembly, ship a small Vite + TypeScript web app to play against it, and record GIFs/MP4s for the README and portfolio.

**Architecture:** The C code stays the source of truth: a reconstructed bitboard (`src/server/`) backs both the original players and a `referee.wasm` module; players 4.1 and 4.4 are compiled unchanged (except a thread switch and a depth cap in 4.4) into `player41.wasm` / `player44.wasm`, each AI instance living in its own Web Worker. TypeScript only orchestrates turns (`Match` state machine), draws the board on a canvas and drives Playwright recordings.

**Tech Stack:** C99 (gcc/clang, pthreads natively), Emscripten (`emcc`, latest), Node 22, Vite, TypeScript (strict), Vitest, Playwright (Chromium), ffmpeg, GitHub Actions + GitHub Pages.

**Spec:** `docs/superpowers/specs/2026-10-06-gomoku-wasm-web-design.md`

## Global Constraints

- All code comments, identifiers, commit messages and UI copy are in **English**; comment only the *why*.
- C code is **C99** and must build with both `gcc` (CI, Linux) and Apple `clang` (local, macOS).
- Rules and AI stay in C. **No game rule (validity, win, full board) is implemented in TypeScript** — TS asks `referee.wasm`.
- The only edits allowed in the original player code are in `src/players/player4.4.c`: the `GOMOKU_NO_THREADS` switch and `set_max_depth()`. Every other file under `src/players/` stays byte-identical.
- Bitboard limit: board side ≤ 11 (`BOARD_MAX_SIZE`). The web app uses a fixed **10×10** board.
- Colors: `BLACK = 0`, `WHITE = 1` everywhere (C enum, WASM API, TS `Color`). Black moves first.
- Levels: Easy = `player41` (depth ignored), Medium = `player44` depth 2, Hard = `player44` depth 4.
- `play()` contract (from `src/players/player.h`): an AI receives every move it has not seen yet, **including its own previous move**.
- Emscripten flags: `-O3 -std=c99 -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web,worker,node -sALLOW_MEMORY_GROWTH=1 -sSTACK_SIZE=1048576 -sFILESYSTEM=0`; outputs go to `web/src/wasm/generated/` (git-ignored except the hand-written `*.d.ts`).
- AI-vs-AI pacing: minimum **400 ms** per AI move.
- Performance budget: Hard answers in **< 5 s per move** (Chromium on Laurent's Mac). Over budget → stop and report to Laurent (proposed fix: cap `nb_childs` like the depth); do not change the algorithm silently.
- GIFs **< 3 MB** each, stored in `docs/media/`.
- Git: work on the current branch `claude/dreamy-euler-093672` (check with `git branch --show-current` before each commit). **Never push** without Laurent's explicit go. Every commit message ends with `Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>`.
- If tasks are dispatched to sub-agents: write the task list and per-task status to `.claude/pipeline-state.md` first and update it after each task; independently verify each agent's work (re-read files, re-run the task's tests, `git diff`) before marking it done.
- Parallelism: tasks run in order. Task 7 (pure TS) only needs the web scaffold from Task 6 Step 1, so it may run in parallel with the rest of Task 6.

## Review Focus

1. **The AI's own previous move is sent back to it.** If TS only sends the opponent's move, the C player desyncs and eventually plays on an occupied cell. → Task 7 test "sends the AI every move it has not seen, including its own last move".
2. **"New game" while the AI is thinking.** The late answer from the cancelled game must be ignored, not played on the new board. → Task 7 test "ignores a move from a cancelled game".
3. **Full board without a five (draw).** Must end the game, not hang waiting for an AI that has no move. → Task 6 referee test "reports a full board" + Task 7 test "ends the game in a draw".
4. **Clicks that are not on a playable intersection** (canvas margin, outside the grid, during the AI's turn, on an occupied cell) are ignored. → Task 9 `cellAt` tests + Task 7 tests "rejects a move on an occupied cell" / "ignores clicks while the AI is thinking".
5. **The Hard AI blocks an immediate five and takes its own win.** A reconstruction bug in `board__explore_line` would make it look broken in every GIF. → Task 6 tests "player44 blocks an open four" / "player44 completes its own five".

---

## File Structure

```
.gitignore                         new — build outputs, node_modules, generated wasm, raw recordings
Makefile                           rewritten — native tests, players, valgrind, wasm
src/server/move.h                  new — move_t, col_move_t, color_t
src/server/board.h                 new — struct board + board__* API
src/server/bitboard.h              new — board.h + board__select_bit (bit-level helper)
src/server/bitboard.c              new — 128-bit bitboard implementation, win + pattern detection
src/server/moves.h / moves.c       new — 4-slot "last moves" queue (kept for test_moves)
src/players/player4.4.c            modified — GOMOKU_NO_THREADS + set_max_depth
src/test/test_bitboard.c           modified — real explore_line tests, won test re-enabled, exit code
src/test/test_moves.c              modified — exit code
src/test/test_matrix.c             modified — exit code
src/test/test_player.c             modified — exit code
src/test/test_game.c               deleted — tested a legacy array board no player uses
src/test/test_match.c              new — 4.4 vs random, ≥ 9/10 wins
src/test/test_equiv.c              new — prints 4.4 moves on fixed positions (threads vs sequential)
src/wasm/referee_api.c             new — referee exports
src/wasm/player_api.c              new — player exports (shared by player41 / player44)
web/package.json, tsconfig.json, vite.config.ts, playwright.config.ts
web/index.html, web/src/style.css, web/src/main.ts
web/src/wasm/types.ts              RefereeModule / PlayerModule interfaces
web/src/wasm/generated/*.d.ts      committed typings for the emcc outputs
web/src/game/types.ts              Color, Move, Level, Referee, AiPlayer, Side
web/src/game/match.ts (+ .test.ts) turn state machine, no DOM
web/src/game/referee.ts            WasmReferee wrapper
web/src/ai/protocol.ts             worker messages + level table
web/src/ai/ai.worker.ts            one player module per worker
web/src/ai/client.ts               WorkerAi (AiPlayer over postMessage)
web/src/ui/board.ts (+ .test.ts)   canvas rendering + click mapping
web/src/ui/controls.ts (+ .test.ts) settings from query/form → sides
web/src/ui/status.ts (+ .test.ts)  status line text
web/test/wasm.test.ts              Node tests of the three wasm modules
web/test/referee.test.ts           WasmReferee wrapper test
web/e2e/smoke.spec.ts              Playwright smoke test
web/scripts/record.config.ts, record.spec.ts, to-gif.sh
.github/workflows/ci.yml           tests + build + Pages deploy
README.md                          new (replaces README.txt)
docs/media/*.gif, *.mp4            recordings
```

---

### Task 1: Repo hygiene + bitboard core

**Files:**
- Create: `.gitignore`, `Makefile` (replaces the old one), `src/server/move.h`, `src/server/board.h`, `src/server/bitboard.h`, `src/server/bitboard.c`
- Modify: `src/test/test_bitboard.c` (only `main`'s return value in this task)
- Delete from git: `build_Debug/`, `install/gmon.out`, `gmon.out`, root binaries `test_bitboard test_game test_matrix test_moves test_player`, `doc/rapport.aux doc/rapport.lof doc/rapport.log doc/rapport.toc`

**Interfaces:**
- Produces (used by every later C task):
  - `enum color_t { BLACK = 0, WHITE = 1 }`, `struct move_t { size_t row; size_t col; }`, `struct col_move_t { struct move_t m; enum color_t c; }`
  - `struct board { __uint128_t* b_w; __uint128_t* b_b; size_t capacity; size_t size; }` (`capacity = size*size`)
  - `struct board* board__initialize(size_t size)` (NULL if size is 0 or > 11), `void board__free(struct board*)`, `struct board* board__copy(const struct board*)`
  - `int board__is_valid_move(const struct board*, struct move_t)`, `void board__add_move(struct board*, struct move_t, enum color_t)`, `void board__remove_move(struct board*, struct move_t)`, `int board__is_full(const struct board*)`
  - `int board__get_color(const struct board*, int row, int col)` → `-1` empty/off-board, else `BLACK`/`WHITE`
  - `__uint128_t board__select_bit(__uint128_t* b, int pos)` → bit of 1-based cell `pos = size*row + col + 1`
  - Declared now, implemented in Tasks 2–3: `int board__won(const struct board*, struct move_t)`, `void board__explore_line(const struct board*, int row, int col, int dir, int color, int* pattern)`

- [ ] **Step 1: Remove tracked build artefacts and add `.gitignore`**

```bash
git branch --show-current   # expect claude/dreamy-euler-093672
git rm -r -q --cached build_Debug install/gmon.out gmon.out test_bitboard test_game test_matrix test_moves test_player doc/rapport.aux doc/rapport.lof doc/rapport.log doc/rapport.toc
rm -rf build_Debug install gmon.out test_bitboard test_game test_matrix test_moves test_player doc/rapport.aux doc/rapport.lof doc/rapport.log doc/rapport.toc
```

Create `.gitignore`:

```gitignore
# Native build
build/
build_Debug/
install/
gmon.out
/test_*

# Docs build
doc/doxygen/html/
doc/*.aux
doc/*.log
doc/*.toc
doc/*.lof

# Web
web/node_modules/
web/dist/
web/test-results/
web/playwright-report/
web/src/wasm/generated/*
!web/src/wasm/generated/*.d.ts

# Raw recordings (converted GIF/MP4 live in docs/media/)
docs/media/raw/
```

- [ ] **Step 2: Write the headers**

`src/server/move.h`:

```c
/**
 * @file move.h
 * Moves and colors shared by the board, the players and the tests.
 */
#ifndef MOVE_H
#define MOVE_H

#include <stddef.h>

enum color_t { BLACK = 0, WHITE = 1 };

struct move_t {
  size_t row;
  size_t col;
};

struct col_move_t {
  struct move_t m;
  enum color_t c;
};

#endif
```

`src/server/board.h`:

```c
/**
 * @file board.h
 * Bitboard: one 128-bit integer per color, one bit per cell (row-major), so
 * boards are limited to 11x11 = 121 cells. Rebuilt from the project report
 * and from the way the players use it (the original sources were lost).
 */
#ifndef BOARD_H
#define BOARD_H

#include <stddef.h>
#include "move.h"

#define BOARD_MAX_SIZE 11

struct board {
  __uint128_t* b_w;
  __uint128_t* b_b;
  size_t capacity;
  size_t size;
};

struct board* board__initialize(size_t size);
void board__free(struct board* b);
struct board* board__copy(const struct board* b);

int board__is_valid_move(const struct board* b, struct move_t m);
void board__add_move(struct board* b, struct move_t m, enum color_t c);
void board__remove_move(struct board* b, struct move_t m);
int board__is_full(const struct board* b);

/** @return -1 for an empty or off-board cell, BLACK or WHITE otherwise */
int board__get_color(const struct board* b, int row, int col);

/** @return 1 if the stone at m is part of 5 or more aligned stones */
int board__won(const struct board* b, struct move_t m);

/**
 * Pattern that `color` would form by playing the empty cell (row, col), along
 * one direction (0 horizontal, 1 vertical, 2 diagonal, 3 anti-diagonal).
 * Increments at most one entry of pattern[9], indexed as:
 * FIVE, FOUR_OPEN, FOUR_HALF, FOUR_SPACED, THREE_OPEN, THREE_HALF,
 * TWO_OPEN, TWO_HALF, OTHERS. Nothing is incremented when the line can never
 * reach five stones.
 */
void board__explore_line(const struct board* b, int row, int col, int dir, int color, int* pattern);

#endif
```

`src/server/bitboard.h`:

```c
/**
 * @file bitboard.h
 * Bit-level helpers of the bitboard, used by the tests.
 */
#ifndef BITBOARD_H
#define BITBOARD_H

#include "board.h"

/** @return the bit of the 1-based cell index pos (pos = size*row + col + 1) */
__uint128_t board__select_bit(__uint128_t* b, int pos);

#endif
```

- [ ] **Step 3: Write the new Makefile (bitboard test only for now)**

`Makefile`:

```make
# Native build (tests, player libraries) and WebAssembly build (`make wasm`).
CC      ?= cc
CFLAGS  ?= -Wall -std=c99 -O2 -g
LDLIBS  := -lm

BUILD := build
SRV   := src/server
PLY   := src/players
TST   := src/test

HDRS     := $(wildcard $(SRV)/*.h $(PLY)/*.h)
BITBOARD := $(SRV)/bitboard.c
COMMON   := $(PLY)/matrix.c $(PLY)/list.c

TESTS := test_bitboard

.PHONY: all test clean doc

all: test

$(BUILD):
	mkdir -p $@

$(BUILD)/test_bitboard: $(TST)/test_bitboard.c $(BITBOARD) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)

test: $(addprefix $(BUILD)/,$(TESTS))
	@set -e; for t in $(TESTS); do echo "== $$t"; ./$(BUILD)/$$t; done

doc:
	cd doc/doxygen && doxygen Doxyfile

clean:
	rm -rf $(BUILD) doc/doxygen/html
```

- [ ] **Step 4: Make `test_bitboard` fail loudly, then run it to see it fail**

In `src/test/test_bitboard.c`, replace the last line of `main` (`return 0;`) with:

```c
  return (res == tot) ? EXIT_SUCCESS : EXIT_FAILURE;
```

Run: `make test`
Expected: FAIL — link/compile error, `src/server/bitboard.c` does not exist.

- [ ] **Step 5: Implement the bitboard core**

`src/server/bitboard.c`:

```c
/**
 * @file bitboard.c
 * Bitboard implementation of board.h.
 */
#include <stdlib.h>
#include "bitboard.h"

/* Direction vectors for dir = 0 (horizontal), 1 (vertical), 2 (diagonal), 3 (anti-diagonal) */
static const int DR[4] = {0, 1, 1, 1};
static const int DC[4] = {1, 0, 1, -1};

static __uint128_t cell_bit(const struct board* b, size_t row, size_t col){
  return (__uint128_t)1 << (row * b->size + col);
}

static int in_board(const struct board* b, long row, long col){
  return row >= 0 && col >= 0 && row < (long)b->size && col < (long)b->size;
}

__uint128_t board__select_bit(__uint128_t* b, int pos){
  return (*b >> (pos - 1)) & 1;
}

struct board* board__initialize(size_t size){
  if (size == 0 || size > BOARD_MAX_SIZE){
    return NULL;
  }
  struct board* b = malloc(sizeof(*b));
  b->b_w = malloc(sizeof(__uint128_t));
  b->b_b = malloc(sizeof(__uint128_t));
  b->size = size;
  b->capacity = size * size;
  /* Bits past the board are set in both integers so that "full" is a single
     comparison against all-ones. */
  __uint128_t outside = ~(__uint128_t)0 << b->capacity;
  *b->b_w = outside;
  *b->b_b = outside;
  return b;
}

void board__free(struct board* b){
  if (b == NULL){
    return;
  }
  free(b->b_w);
  free(b->b_b);
  free(b);
}

struct board* board__copy(const struct board* b){
  struct board* c = board__initialize(b->size);
  *c->b_w = *b->b_w;
  *c->b_b = *b->b_b;
  return c;
}

int board__get_color(const struct board* b, int row, int col){
  if (!in_board(b, row, col)){
    return -1;
  }
  size_t pos = (size_t)row * b->size + (size_t)col;
  if ((*b->b_b >> pos) & 1){
    return BLACK;
  }
  if ((*b->b_w >> pos) & 1){
    return WHITE;
  }
  return -1;
}

int board__is_valid_move(const struct board* b, struct move_t m){
  if (m.row >= b->size || m.col >= b->size){
    return 0;
  }
  return board__get_color(b, (int)m.row, (int)m.col) == -1;
}

void board__add_move(struct board* b, struct move_t m, enum color_t c){
  __uint128_t* stones = (c == BLACK) ? b->b_b : b->b_w;
  *stones |= cell_bit(b, m.row, m.col);
}

void board__remove_move(struct board* b, struct move_t m){
  __uint128_t keep = ~cell_bit(b, m.row, m.col);
  *b->b_b &= keep;
  *b->b_w &= keep;
}

int board__is_full(const struct board* b){
  return (*b->b_w | *b->b_b) == ~(__uint128_t)0;
}
```

Also add temporary stubs at the end of `bitboard.c` so the file links (they are replaced in Tasks 2 and 3):

```c
int board__won(const struct board* b, struct move_t m){
  (void)b; (void)m;
  return 0;
}

void board__explore_line(const struct board* b, int row, int col, int dir, int color, int* pattern){
  (void)b; (void)row; (void)col; (void)dir; (void)color; (void)pattern;
}
```

- [ ] **Step 6: Run the tests**

Run: `make test`
Expected: builds; `test_bitboard` prints `SUCCESS` for `initialization`, `board__is_full`, `board__is_valid_move`, and `board__explore_line` (its current assertions are vacuous — they compare pointers; Task 3 replaces them). Exit code 0.

- [ ] **Step 7: Commit**

```bash
git add -A .gitignore Makefile src/server src/test/test_bitboard.c
git add -u
git status --short   # only the files above + deletions of the build artefacts
git commit -m "Rebuild bitboard core and clean tracked build artefacts

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 2: Win detection (`board__won`)

**Files:**
- Modify: `src/server/bitboard.c` (replace the `board__won` stub)
- Modify: `src/test/test_bitboard.c` (`main`: re-enable the won test)

**Interfaces:**
- Consumes: Task 1 board API.
- Produces: `int board__won(const struct board* b, struct move_t m)` — 0 for an off-board or empty cell, 1 when the stone at `m` belongs to ≥ 5 aligned stones of its color.

- [ ] **Step 1: Re-enable the existing won test**

In `src/test/test_bitboard.c` `main`, uncomment:

```c
  TEST("board__won",test_board__won(act),1);
```

- [ ] **Step 2: Run it to verify it fails**

Run: `make test`
Expected: FAIL — `Test board__won : FAILURE` (the stub returns 0), non-zero exit.

- [ ] **Step 3: Implement `board__won`**

In `src/server/bitboard.c`, replace the `board__won` stub with:

```c
/* Number of consecutive stones of `color` after (row, col) in direction (dr, dc) */
static int run_length(const struct board* b, int row, int col, int dr, int dc, int color){
  int n = 0;
  row += dr;
  col += dc;
  while (in_board(b, row, col) && board__get_color(b, row, col) == color){
    n++;
    row += dr;
    col += dc;
  }
  return n;
}

int board__won(const struct board* b, struct move_t m){
  if (m.row >= b->size || m.col >= b->size){
    return 0;
  }
  int row = (int)m.row;
  int col = (int)m.col;
  int color = board__get_color(b, row, col);
  if (color == -1){
    return 0;
  }
  for (int d = 0; d < 4; d++){
    int n = 1 + run_length(b, row, col, DR[d], DC[d], color)
              + run_length(b, row, col, -DR[d], -DC[d], color);
    if (n >= 5){
      return 1;
    }
  }
  return 0;
}
```

- [ ] **Step 4: Run the tests**

Run: `make test`
Expected: `board__won : SUCCESS`, exit 0.

- [ ] **Step 5: Commit**

```bash
git add src/server/bitboard.c src/test/test_bitboard.c
git commit -m "Add bitboard win detection

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 3: Pattern detection (`board__explore_line`) + heuristic oracle

**Files:**
- Modify: `src/server/bitboard.c` (replace the `board__explore_line` stub)
- Modify: `src/test/test_bitboard.c` (replace the body of `test_explore__line`)
- Modify: `src/test/test_player.c` (`main` exit code)
- Modify: `Makefile` (add `test_player`)

**Interfaces:**
- Consumes: Task 1 board API.
- Produces: `board__explore_line` as specified in `board.h` and spec §5.1.3. Used unchanged by `heuristic.c`, `heuristic2.c`, `heuristic3.c`.

Rule (spec §5.1.3), for the empty cell (row, col), direction `dir`, looking ±4 cells away:
1. Mark each cell `OWN` (the explored cell itself, or a `color` stone), `EMPTY`, or `BLOCKED` (opponent stone or off-board).
2. The run of non-blocked cells containing the centre is shorter than 5 → increment nothing.
3. `run` = contiguous `OWN` cells through the centre; `open` = how many of its two ends are `EMPTY`.
4. `run ≥ 5` → FIVE; `run == 4` → FOUR_OPEN if `open == 2` else FOUR_HALF.
5. Else, a 5-cell window containing the centre with no `BLOCKED` cell and 4 `OWN` cells → FOUR_SPACED.
6. `run == 3` → THREE_OPEN / THREE_HALF; `run == 2` → TWO_OPEN / TWO_HALF (open = 2 / otherwise); `run == 1` → OTHERS.
7. Occupied centre, off-board centre or `dir` outside 0..3 → increment nothing.

- [ ] **Step 1: Replace the vacuous tests with real ones**

In `src/test/test_bitboard.c`, replace the whole `int test_explore__line(int act){ ... }` function with:

```c
/* Pattern indices of board__explore_line, see board.h */
enum { FIVE = 0, FOUR_OPEN, FOUR_HALF, FOUR_SPACED, THREE_OPEN, THREE_HALF,
       TWO_OPEN, TWO_HALF, OTHERS, NOTHING = -1 };

/* Plays `stones`, explores (row, col, dir) for BLACK and checks that exactly the
   `expected` pattern was incremented (or nothing at all for NOTHING). */
static int explores_to(const struct col_move_t* stones, int n,
                       int row, int col, int dir, int expected){
  struct board* board = board__initialize(SIZE);
  for (int i = 0; i < n; i++){
    board__add_move(board, stones[i].m, stones[i].c);
  }
  int pattern[9] = {0};
  board__explore_line(board, row, col, dir, BLACK, pattern);
  board__free(board);
  int total = 0;
  for (int i = 0; i < 9; i++){
    total += pattern[i];
  }
  if (expected == NOTHING){
    return total == 0;
  }
  return total == 1 && pattern[expected] == 1;
}

int test_explore__line(int act){
  INIT_TEST("-",act);
  const struct col_move_t open_two[] = {{{5,4},BLACK}};
  const struct col_move_t edge_two[] = {{{0,1},BLACK}};
  const struct col_move_t open_three[] = {{{5,3},BLACK},{{5,4},BLACK}};
  const struct col_move_t half_three[] = {{{5,3},BLACK},{{5,4},BLACK},{{5,2},WHITE}};
  const struct col_move_t open_four[] = {{{5,2},BLACK},{{5,3},BLACK},{{5,4},BLACK}};
  const struct col_move_t half_four[] = {{{5,2},BLACK},{{5,3},BLACK},{{5,4},BLACK},{{5,1},WHITE}};
  const struct col_move_t five[] = {{{5,1},BLACK},{{5,2},BLACK},{{5,3},BLACK},{{5,4},BLACK}};
  const struct col_move_t spaced_four[] = {{{5,1},BLACK},{{5,2},BLACK},{{5,4},BLACK}};
  const struct col_move_t blocked[] = {{{0,4},WHITE}};
  const struct col_move_t vertical[] = {{{4,5},BLACK},{{3,5},BLACK}};
  const struct col_move_t diagonal[] = {{{3,3},BLACK},{{4,4},BLACK}};
  const struct col_move_t anti[] = {{{4,6},BLACK}};
  const struct col_move_t anti_half[] = {{{4,6},BLACK},{{3,7},WHITE}};
  const struct col_move_t occupied[] = {{{5,5},BLACK}};
  const struct col_move_t opponent[] = {{{5,4},WHITE}};

  TEST("single stone", explores_to(NULL, 0, 5, 5, 0, OTHERS), act);
  TEST("open two", explores_to(open_two, 1, 5, 5, 0, TWO_OPEN), act);
  TEST("two against the edge is half open", explores_to(edge_two, 1, 0, 0, 0, TWO_HALF), act);
  TEST("open three", explores_to(open_three, 2, 5, 5, 0, THREE_OPEN), act);
  TEST("half three", explores_to(half_three, 3, 5, 5, 0, THREE_HALF), act);
  TEST("open four", explores_to(open_four, 3, 5, 5, 0, FOUR_OPEN), act);
  TEST("half four", explores_to(half_four, 4, 5, 5, 0, FOUR_HALF), act);
  TEST("five", explores_to(five, 4, 5, 5, 0, FIVE), act);
  TEST("spaced four", explores_to(spaced_four, 3, 5, 5, 0, FOUR_SPACED), act);
  TEST("line too short to reach five", explores_to(blocked, 1, 0, 0, 0, NOTHING), act);
  TEST("vertical three", explores_to(vertical, 2, 5, 5, 1, THREE_OPEN), act);
  TEST("diagonal three", explores_to(diagonal, 2, 5, 5, 2, THREE_OPEN), act);
  TEST("anti-diagonal two", explores_to(anti, 1, 5, 5, 3, TWO_OPEN), act);
  TEST("anti-diagonal half two", explores_to(anti_half, 2, 5, 5, 3, TWO_HALF), act);
  TEST("occupied cell", explores_to(occupied, 1, 5, 5, 0, NOTHING), act);
  TEST("opponent stones are not counted", explores_to(opponent, 1, 5, 5, 0, OTHERS), act);
  TEST("invalid direction", explores_to(NULL, 0, 5, 5, 4, NOTHING), act);
  END_TEST("-",act);
  return (res==tot);
}
```

- [ ] **Step 2: Add `test_player` (the heuristic oracle) to the build**

In `src/test/test_player.c`, replace the body of `main` with:

```c
int main(){
  long clk_tck = CLOCKS_PER_SEC;
  clock_t t1, t2;
  t1 = clock();
  int failures = 0;
  failures += print_error(test_explore_lines(), "test evaluate_point");
  failures += print_error(test_generating_moves(), "test generating_moves");
  failures += print_error(test_heuristic_full(), "test heuristic_full");
  t2 = clock();
  printf("Elapsed time : %lf \n",(double)(t2-t1)/(double)clk_tck);
  heuristic_value();
  return failures ? EXIT_FAILURE : EXIT_SUCCESS;
}
```

and add `#include <stdlib.h>` under the existing includes if `EXIT_SUCCESS` is not already visible.

In `Makefile`, change `TESTS := test_bitboard` to `TESTS := test_bitboard test_player` and add:

```make
$(BUILD)/test_player: $(TST)/test_player.c $(PLY)/heuristic2.c $(COMMON) $(BITBOARD) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)
```

- [ ] **Step 3: Run the tests to verify they fail**

Run: `make test`
Expected: FAIL — `test_bitboard` reports FAILURE for every explore_line case except "line too short", "occupied cell" and "invalid direction"; non-zero exit.

- [ ] **Step 4: Implement `board__explore_line`**

In `src/server/bitboard.c`, replace the `board__explore_line` stub with:

```c
enum { P_FIVE = 0, P_FOUR_OPEN, P_FOUR_HALF, P_FOUR_SPACED, P_THREE_OPEN,
       P_THREE_HALF, P_TWO_OPEN, P_TWO_HALF, P_OTHERS };
enum { CELL_OWN, CELL_EMPTY, CELL_BLOCKED };

/* Cells looked at on each side of the explored cell: a five never spans more */
#define REACH 4
#define WINDOW (2 * REACH + 1)

void board__explore_line(const struct board* b, int row, int col, int dir, int color, int* pattern){
  if (dir < 0 || dir > 3 || !in_board(b, row, col) || board__get_color(b, row, col) != -1){
    return;
  }
  int line[WINDOW];
  for (int k = -REACH; k <= REACH; k++){
    int r = row + k * DR[dir];
    int c = col + k * DC[dir];
    if (k == 0){
      line[k + REACH] = CELL_OWN;
    } else if (!in_board(b, r, c)){
      line[k + REACH] = CELL_BLOCKED;
    } else {
      int v = board__get_color(b, r, c);
      line[k + REACH] = (v == -1) ? CELL_EMPTY : (v == color ? CELL_OWN : CELL_BLOCKED);
    }
  }

  /* A line that cannot hold five stones is worthless */
  int lo = REACH, hi = REACH;
  while (lo > 0 && line[lo - 1] != CELL_BLOCKED){ lo--; }
  while (hi < WINDOW - 1 && line[hi + 1] != CELL_BLOCKED){ hi++; }
  if (hi - lo + 1 < 5){
    return;
  }

  int first = REACH, last = REACH;
  while (first > 0 && line[first - 1] == CELL_OWN){ first--; }
  while (last < WINDOW - 1 && line[last + 1] == CELL_OWN){ last++; }
  int run = last - first + 1;
  int open = (first > 0 && line[first - 1] == CELL_EMPTY)
           + (last < WINDOW - 1 && line[last + 1] == CELL_EMPTY);

  if (run >= 5){ pattern[P_FIVE]++; return; }
  if (run == 4){ pattern[open == 2 ? P_FOUR_OPEN : P_FOUR_HALF]++; return; }

  /* Four stones with one gap inside a playable five-cell window */
  for (int s = 0; s <= REACH; s++){
    int own = 0, blocked = 0;
    for (int k = s; k < s + 5; k++){
      if (line[k] == CELL_OWN){ own++; }
      else if (line[k] == CELL_BLOCKED){ blocked = 1; }
    }
    if (!blocked && own == 4){ pattern[P_FOUR_SPACED]++; return; }
  }

  if (run == 3){ pattern[open == 2 ? P_THREE_OPEN : P_THREE_HALF]++; return; }
  if (run == 2){ pattern[open == 2 ? P_TWO_OPEN : P_TWO_HALF]++; return; }
  pattern[P_OTHERS]++;
}
```

- [ ] **Step 5: Run the tests**

Run: `make test`
Expected: `test_bitboard` all SUCCESS; `test_player` prints `test evaluate_point ... SUCCESS`, `test generating_moves ... SUCCESS`, `test heuristic_full ... SUCCESS`; exit 0.
(The rule was prototyped against `test_player.c` before writing this plan: opportunity values `[1,1,1,1]` and the 12 expected generated moves. If `test generating_moves` fails, stop and report — do not tune the rule to the test without telling Laurent.)

- [ ] **Step 6: Commit**

```bash
git add src/server/bitboard.c src/test/test_bitboard.c src/test/test_player.c Makefile
git commit -m "Rebuild explore_line pattern detection with real tests

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 4: Moves queue, remaining suites, player libraries, valgrind

**Files:**
- Create: `src/server/moves.h`, `src/server/moves.c`
- Modify: `src/test/test_moves.c` (`main` exit code), `src/test/test_matrix.c` (`main` exit code), `Makefile`
- Delete: `src/test/test_game.c`

**Interfaces:**
- Consumes: Task 1 board API.
- Produces: `struct moves`, `moves__initialize/free/get_size/last_n/enqueue/add_opening` (only used by `test_moves`); Make targets `players` and `valgrind`.

- [ ] **Step 1: Fix the test exit codes and drop the legacy test**

`src/test/test_moves.c`, last line of `main`: replace `return 0;` with

```c
  return total ? EXIT_FAILURE : EXIT_SUCCESS;
```

`src/test/test_matrix.c`, replace the body of `main` with:

```c
int main(){
  int failures = 0;
  failures += print_error(test_matrix_set_get(), "test matrix set get");
  failures += print_error(test_matrix_get_max(), "test matrix get max");
  failures += print_error(test_matrix_list_max(), "test matrix list max");
  return failures ? 1 : 0;
}
```

```bash
git rm -q src/test/test_game.c
```

- [ ] **Step 2: Add the targets to the Makefile**

Change the `TESTS` line to `TESTS := test_bitboard test_player test_moves test_matrix`, change `.PHONY` to `.PHONY: all test players valgrind clean doc`, change `all: test` to `all: test players`, and add:

```make
$(BUILD)/test_moves: $(TST)/test_moves.c $(SRV)/moves.c $(BITBOARD) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)

$(BUILD)/test_matrix: $(TST)/test_matrix.c $(COMMON) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) $(filter %.c,$^) -o $@ $(LDLIBS)

# The original players, built as in the project (one shared library each).
# Nothing loads them any more; building them checks the sources still compile.
PLAYERS := player4 player4.2 player4.3 player4.4

players: $(PLAYERS:%=$(BUILD)/%.so)

$(BUILD)/%.so: | $(BUILD)
	$(CC) $(CFLAGS) -fPIC -shared -pthread $(filter %.c,$^) -o $@ $(LDLIBS)

$(BUILD)/player4.so: $(PLY)/player4.c $(PLY)/heuristic0.c $(COMMON) $(HDRS)
$(BUILD)/player4.2.so: $(PLY)/player4.2.c $(PLY)/heuristic.c $(COMMON) $(BITBOARD) $(HDRS)
$(BUILD)/player4.3.so: $(PLY)/player4.3.c $(PLY)/heuristic2.c $(COMMON) $(BITBOARD) $(HDRS)
$(BUILD)/player4.4.so: $(PLY)/player4.4.c $(PLY)/heuristic3.c $(COMMON) $(BITBOARD) $(HDRS)

# Memory errors only: the original player code is not leak-free.
VALGRIND_TESTS := test_bitboard test_player test_moves test_matrix

valgrind: $(addprefix $(BUILD)/,$(VALGRIND_TESTS))
	@set -e; for t in $(VALGRIND_TESTS); do echo "== valgrind $$t"; \
	  valgrind --quiet --error-exitcode=1 --errors-for-leak-kinds=none ./$(BUILD)/$$t > /dev/null; done
```

- [ ] **Step 3: Run the tests to verify they fail**

Run: `make test`
Expected: FAIL — `src/server/moves.c` / `moves.h` not found.

- [ ] **Step 4: Implement the moves queue**

`src/server/moves.h`:

```c
/**
 * @file moves.h
 * Last moves of a game, as the server sent them to the players: the 3 opening
 * moves plus the first real move, then the last 2 moves of each turn.
 */
#ifndef MOVES_H
#define MOVES_H

#include <stddef.h>
#include "move.h"
#include "board.h"

#define MOVES_CAPACITY 4

struct moves {
  struct col_move_t t[MOVES_CAPACITY];
  size_t size;
};

struct moves* moves__initialize(void);
void moves__free(struct moves* m);
size_t moves__get_size(const struct moves* m);
/** @return the stored moves, oldest first (never NULL) */
struct col_move_t* moves__last_n(struct moves* m);
void moves__enqueue(struct moves* m, const struct col_move_t* move);
/** Stores the 3 opening moves, plays them on the board and frees `opening`. */
void moves__add_opening(struct col_move_t* opening, struct moves* m, struct board* b);

#endif
```

`src/server/moves.c`:

```c
/**
 * @file moves.c
 * Implementation of moves.h.
 */
#include <stdlib.h>
#include "moves.h"

struct moves* moves__initialize(void){
  struct moves* m = malloc(sizeof(*m));
  m->size = 0;
  return m;
}

void moves__free(struct moves* m){
  free(m);
}

size_t moves__get_size(const struct moves* m){
  return m->size;
}

struct col_move_t* moves__last_n(struct moves* m){
  return m->t;
}

void moves__enqueue(struct moves* m, const struct col_move_t* move){
  /* Once a turn is complete (2 moves, or 4 right after an opening) only the
     latest move is kept: a player receives its own last move and the reply. */
  if (m->size == 2 || m->size == MOVES_CAPACITY){
    m->t[0] = m->t[m->size - 1];
    m->size = 1;
  }
  m->t[m->size++] = *move;
}

void moves__add_opening(struct col_move_t* opening, struct moves* m, struct board* b){
  for (int i = 0; i < 3; i++){
    m->t[i] = opening[i];
    board__add_move(b, opening[i].m, opening[i].c);
  }
  m->size = 3;
  free(opening);
}
```

- [ ] **Step 5: Run everything**

Run: `make test && make players`
Expected: `test_moves` prints `NOMBRE D'ECHEC : 0`, `test_matrix` three SUCCESS lines, all four suites exit 0; `build/player4.so`, `build/player4.2.so`, `build/player4.3.so`, `build/player4.4.so` exist. Warnings from the original player sources are acceptable; errors are not. If a player fails to compile because of a missing `board__*` function, add it to `board.h`/`bitboard.c` (never edit the player).

If `valgrind` is installed (Linux only): `make valgrind` → exit 0. On macOS skip it; CI runs it (Task 10).

- [ ] **Step 6: Commit**

```bash
git add Makefile src/server/moves.h src/server/moves.c src/test/test_moves.c src/test/test_matrix.c
git commit -m "Rebuild moves queue, restore all native test suites and player builds

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 5: Player 4.4 — thread switch, depth cap, strength and equivalence tests

**Files:**
- Modify: `src/players/player4.4.c`
- Create: `src/test/test_match.c`, `src/test/test_equiv.c`
- Modify: `Makefile`

**Interfaces:**
- Consumes: Tasks 1–4.
- Produces:
  - `void set_max_depth(int depth)` in `player4.4.c` (default cap 4 = original behaviour; values ≤ 0 ignored).
  - Compile switch `GOMOKU_NO_THREADS`: root moves evaluated sequentially, same order and result as the threaded build.
  - Make target `test-equiv`; `test_match` in `TESTS`.

- [ ] **Step 1: Write the strength test**

`src/test/test_match.c`:

```c
/**
 * @file test_match.c
 * Player 4.4 against a random opponent: checks that the rebuilt bitboard
 * (explore_line especially) still produces an AI that actually plays.
 * The random opponent lives here rather than in player1.c because two
 * players cannot be linked in the same binary (same symbol names).
 */
#include <stdio.h>
#include <stdlib.h>
#include "../players/player.h"
#include "../server/board.h"

#define SIZE 10
#define GAMES 10
#define MIN_WINS 9

static struct move_t random_move(const struct board* b){
  struct move_t m;
  do {
    m.row = (size_t)(rand() % SIZE);
    m.col = (size_t)(rand() % SIZE);
  } while (!board__is_valid_move(b, m));
  return m;
}

/* @return 1 if player 4.4, playing `ai_color`, wins the game */
static int play_game(enum color_t ai_color){
  struct board* b = board__initialize(SIZE);
  initialize(SIZE, ai_color);
  /* Moves the AI has not seen yet: its own last move and the reply */
  struct col_move_t pending[4];
  size_t n_pending = 0;
  enum color_t turn = BLACK;
  int winner = -1;
  for (int ply = 0; ply < SIZE * SIZE && winner < 0; ply++){
    struct col_move_t cm;
    cm.c = turn;
    if (turn == ai_color){
      cm.m = play(pending, n_pending);
      n_pending = 0;
      if (!board__is_valid_move(b, cm.m)){
        fprintf(stderr, "4.4 played an invalid move (%zu, %zu)\n", cm.m.row, cm.m.col);
        winner = (ai_color == BLACK) ? WHITE : BLACK;
        break;
      }
    } else {
      cm.m = random_move(b);
    }
    board__add_move(b, cm.m, turn);
    pending[n_pending++] = cm;
    if (board__won(b, cm.m)){
      winner = turn;
    }
    turn = (turn == BLACK) ? WHITE : BLACK;
  }
  finalize();
  board__free(b);
  return winner == (int)ai_color;
}

int main(void){
  srand(42);
  int wins = 0;
  for (int g = 0; g < GAMES; g++){
    wins += play_game(g % 2 == 0 ? BLACK : WHITE);
  }
  printf("4.4 won %d/%d games against random\n", wins, GAMES);
  return wins >= MIN_WINS ? EXIT_SUCCESS : EXIT_FAILURE;
}
```

- [ ] **Step 2: Write the equivalence program**

`src/test/test_equiv.c`:

```c
/**
 * @file test_equiv.c
 * Prints the moves player 4.4 picks on fixed positions. Built twice (with and
 * without GOMOKU_NO_THREADS); `make test-equiv` checks both builds agree.
 */
#include <stdio.h>
#include "../players/player.h"

#define SIZE 10

struct position {
  enum color_t to_play;
  size_t n;
  struct col_move_t moves[8];
};

static const struct position positions[] = {
  { WHITE, 1, {{{4,4},BLACK}} },
  { BLACK, 2, {{{4,4},BLACK},{{4,5},WHITE}} },
  { BLACK, 4, {{{4,4},BLACK},{{5,5},WHITE},{{4,5},BLACK},{{3,3},WHITE}} },
  { WHITE, 5, {{{4,4},BLACK},{{5,5},WHITE},{{4,5},BLACK},{{3,3},WHITE},{{4,6},BLACK}} },
  { BLACK, 6, {{{2,2},BLACK},{{7,7},WHITE},{{2,3},BLACK},{{7,6},WHITE},{{3,3},BLACK},{{6,6},WHITE}} },
};

int main(void){
  size_t count = sizeof(positions) / sizeof(positions[0]);
  for (size_t i = 0; i < count; i++){
    initialize(SIZE, positions[i].to_play);
    struct move_t m = play(positions[i].moves, positions[i].n);
    printf("MOVE %zu %zu %zu\n", i, m.row, m.col);
    finalize();
  }
  return 0;
}
```

- [ ] **Step 3: Wire them into the Makefile**

Change `TESTS` to `TESTS := test_bitboard test_player test_moves test_matrix test_match`, add `test-equiv` to `.PHONY`, change the `test:` rule's prerequisites to `$(addprefix $(BUILD)/,$(TESTS)) test-equiv`, and add:

```make
PLAYER44 := $(PLY)/player4.4.c $(PLY)/heuristic3.c $(COMMON) $(BITBOARD)

$(BUILD)/test_match: $(TST)/test_match.c $(PLAYER44) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -pthread $(filter %.c,$^) -o $@ $(LDLIBS)

$(BUILD)/equiv_threads: $(TST)/test_equiv.c $(PLAYER44) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -pthread $(filter %.c,$^) -o $@ $(LDLIBS)

# pthreads live in libc on macOS and recent glibc: renaming pthread_create
# turns any leftover thread use in the sequential build into a link error.
$(BUILD)/equiv_seq: $(TST)/test_equiv.c $(PLAYER44) $(HDRS) | $(BUILD)
	$(CC) $(CFLAGS) -DGOMOKU_NO_THREADS -Dpthread_create=gomoku_threads_forbidden $(filter %.c,$^) -o $@ $(LDLIBS)

# The WebAssembly build has no threads: both builds must pick the same moves.
test-equiv: $(BUILD)/equiv_threads $(BUILD)/equiv_seq
	./$(BUILD)/equiv_threads | grep '^MOVE' > $(BUILD)/equiv_threads.txt
	./$(BUILD)/equiv_seq | grep '^MOVE' > $(BUILD)/equiv_seq.txt
	diff $(BUILD)/equiv_threads.txt $(BUILD)/equiv_seq.txt
	@echo "threaded and sequential 4.4 agree"
```

- [ ] **Step 4: Run the tests to verify they fail**

Run: `make test`
Expected: `test_match` prints `4.4 won N/10` (the strength check may already pass; it takes 1–3 minutes), then `equiv_seq` fails to link with an undefined symbol `gomoku_threads_forbidden` — `player4.4.c` does not handle `GOMOKU_NO_THREADS` yet.

- [ ] **Step 5: Add the thread switch and depth cap to `player4.4.c`**

Replace `#include <pthread.h>` with:

```c
#ifndef GOMOKU_NO_THREADS
#include <pthread.h>
#endif
```

After `struct player self;` add:

```c
/* Upper bound on the search depth; the web build lowers it for easier levels */
static int max_depth = 4;

void set_max_depth(int depth){
  if (depth > 0){ max_depth = depth; }
}
```

In `play()`, right after the line `if (self.nb_moves < 6){ self.h->depth = 4; self.h->nb_childs = 10; }` add:

```c
  if (self.h->depth > max_depth){ self.h->depth = max_depth; }
```

In `play()`, replace everything from the line `  // multithreading` down to (and including) `  return next_mv;` with:

```c
  int length = self.h->cursor-first_cursor;
#ifdef GOMOKU_NO_THREADS
  // WebAssembly build: GitHub Pages cannot serve the COOP/COEP headers that
  // wasm threads need, so the root moves are evaluated one after the other,
  // in the same order as the threaded version, keeping one heuristic copy alive.
  for (int i = 0; i < length; i++){
    struct data d;
    d.mv = pop(self.h);
    d.step = 0;
    d.cp = heuristic_copy(self.h);
    thread_evaluation(&d);
    if (d.step > better_move){
      better_move = d.step;
      next_mv = d.mv;
    }
    nb_nodes += d.cp->nb_node_explored;
    heuristic_free(d.cp);
  }
  t2 = clock();
  printf("%d noeuds en %lf secondes\n", nb_nodes, (double)(t2-t1)/(double)clk_tck);
  return next_mv;
#else
  // multithreading
  // we need to make a copy of the heuristic because its value will be modified
  // during recursion
  pthread_t * threads = malloc(sizeof(pthread_t)*length);
  struct data * dati = malloc(sizeof(struct data)*length);
  for (int i = 0; i < length; i++){
    dati[i].mv = pop(self.h);
    dati[i].step = 0;
    dati[i].cp = heuristic_copy(self.h);
  }
  for(int i = 0; i < length; i++){
      pthread_create(threads+i, NULL, thread_evaluation, &dati[i]);
  }

  for(int i = 0; i < length; i++){
      pthread_join (threads[i], NULL);
     //printf("power of %ld %ld : %d\n", dati[i].mv.row, dati[i].mv.col, dati[i].step);
      if (dati[i].step > better_move){
        better_move = dati[i].step;
        next_mv = dati[i].mv;
    }
    nb_nodes += dati[i].cp->nb_node_explored;
    heuristic_free(dati[i].cp);
  }
  t2 = clock();
  printf("%d noeuds en %lf secondes\n", nb_nodes, (double)(t2-t1)/(double)clk_tck);
  free(threads);
  free(dati);
  return next_mv;
#endif
```

(The `#else` branch is the original code, unchanged.)

- [ ] **Step 6: Run everything**

Run: `make test && make players`
Expected: every suite exits 0, `4.4 won N/10 games against random` with N ≥ 9, `threaded and sequential 4.4 agree`.
If N < 9: stop and report to Laurent with the output — it means the rebuilt `explore_line` weakens the AI.

- [ ] **Step 7: Commit**

```bash
git add src/players/player4.4.c src/test/test_match.c src/test/test_equiv.c Makefile
git commit -m "Add sequential build switch and depth cap to player 4.4

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 6: Web tooling + WebAssembly modules

**Files:**
- Create: `src/wasm/referee_api.c`, `src/wasm/player_api.c`
- Modify: `Makefile` (`wasm` target)
- Create: `web/package.json` (via npm), `web/tsconfig.json`, `web/vite.config.ts`
- Create: `web/src/wasm/types.ts`, `web/src/wasm/generated/referee.d.ts`, `web/src/wasm/generated/player41.d.ts`, `web/src/wasm/generated/player44.d.ts`
- Test: `web/test/wasm.test.ts`

**Interfaces:**
- Consumes: Tasks 1–5 (`board__*`, `player.h`, `set_max_depth`).
- Produces (C exports, called from TS with a leading underscore):
  - referee: `int ref_new(int size)` (1 on success), `void ref_free(void)`, `int ref_play(int row, int col, int color)` → `0 invalid | 1 ok | 2 won | 3 full`, `int ref_win_count(void)`, `int ref_win_cell(int i)` → `row*size+col` or -1
  - player: `void ai_init(int size, int color, int depth)`, `void ai_push(int row, int col, int color)`, `int ai_play(void)` → `row*size+col`, `void ai_finalize(void)`
  - TS: `RefereeModule`, `PlayerModule`, default-exported factories from `web/src/wasm/generated/{referee,player41,player44}.js`

Prerequisite: Emscripten installed (`brew install emscripten`, then `emcc --version` works).

- [ ] **Step 1: Scaffold the web project**

```bash
mkdir -p web/src/wasm/generated web/test
cd web
npm init -y
npm pkg set name=gomoku-web type=module
npm pkg set private=true --json
npm pkg delete main
npm pkg set scripts.dev="vite" scripts.build="tsc && vite build" scripts.preview="vite preview" scripts.test="vitest run" scripts.e2e="playwright test" scripts.record="playwright test --config scripts/record.config.ts && bash scripts/to-gif.sh"
npm install -D vite typescript vitest @playwright/test @types/node
cd ..
```

`web/tsconfig.json`:

```json
{
  "compilerOptions": {
    "target": "ES2022",
    "module": "ESNext",
    "moduleResolution": "Bundler",
    "lib": ["ES2022", "DOM", "DOM.Iterable"],
    "types": ["vite/client", "node"],
    "strict": true,
    "noEmit": true,
    "skipLibCheck": true,
    "isolatedModules": true,
    "noUnusedLocals": true,
    "noUnusedParameters": true
  },
  "include": ["src", "test", "e2e", "scripts", "vite.config.ts", "playwright.config.ts"]
}
```

`web/vite.config.ts`:

```ts
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
```

- [ ] **Step 2: Write the TypeScript typings for the modules**

`web/src/wasm/types.ts`:

```ts
/** Exports of referee.wasm (src/wasm/referee_api.c). */
export interface RefereeModule {
  _ref_new(size: number): number;
  _ref_free(): void;
  /** 0 invalid, 1 ok, 2 won, 3 full */
  _ref_play(row: number, col: number, color: number): number;
  _ref_win_count(): number;
  _ref_win_cell(index: number): number;
}

/** Exports of player41.wasm / player44.wasm (src/wasm/player_api.c). */
export interface PlayerModule {
  _ai_init(size: number, color: number, depth: number): void;
  _ai_push(row: number, col: number, color: number): void;
  /** @returns row * size + col */
  _ai_play(): number;
  _ai_finalize(): void;
}

export interface ModuleOptions {
  wasmBinary?: Uint8Array;
}
```

`web/src/wasm/generated/referee.d.ts`:

```ts
import type { ModuleOptions, RefereeModule } from '../types';

declare const createReferee: (options?: ModuleOptions) => Promise<RefereeModule>;
export default createReferee;
```

`web/src/wasm/generated/player41.d.ts` and `web/src/wasm/generated/player44.d.ts` (same content in both files):

```ts
import type { ModuleOptions, PlayerModule } from '../types';

declare const createPlayer: (options?: ModuleOptions) => Promise<PlayerModule>;
export default createPlayer;
```

- [ ] **Step 3: Write the failing Node tests**

`web/test/wasm.test.ts`:

```ts
import { readFileSync } from 'node:fs';
import { describe, expect, it } from 'vitest';
import createReferee from '../src/wasm/generated/referee.js';
import createPlayer41 from '../src/wasm/generated/player41.js';
import createPlayer44 from '../src/wasm/generated/player44.js';
import type { ModuleOptions, PlayerModule, RefereeModule } from '../src/wasm/types';

const SIZE = 10;
const BLACK = 0;
const WHITE = 1;
const INVALID = 0;
const OK = 1;
const WON = 2;
const FULL = 3;

type Factory<T> = (options?: ModuleOptions) => Promise<T>;
type Stone = [row: number, col: number, color: number];

// Feed the binary directly: no reliance on the loader finding the file on disk.
const wasmBinary = (name: string) =>
  readFileSync(new URL(`../src/wasm/generated/${name}.wasm`, import.meta.url));

async function newReferee(): Promise<RefereeModule> {
  const ref = await createReferee({ wasmBinary: wasmBinary('referee') });
  expect(ref._ref_new(SIZE)).toBe(1);
  return ref;
}

async function aiMove(
  factory: Factory<PlayerModule>, name: string, depth: number, color: number, stones: Stone[],
): Promise<number> {
  const ai = await factory({ wasmBinary: wasmBinary(name) });
  ai._ai_init(SIZE, color, depth);
  for (const [row, col, c] of stones) ai._ai_push(row, col, c);
  const index = ai._ai_play();
  ai._ai_finalize();
  return index;
}

describe('referee.wasm', () => {
  it('rejects occupied and out-of-board cells', async () => {
    const ref = await newReferee();
    expect(ref._ref_play(4, 4, BLACK)).toBe(OK);
    expect(ref._ref_play(4, 4, WHITE)).toBe(INVALID);
    expect(ref._ref_play(-1, 0, WHITE)).toBe(INVALID);
    expect(ref._ref_play(0, SIZE, WHITE)).toBe(INVALID);
  });

  const lines: Array<[name: string, dr: number, dc: number]> = [
    ['row', 0, 1],
    ['column', 1, 0],
    ['diagonal', 1, 1],
    ['anti-diagonal', 1, -1],
  ];
  it.each(lines)('detects a five in a %s and reports its cells', async (_name, dr, dc) => {
    const ref = await newReferee();
    const whiteReplies = [[9, 0], [9, 1], [9, 2], [9, 3]];
    for (let k = 0; k < 4; k++) {
      expect(ref._ref_play(2 + k * dr, 5 + k * dc, BLACK)).toBe(OK);
      expect(ref._ref_play(whiteReplies[k][0], whiteReplies[k][1], WHITE)).toBe(OK);
    }
    expect(ref._ref_play(2 + 4 * dr, 5 + 4 * dc, BLACK)).toBe(WON);
    const cells = Array.from({ length: ref._ref_win_count() }, (_, i) => ref._ref_win_cell(i));
    const expected = Array.from({ length: 5 }, (_, k) => (2 + k * dr) * SIZE + 5 + k * dc);
    expect([...cells].sort((a, b) => a - b)).toEqual([...expected].sort((a, b) => a - b));
  });

  it('reports a full board without a five as full', async () => {
    const ref = await newReferee();
    // (col + floor(row / 2)) % 2 fills a 10x10 board without any five in a row.
    for (let row = 0; row < SIZE; row++) {
      for (let col = 0; col < SIZE; col++) {
        const last = row === SIZE - 1 && col === SIZE - 1;
        expect(ref._ref_play(row, col, (col + Math.floor(row / 2)) % 2)).toBe(last ? FULL : OK);
      }
    }
  });
});

describe('player modules', () => {
  const midGame: Stone[] = [[4, 4, BLACK], [5, 5, WHITE], [4, 5, BLACK], [3, 3, WHITE], [5, 4, BLACK], [6, 6, WHITE]];

  it.each([
    ['player41', createPlayer41, 0],
    ['player44', createPlayer44, 2],
    ['player44', createPlayer44, 4],
  ] as const)('%s (depth %i) returns a legal move in under 10 s', async (name, factory, depth) => {
    const started = performance.now();
    const index = await aiMove(factory, name, depth, BLACK, midGame);
    expect(performance.now() - started).toBeLessThan(10_000);
    expect(index).toBeGreaterThanOrEqual(0);
    expect(index).toBeLessThan(SIZE * SIZE);
    const occupied = midGame.map(([r, c]) => r * SIZE + c);
    expect(occupied).not.toContain(index);
  });

  it('player44 blocks an open four', async () => {
    const stones: Stone[] = [[4, 2, BLACK], [4, 1, WHITE], [4, 3, BLACK], [0, 0, WHITE], [4, 4, BLACK], [9, 9, WHITE], [4, 5, BLACK]];
    expect(await aiMove(createPlayer44, 'player44', 4, WHITE, stones)).toBe(4 * SIZE + 6);
  });

  it('player44 completes its own five', async () => {
    const stones: Stone[] = [[4, 2, BLACK], [0, 0, WHITE], [4, 3, BLACK], [0, 9, WHITE], [4, 4, BLACK], [9, 0, WHITE], [4, 5, BLACK], [9, 9, WHITE]];
    expect([4 * SIZE + 1, 4 * SIZE + 6]).toContain(await aiMove(createPlayer44, 'player44', 4, BLACK, stones));
  });
});
```

- [ ] **Step 4: Run them to verify they fail**

Run: `cd web && npx vitest run test/wasm.test.ts; cd ..`
Expected: FAIL — cannot resolve `../src/wasm/generated/referee.js`.

- [ ] **Step 5: Write the C adapters**

`src/wasm/referee_api.c`:

```c
/**
 * @file referee_api.c
 * WebAssembly exports of the game rules: the web app never decides by itself
 * whether a move is legal, wins or fills the board.
 */
#include <emscripten/emscripten.h>
#include "../server/board.h"

/* A single move can join two runs of 4: at most 9 aligned stones */
#define MAX_LINE 9

static struct board* g_board = NULL;
static int g_line[MAX_LINE];
static int g_line_count = 0;

static const int DR[4] = {0, 1, 1, 1};
static const int DC[4] = {1, 0, 1, -1};

EMSCRIPTEN_KEEPALIVE int ref_new(int size){
  board__free(g_board);
  g_board = board__initialize((size_t)size);
  g_line_count = 0;
  return g_board != NULL;
}

EMSCRIPTEN_KEEPALIVE void ref_free(void){
  board__free(g_board);
  g_board = NULL;
  g_line_count = 0;
}

static int same_color(int row, int col, int color){
  return board__get_color(g_board, row, col) == color;
}

/* Stores the cells of the five (or more) that goes through (row, col) */
static void record_line(int row, int col, int color){
  int size = (int)g_board->size;
  for (int d = 0; d < 4; d++){
    int r = row, c = col;
    while (same_color(r - DR[d], c - DC[d], color)){ r -= DR[d]; c -= DC[d]; }
    int count = 0;
    while (count < MAX_LINE && same_color(r, c, color)){
      g_line[count++] = r * size + c;
      r += DR[d];
      c += DC[d];
    }
    if (count >= 5){
      g_line_count = count;
      return;
    }
  }
  g_line_count = 0;
}

EMSCRIPTEN_KEEPALIVE int ref_play(int row, int col, int color){
  if (g_board == NULL || row < 0 || col < 0 || (color != BLACK && color != WHITE)){
    return 0;
  }
  struct move_t m = { (size_t)row, (size_t)col };
  if (!board__is_valid_move(g_board, m)){
    return 0;
  }
  board__add_move(g_board, m, color == BLACK ? BLACK : WHITE);
  if (board__won(g_board, m)){
    record_line(row, col, color);
    return 2;
  }
  return board__is_full(g_board) ? 3 : 1;
}

EMSCRIPTEN_KEEPALIVE int ref_win_count(void){
  return g_line_count;
}

EMSCRIPTEN_KEEPALIVE int ref_win_cell(int i){
  return (i >= 0 && i < g_line_count) ? g_line[i] : -1;
}
```

`src/wasm/player_api.c`:

```c
/**
 * @file player_api.c
 * WebAssembly exports wrapping the generic player interface (player.h).
 * Compiled once per player; each module instance holds one player.
 */
#include <emscripten/emscripten.h>
#include "../players/player.h"

#ifdef PLAYER_HAS_DEPTH
void set_max_depth(int depth);
#endif

#define MAX_CELLS 121

static size_t g_size = 0;
/* Moves the player has not seen yet, pushed one by one from JavaScript to
   avoid sharing heap memory with the caller. */
static struct col_move_t g_pending[MAX_CELLS];
static size_t g_count = 0;

EMSCRIPTEN_KEEPALIVE void ai_init(int size, int color, int depth){
  g_size = (size_t)size;
  g_count = 0;
  initialize(g_size, color == BLACK ? BLACK : WHITE);
#ifdef PLAYER_HAS_DEPTH
  set_max_depth(depth);
#else
  (void)depth;
#endif
}

EMSCRIPTEN_KEEPALIVE void ai_push(int row, int col, int color){
  if (g_count >= MAX_CELLS){
    return;
  }
  g_pending[g_count].m.row = (size_t)row;
  g_pending[g_count].m.col = (size_t)col;
  g_pending[g_count].c = (color == BLACK) ? BLACK : WHITE;
  g_count++;
}

EMSCRIPTEN_KEEPALIVE int ai_play(void){
  struct move_t m = play(g_pending, g_count);
  g_count = 0;
  return (int)(m.row * g_size + m.col);
}

EMSCRIPTEN_KEEPALIVE void ai_finalize(void){
  finalize();
}
```

- [ ] **Step 6: Add the `wasm` target to the Makefile**

Add `wasm` to `.PHONY` and append:

```make
EMCC     ?= emcc
WASM_OUT := web/src/wasm/generated
EMFLAGS  := -O3 -std=c99 -Wall -sMODULARIZE=1 -sEXPORT_ES6=1 \
            -sENVIRONMENT=web,worker,node -sALLOW_MEMORY_GROWTH=1 \
            -sSTACK_SIZE=1048576 -sFILESYSTEM=0

wasm: $(WASM_OUT)/referee.js $(WASM_OUT)/player41.js $(WASM_OUT)/player44.js

$(WASM_OUT)/referee.js: src/wasm/referee_api.c $(BITBOARD) $(HDRS)
	$(EMCC) $(EMFLAGS) $(filter %.c,$^) -o $@

$(WASM_OUT)/player41.js: src/wasm/player_api.c $(PLY)/player4.c $(PLY)/heuristic0.c $(COMMON) $(HDRS)
	$(EMCC) $(EMFLAGS) $(filter %.c,$^) -o $@

$(WASM_OUT)/player44.js: src/wasm/player_api.c $(PLAYER44) $(HDRS)
	$(EMCC) $(EMFLAGS) -DGOMOKU_NO_THREADS -DPLAYER_HAS_DEPTH $(filter %.c,$^) -o $@
```

- [ ] **Step 7: Build and run the tests**

Run: `make wasm && cd web && npx vitest run test/wasm.test.ts; cd ..`
Expected: `web/src/wasm/generated/` contains `referee.js/.wasm`, `player41.js/.wasm`, `player44.js/.wasm`; all tests PASS (the AI tests print the original `noeuds en … secondes` lines — expected).
If a player test fails with `RuntimeError: memory access out of bounds` or `stack overflow`, raise `-sSTACK_SIZE` to `4194304` and retry once; if it still fails, stop and report.

- [ ] **Step 8: Typecheck and commit**

Run: `cd web && npx tsc; cd ..` → no errors.

```bash
git add Makefile src/wasm web/package.json web/package-lock.json web/tsconfig.json web/vite.config.ts web/src/wasm web/test/wasm.test.ts
git status --short   # generated .js/.wasm must NOT be staged
git commit -m "Compile the referee and players 4.1/4.4 to WebAssembly

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 7: Match state machine (pure TypeScript)

**Files:**
- Create: `web/src/game/types.ts`, `web/src/game/match.ts`
- Test: `web/src/game/match.test.ts`

Needs only the web scaffold from Task 6 Step 1.

**Interfaces:**
- Consumes: nothing from C.
- Produces:
  - `types.ts`: `Color` (`0 | 1`), `BLACK`, `WHITE`, `other(c)`, `Cell {row, col}`, `Move extends Cell {color}`, `Level` (`'easy' | 'medium' | 'hard'`), `PlayResult` (`'invalid' | 'ok' | 'won' | 'full'`), `Referee {reset(size), play(row, col, color): PlayResult, winningLine(): Cell[]}`, `AiMove extends Cell {ms}`, `AiPlayer {init(size, color): Promise<void>, play(moves: Move[]): Promise<AiMove>, dispose(): void}`, `Side` (`{kind:'human'} | {kind:'ai', level}`)
  - `match.ts`: `MatchState` union, `MatchDeps {referee, createAi(level), delay(ms), minAiDelayMs}`, `class Match { moves; state; lastAiMs; constructor(deps, onChange: () => void); start(size, black: Side, white: Side): Promise<void>; humanPlay(row, col): boolean; dispose(): void }`

- [ ] **Step 1: Write the shared types**

`web/src/game/types.ts`:

```ts
/** Stone colors, matching `enum color_t` in src/server/move.h. */
export type Color = 0 | 1;
export const BLACK: Color = 0;
export const WHITE: Color = 1;
export const other = (color: Color): Color => (color === BLACK ? WHITE : BLACK);

export interface Cell {
  row: number;
  col: number;
}

export interface Move extends Cell {
  color: Color;
}

export type Level = 'easy' | 'medium' | 'hard';

export type PlayResult = 'invalid' | 'ok' | 'won' | 'full';

/** Game rules, implemented in C (referee.wasm). */
export interface Referee {
  reset(size: number): void;
  play(row: number, col: number, color: Color): PlayResult;
  winningLine(): Cell[];
}

export interface AiMove extends Cell {
  ms: number;
}

export interface AiPlayer {
  init(size: number, color: Color): Promise<void>;
  /** `moves`: every move this AI has not seen yet, its own previous move included. */
  play(moves: Move[]): Promise<AiMove>;
  dispose(): void;
}

export type Side = { kind: 'human' } | { kind: 'ai'; level: Level };
```

- [ ] **Step 2: Write the failing tests**

`web/src/game/match.test.ts`:

```ts
import { describe, expect, it, vi } from 'vitest';
import { Match, type MatchDeps } from './match';
import { BLACK, WHITE, type AiMove, type AiPlayer, type Cell, type Move, type PlayResult, type Referee } from './types';

class FakeReferee implements Referee {
  occupied = new Set<string>();
  /** "row,col" of the move that completes a five */
  winOn: string | null = null;
  fullAfter = Infinity;
  reset(): void {
    this.occupied.clear();
  }
  play(row: number, col: number): PlayResult {
    const key = `${row},${col}`;
    if (this.occupied.has(key) || row < 0 || col < 0 || row >= 10 || col >= 10) return 'invalid';
    this.occupied.add(key);
    if (key === this.winOn) return 'won';
    if (this.occupied.size >= this.fullAfter) return 'full';
    return 'ok';
  }
  winningLine(): Cell[] {
    return [{ row: 0, col: 0 }];
  }
}

class FakeAi implements AiPlayer {
  received: Move[][] = [];
  disposed = false;
  constructor(private readonly script: Array<AiMove | Promise<AiMove>> = []) {}
  async init(): Promise<void> {}
  play(moves: Move[]): Promise<AiMove> {
    this.received.push(moves);
    const next = this.script.shift();
    return next ? Promise.resolve(next) : Promise.reject(new Error('script exhausted'));
  }
  dispose(): void {
    this.disposed = true;
  }
}

class FailingAi extends FakeAi {
  async init(): Promise<void> {
    throw new Error('wasm failed to load');
  }
}

const at = (row: number, col: number): AiMove => ({ row, col, ms: 1 });
const flush = () => new Promise((resolve) => setTimeout(resolve, 0));
const human = { kind: 'human' } as const;
const ai = { kind: 'ai', level: 'easy' } as const;

function setup(ais: FakeAi[]) {
  const referee = new FakeReferee();
  const queue = [...ais];
  const deps: MatchDeps = {
    referee,
    createAi: () => queue.shift()!,
    delay: () => Promise.resolve(),
    minAiDelayMs: 0,
  };
  return { match: new Match(deps, () => {}), referee };
}

describe('Match', () => {
  it('starts on the human turn when the human plays black', async () => {
    const { match } = setup([new FakeAi()]);
    await match.start(10, human, ai);
    expect(match.state).toEqual({ phase: 'humanTurn', color: BLACK });
  });

  it('sends the AI every move it has not seen, including its own last move', async () => {
    const bot = new FakeAi([at(5, 5), at(6, 6)]);
    const { match } = setup([bot]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await flush();
    match.humanPlay(4, 5);
    await flush();
    expect(bot.received).toEqual([
      [{ row: 4, col: 4, color: BLACK }],
      [{ row: 5, col: 5, color: WHITE }, { row: 4, col: 5, color: BLACK }],
    ]);
    expect(match.moves).toHaveLength(4);
    expect(match.state).toEqual({ phase: 'humanTurn', color: BLACK });
  });

  it('lets the AI open as black', async () => {
    const bot = new FakeAi([at(4, 4)]);
    const { match } = setup([bot]);
    await match.start(10, ai, human);
    await flush();
    expect(bot.received).toEqual([[]]);
    expect(match.state).toEqual({ phase: 'humanTurn', color: WHITE });
  });

  it('rejects a move on an occupied cell', async () => {
    const { match } = setup([new FakeAi([at(5, 5)])]);
    await match.start(10, human, ai);
    expect(match.humanPlay(4, 4)).toBe(true);
    await flush();
    expect(match.humanPlay(5, 5)).toBe(false);
    expect(match.state).toEqual({ phase: 'humanTurn', color: BLACK });
  });

  it('ignores clicks while the AI is thinking', async () => {
    const { match } = setup([new FakeAi([new Promise<AiMove>(() => {})])]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    expect(match.state.phase).toBe('aiThinking');
    expect(match.humanPlay(0, 0)).toBe(false);
    expect(match.moves).toHaveLength(1);
  });

  it('ends the game on a win', async () => {
    const { match, referee } = setup([new FakeAi()]);
    referee.winOn = '0,4';
    await match.start(10, human, ai);
    match.humanPlay(0, 4);
    expect(match.state).toEqual({ phase: 'over', winner: BLACK, line: [{ row: 0, col: 0 }] });
  });

  it('ends the game in a draw when the board is full', async () => {
    const { match, referee } = setup([new FakeAi([at(5, 5)])]);
    referee.fullAfter = 2;
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await flush();
    expect(match.state).toEqual({ phase: 'over', winner: null, line: [] });
  });

  it('plays AI against AI until the end', async () => {
    const blackAi = new FakeAi([at(0, 0), at(0, 1), at(0, 2)]);
    const whiteAi = new FakeAi([at(9, 0), at(9, 1)]);
    const { match, referee } = setup([blackAi, whiteAi]);
    referee.winOn = '0,2';
    await match.start(10, ai, { kind: 'ai', level: 'hard' });
    await vi.waitFor(() => expect(match.state.phase).toBe('over'));
    expect(match.state).toMatchObject({ phase: 'over', winner: BLACK });
    expect(match.moves).toHaveLength(5);
  });

  it('reports a crash when the AI throws', async () => {
    vi.spyOn(console, 'error').mockImplementation(() => {});
    const bot = new FakeAi([]);
    const { match } = setup([bot]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await flush();
    expect(match.state).toEqual({ phase: 'error', message: 'The AI crashed' });
    expect(bot.disposed).toBe(true);
  });

  it('reports a crash when the AI plays an illegal move', async () => {
    vi.spyOn(console, 'error').mockImplementation(() => {});
    const { match } = setup([new FakeAi([at(4, 4)])]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await flush();
    expect(match.state).toEqual({ phase: 'error', message: 'The AI crashed' });
  });

  it('reports an AI that fails to load', async () => {
    vi.spyOn(console, 'error').mockImplementation(() => {});
    const { match } = setup([new FailingAi()]);
    await match.start(10, human, ai);
    expect(match.state).toEqual({ phase: 'error', message: 'Could not load the AI' });
  });

  it('ignores a move from a cancelled game', async () => {
    let resolveOld!: (move: AiMove) => void;
    const oldAi = new FakeAi([new Promise<AiMove>((resolve) => (resolveOld = resolve))]);
    const { match } = setup([oldAi, new FakeAi()]);
    await match.start(10, human, ai);
    match.humanPlay(4, 4);
    await match.start(10, human, ai);
    expect(oldAi.disposed).toBe(true);
    resolveOld(at(5, 5));
    await flush();
    expect(match.moves).toEqual([]);
    expect(match.state).toEqual({ phase: 'humanTurn', color: BLACK });
  });
});
```

- [ ] **Step 3: Run them to verify they fail**

Run: `cd web && npx vitest run src/game/match.test.ts; cd ..`
Expected: FAIL — cannot find module `./match`.

- [ ] **Step 4: Implement the state machine**

`web/src/game/match.ts`:

```ts
import {
  BLACK, WHITE, other,
  type AiMove, type AiPlayer, type Cell, type Color, type Level, type Move, type PlayResult, type Referee, type Side,
} from './types';

export type MatchState =
  | { phase: 'idle' }
  | { phase: 'humanTurn'; color: Color }
  | { phase: 'aiThinking'; color: Color }
  | { phase: 'over'; winner: Color | null; line: Cell[] }
  | { phase: 'error'; message: string };

export interface MatchDeps {
  referee: Referee;
  createAi(level: Level): AiPlayer;
  delay(ms: number): Promise<void>;
  /** Minimum time per AI move, so AI-vs-AI games stay watchable. */
  minAiDelayMs: number;
}

interface AiSlot {
  player: AiPlayer;
  /** Number of moves of `Match.moves` already sent to this AI */
  sent: number;
}

/** Turn order of one game. Rules come from the referee; this only sequences turns. */
export class Match {
  moves: Move[] = [];
  state: MatchState = { phase: 'idle' };
  lastAiMs: number | null = null;
  private sides: Record<Color, Side> = { 0: { kind: 'human' }, 1: { kind: 'human' } };
  private readonly ais = new Map<Color, AiSlot>();
  // Bumped on every new game so that answers from a cancelled game are dropped.
  private generation = 0;

  constructor(private readonly deps: MatchDeps, private readonly onChange: () => void) {}

  async start(size: number, black: Side, white: Side): Promise<void> {
    this.disposeAis();
    const generation = ++this.generation;
    this.moves = [];
    this.lastAiMs = null;
    this.sides = { 0: black, 1: white };
    this.deps.referee.reset(size);
    this.setState({ phase: 'idle' });
    try {
      for (const color of [BLACK, WHITE]) {
        const side = this.sides[color];
        if (side.kind === 'ai') {
          const player = this.deps.createAi(side.level);
          this.ais.set(color, { player, sent: 0 });
          await player.init(size, color);
        }
      }
    } catch (err) {
      if (generation === this.generation) this.fail('Could not load the AI', err);
      return;
    }
    if (generation !== this.generation) return;
    this.turn(BLACK);
  }

  humanPlay(row: number, col: number): boolean {
    if (this.state.phase !== 'humanTurn') return false;
    return this.apply(row, col, this.state.color) !== 'invalid';
  }

  dispose(): void {
    this.generation++;
    this.disposeAis();
  }

  private turn(color: Color): void {
    if (this.sides[color].kind === 'human') this.setState({ phase: 'humanTurn', color });
    else void this.aiTurn(color);
  }

  private async aiTurn(color: Color): Promise<void> {
    const generation = this.generation;
    const slot = this.ais.get(color)!;
    // player.h contract: the AI gets every move it has not seen, its own last one included.
    const pending = this.moves.slice(slot.sent);
    slot.sent = this.moves.length;
    this.setState({ phase: 'aiThinking', color });
    let move: AiMove;
    try {
      [move] = await Promise.all([slot.player.play(pending), this.deps.delay(this.deps.minAiDelayMs)]);
    } catch (err) {
      if (generation === this.generation) this.fail('The AI crashed', err);
      return;
    }
    if (generation !== this.generation) return;
    this.lastAiMs = move.ms;
    if (this.apply(move.row, move.col, color) === 'invalid') {
      this.fail('The AI crashed', new Error(`illegal AI move ${move.row},${move.col}`));
    }
  }

  private apply(row: number, col: number, color: Color): PlayResult {
    const result = this.deps.referee.play(row, col, color);
    if (result === 'invalid') return result;
    this.moves.push({ row, col, color });
    if (result === 'won') this.setState({ phase: 'over', winner: color, line: this.deps.referee.winningLine() });
    else if (result === 'full') this.setState({ phase: 'over', winner: null, line: [] });
    else this.turn(other(color));
    return result;
  }

  private fail(message: string, err: unknown): void {
    console.error(message, err);
    this.disposeAis();
    this.setState({ phase: 'error', message });
  }

  private disposeAis(): void {
    for (const slot of this.ais.values()) slot.player.dispose();
    this.ais.clear();
  }

  private setState(state: MatchState): void {
    this.state = state;
    this.onChange();
  }
}
```

- [ ] **Step 5: Run the tests and typecheck**

Run: `cd web && npx vitest run src/game/match.test.ts && npx tsc; cd ..`
Expected: 12 tests PASS, no type errors.

- [ ] **Step 6: Commit**

```bash
git add web/src/game/types.ts web/src/game/match.ts web/src/game/match.test.ts
git commit -m "Add the match state machine

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 8: Referee wrapper + AI worker and client

**Files:**
- Create: `web/src/game/referee.ts`, `web/src/ai/protocol.ts`, `web/src/ai/ai.worker.ts`, `web/src/ai/client.ts`
- Test: `web/test/referee.test.ts` (worker/client are covered end-to-end by Task 9's Playwright smoke test — Node has no Worker for Vite-bundled modules)

**Interfaces:**
- Consumes: Task 6 generated modules + typings; Task 7 `Referee`, `AiPlayer`, `Color`, `Level`, `Move`, `AiMove`, `PlayResult`, `Cell`.
- Produces: `class WasmReferee implements Referee { static load(options?: ModuleOptions): Promise<WasmReferee> }`; `class WorkerAi implements AiPlayer { constructor(level: Level) }`; `LEVELS: Record<Level, {module: 'player41' | 'player44'; depth: number}>`; `WorkerRequest`, `WorkerResponse`.

- [ ] **Step 1: Write the failing wrapper test**

`web/test/referee.test.ts`:

```ts
import { readFileSync } from 'node:fs';
import { expect, it } from 'vitest';
import { WasmReferee } from '../src/game/referee';
import { BLACK, WHITE } from '../src/game/types';

const wasmBinary = readFileSync(new URL('../src/wasm/generated/referee.wasm', import.meta.url));

it('maps referee codes and the winning line to typed values', async () => {
  const referee = await WasmReferee.load({ wasmBinary });
  referee.reset(10);
  expect(referee.play(4, 4, BLACK)).toBe('ok');
  expect(referee.play(4, 4, WHITE)).toBe('invalid');
  for (let col = 5; col < 8; col++) referee.play(4, col, BLACK);
  expect(referee.play(4, 8, BLACK)).toBe('won');
  expect(referee.winningLine()).toEqual([4, 5, 6, 7, 8].map((col) => ({ row: 4, col })));
  referee.reset(10);
  expect(referee.play(4, 4, WHITE)).toBe('ok');
});
```

- [ ] **Step 2: Run it to verify it fails**

Run: `cd web && npx vitest run test/referee.test.ts; cd ..`
Expected: FAIL — cannot find module `../src/game/referee`.

- [ ] **Step 3: Implement the wrapper**

`web/src/game/referee.ts`:

```ts
import createReferee from '../wasm/generated/referee.js';
import type { ModuleOptions, RefereeModule } from '../wasm/types';
import type { Cell, Color, PlayResult, Referee } from './types';

const RESULTS: readonly PlayResult[] = ['invalid', 'ok', 'won', 'full'];

/** Typed view of referee.wasm. */
export class WasmReferee implements Referee {
  private size = 0;

  private constructor(private readonly mod: RefereeModule) {}

  static async load(options?: ModuleOptions): Promise<WasmReferee> {
    return new WasmReferee(await createReferee(options));
  }

  reset(size: number): void {
    if (!this.mod._ref_new(size)) throw new Error(`Unsupported board size ${size}`);
    this.size = size;
  }

  play(row: number, col: number, color: Color): PlayResult {
    return RESULTS[this.mod._ref_play(row, col, color)] ?? 'invalid';
  }

  winningLine(): Cell[] {
    const count = this.mod._ref_win_count();
    return Array.from({ length: count }, (_, i) => {
      const index = this.mod._ref_win_cell(i);
      return { row: Math.floor(index / this.size), col: index % this.size };
    });
  }
}
```

- [ ] **Step 4: Implement the worker protocol, worker and client**

`web/src/ai/protocol.ts`:

```ts
import type { Color, Level, Move } from '../game/types';

export type WorkerRequest =
  | { type: 'init'; level: Level; size: number; color: Color }
  | { type: 'play'; moves: Move[] };

export type WorkerResponse =
  | { type: 'ready' }
  | { type: 'move'; row: number; col: number; ms: number }
  | { type: 'error'; message: string };

export const LEVELS: Record<Level, { module: 'player41' | 'player44'; depth: number }> = {
  easy: { module: 'player41', depth: 0 },
  medium: { module: 'player44', depth: 2 },
  hard: { module: 'player44', depth: 4 },
};
```

`web/src/ai/ai.worker.ts`:

```ts
// One worker = one player instance: the C players keep their state in globals.
import type { PlayerModule } from '../wasm/types';
import { LEVELS, type WorkerRequest, type WorkerResponse } from './protocol';

const scope = self as unknown as {
  postMessage(message: WorkerResponse): void;
  onmessage: ((event: MessageEvent<WorkerRequest>) => void) | null;
};

let player: PlayerModule | null = null;
let size = 0;

async function loadModule(name: 'player41' | 'player44'): Promise<PlayerModule> {
  if (name === 'player41') return (await import('../wasm/generated/player41.js')).default();
  return (await import('../wasm/generated/player44.js')).default();
}

async function handle(request: WorkerRequest): Promise<WorkerResponse> {
  if (request.type === 'init') {
    const level = LEVELS[request.level];
    player = await loadModule(level.module);
    size = request.size;
    player._ai_init(request.size, request.color, level.depth);
    return { type: 'ready' };
  }
  if (!player) throw new Error('AI not initialized');
  for (const move of request.moves) player._ai_push(move.row, move.col, move.color);
  const started = performance.now();
  const index = player._ai_play();
  return { type: 'move', row: Math.floor(index / size), col: index % size, ms: Math.round(performance.now() - started) };
}

scope.onmessage = (event) => {
  handle(event.data).then(
    (response) => scope.postMessage(response),
    (err: unknown) => scope.postMessage({ type: 'error', message: String(err) }),
  );
};
```

`web/src/ai/client.ts`:

```ts
import type { AiMove, AiPlayer, Color, Level, Move } from '../game/types';
import type { WorkerRequest, WorkerResponse } from './protocol';

interface Pending {
  resolve(response: WorkerResponse): void;
  reject(err: Error): void;
}

/** AiPlayer backed by a dedicated Web Worker; terminate() is the only way to stop a search. */
export class WorkerAi implements AiPlayer {
  private readonly worker: Worker;
  private pending: Pending | null = null;

  constructor(private readonly level: Level) {
    this.worker = new Worker(new URL('./ai.worker.ts', import.meta.url), { type: 'module' });
    this.worker.onmessage = (event: MessageEvent<WorkerResponse>) => this.settle(event.data);
    this.worker.onerror = (event) => this.fail(new Error(event.message || 'AI worker crashed'));
  }

  async init(size: number, color: Color): Promise<void> {
    await this.request({ type: 'init', level: this.level, size, color });
  }

  async play(moves: Move[]): Promise<AiMove> {
    const response = await this.request({ type: 'play', moves });
    if (response.type !== 'move') throw new Error(`Unexpected AI response: ${response.type}`);
    return { row: response.row, col: response.col, ms: response.ms };
  }

  dispose(): void {
    this.worker.terminate();
    this.fail(new Error('AI disposed'));
  }

  private request(message: WorkerRequest): Promise<WorkerResponse> {
    if (this.pending) return Promise.reject(new Error('AI is busy'));
    return new Promise((resolve, reject) => {
      this.pending = { resolve, reject };
      this.worker.postMessage(message);
    });
  }

  private settle(response: WorkerResponse): void {
    const pending = this.pending;
    this.pending = null;
    if (!pending) return;
    if (response.type === 'error') pending.reject(new Error(response.message));
    else pending.resolve(response);
  }

  private fail(err: Error): void {
    const pending = this.pending;
    this.pending = null;
    pending?.reject(err);
  }
}
```

- [ ] **Step 5: Run the tests and typecheck**

Run: `cd web && npx vitest run && npx tsc; cd ..`
Expected: all tests PASS (wasm, referee, match), no type errors.

- [ ] **Step 6: Commit**

```bash
git add web/src/game/referee.ts web/src/ai web/test/referee.test.ts
git commit -m "Wrap the referee module and run each AI in a Web Worker

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 9: User interface + smoke test

**Files:**
- Create: `web/index.html`, `web/src/style.css`, `web/src/main.ts`, `web/src/ui/board.ts`, `web/src/ui/controls.ts`, `web/src/ui/status.ts`, `web/playwright.config.ts`, `web/e2e/smoke.spec.ts`
- Test: `web/src/ui/board.test.ts`, `web/src/ui/controls.test.ts`, `web/src/ui/status.test.ts`

**Interfaces:**
- Consumes: Task 7 `Match`, `MatchState`, types; Task 8 `WasmReferee`, `WorkerAi`, `LEVELS`.
- Produces:
  - `cellAt(x, y, width, size): Cell | null`, `createBoardView(canvas, size): BoardView {render(moves, line), onCellClick(handler), cellCenter(cell): {x, y}}`
  - `Settings {mode: 'human' | 'aivsai'; humanColor: Color; levelA: Level; levelB: Level}`, `settingsFromQuery(search): Settings`, `sidesFor(settings): {black: Side; white: Side}`, `bindControls(root, initial): {read(): Settings; onNewGame(handler)}`
  - `statusText(state: MatchState): string`
  - URL parameters: `mode=human|aivsai`, `color=black|white`, `a=<level>` (AI level, or Black AI in AI vs AI), `b=<level>` (White AI)
  - `window.__gomoku = { state(), moves(), lastAiMs(), cellCenter(cell) }` — used by Playwright (Tasks 9 and 11)

- [ ] **Step 1: Write the failing unit tests**

`web/src/ui/board.test.ts`:

```ts
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
```

`web/src/ui/controls.test.ts`:

```ts
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
```

`web/src/ui/status.test.ts`:

```ts
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
```

- [ ] **Step 2: Run them to verify they fail**

Run: `cd web && npx vitest run src/ui; cd ..`
Expected: FAIL — modules `./board`, `./controls`, `./status` not found.

- [ ] **Step 3: Implement the UI modules**

`web/src/ui/status.ts`:

```ts
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
```

`web/src/ui/board.ts`:

```ts
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
```

`web/src/ui/controls.ts`:

```ts
import { BLACK, WHITE, type Color, type Level, type Side } from '../game/types';

export type Mode = 'human' | 'aivsai';

export interface Settings {
  mode: Mode;
  humanColor: Color;
  /** AI level in Human vs AI, Black AI in AI vs AI */
  levelA: Level;
  /** White AI in AI vs AI */
  levelB: Level;
}

const LEVEL_NAMES: readonly Level[] = ['easy', 'medium', 'hard'];

const asLevel = (value: string | null, fallback: Level): Level =>
  LEVEL_NAMES.includes(value as Level) ? (value as Level) : fallback;

export function settingsFromQuery(search: string): Settings {
  const query = new URLSearchParams(search);
  return {
    mode: query.get('mode') === 'aivsai' ? 'aivsai' : 'human',
    humanColor: query.get('color') === 'white' ? WHITE : BLACK,
    levelA: asLevel(query.get('a'), 'medium'),
    levelB: asLevel(query.get('b'), 'hard'),
  };
}

export function sidesFor(settings: Settings): { black: Side; white: Side } {
  if (settings.mode === 'aivsai') {
    return { black: { kind: 'ai', level: settings.levelA }, white: { kind: 'ai', level: settings.levelB } };
  }
  const ai: Side = { kind: 'ai', level: settings.levelA };
  return settings.humanColor === BLACK ? { black: { kind: 'human' }, white: ai } : { black: ai, white: { kind: 'human' } };
}

export interface ControlsView {
  read(): Settings;
  onNewGame(handler: () => void): void;
}

export function bindControls(root: HTMLElement, initial: Settings): ControlsView {
  const mode = root.querySelector<HTMLSelectElement>('#mode')!;
  const color = root.querySelector<HTMLSelectElement>('#human-color')!;
  const levelA = root.querySelector<HTMLSelectElement>('#level-a')!;
  const levelB = root.querySelector<HTMLSelectElement>('#level-b')!;
  const levelALabel = root.querySelector<HTMLElement>('#level-a-label')!;

  mode.value = initial.mode;
  color.value = initial.humanColor === BLACK ? 'black' : 'white';
  levelA.value = initial.levelA;
  levelB.value = initial.levelB;

  const sync = () => {
    const aiVsAi = mode.value === 'aivsai';
    root.querySelectorAll<HTMLElement>('[data-mode="human"]').forEach((el) => (el.hidden = aiVsAi));
    root.querySelectorAll<HTMLElement>('[data-mode="aivsai"]').forEach((el) => (el.hidden = !aiVsAi));
    levelALabel.textContent = aiVsAi ? 'Black AI' : 'AI level';
  };
  mode.addEventListener('change', sync);
  sync();

  return {
    read: () => ({
      mode: mode.value === 'aivsai' ? 'aivsai' : 'human',
      humanColor: color.value === 'white' ? WHITE : BLACK,
      levelA: asLevel(levelA.value, 'medium'),
      levelB: asLevel(levelB.value, 'hard'),
    }),
    onNewGame: (handler) => root.querySelector<HTMLButtonElement>('#new-game')!.addEventListener('click', handler),
  };
}
```

- [ ] **Step 4: Run the unit tests**

Run: `cd web && npx vitest run src/ui; cd ..`
Expected: PASS.

- [ ] **Step 5: Write the page, styles and entry point**

`web/index.html`:

```html
<!doctype html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <meta name="viewport" content="width=device-width, initial-scale=1" />
    <title>Gomoku AI</title>
    <meta name="description" content="Play Gomoku against a minimax AI written in C and compiled to WebAssembly." />
  </head>
  <body>
    <main>
      <header>
        <h1>Gomoku AI</h1>
        <p class="tagline">A C school project — bitboard, minimax, alpha-beta — compiled to WebAssembly.</p>
      </header>

      <section id="controls" class="controls" aria-label="Game settings">
        <label>Mode
          <select id="mode">
            <option value="human">Human vs AI</option>
            <option value="aivsai">AI vs AI</option>
          </select>
        </label>
        <label data-mode="human">You play
          <select id="human-color">
            <option value="black">Black (first)</option>
            <option value="white">White</option>
          </select>
        </label>
        <label><span id="level-a-label">AI level</span>
          <select id="level-a">
            <option value="easy">Easy — player 4.1</option>
            <option value="medium">Medium — player 4.4, depth 2</option>
            <option value="hard">Hard — player 4.4, depth 4</option>
          </select>
        </label>
        <label data-mode="aivsai">White AI
          <select id="level-b">
            <option value="easy">Easy — player 4.1</option>
            <option value="medium">Medium — player 4.4, depth 2</option>
            <option value="hard">Hard — player 4.4, depth 4</option>
          </select>
        </label>
        <button id="new-game" type="button">New game</button>
      </section>

      <section id="board-area">
        <canvas id="board" aria-label="Gomoku board, 10 by 10"></canvas>
        <p id="status" role="status" aria-live="polite"></p>
        <p id="timing"></p>
      </section>

      <section class="how">
        <h2>How it works</h2>
        <p><strong>Bitboard.</strong> Each player's stones live in one 128-bit integer, one bit per cell — which is why boards stop at 11×11. Checking a win or a full board is a handful of bitwise operations.</p>
        <p><strong>Minimax with alpha-beta.</strong> The AI looks a few moves ahead (negamax), assumes you always answer with your best move, and prunes branches that cannot change its choice.</p>
        <p><strong>Crossing-lines heuristic.</strong> Every empty cell gets a score from 0 to 13 from the lines it would create — open fours, double open threes… Crossings score highest because that is how games are won. Only the best cells are searched.</p>
        <p><strong>In your browser.</strong> The original C code is compiled with Emscripten into three WebAssembly modules — the rules and two AI players. Each AI runs in its own Web Worker, so the page never freezes.</p>
        <p><a href="https://github.com/LaurentGENTY/gomoku-AI">Source code</a> · <a href="https://github.com/LaurentGENTY/gomoku-AI/blob/master/doc/rapport.pdf">Project report (French)</a></p>
      </section>

      <footer>ENSEIRB-MATMECA semester 6 project — Emeric Duchemin, Laurent Genty, Julien Miens, Tanguy Pemeja.</footer>
    </main>
    <script type="module" src="/src/main.ts"></script>
  </body>
</html>
```

`web/src/style.css`:

```css
:root {
  color-scheme: light dark;
  --bg: #f6f3ec;
  --fg: #1d1b16;
  --muted: #6b6559;
  --accent: #b5481f;
  --panel: #ffffff;
  --border: #ddd5c4;
  font-family: system-ui, -apple-system, 'Segoe UI', sans-serif;
}

@media (prefers-color-scheme: dark) {
  :root {
    --bg: #17150f;
    --fg: #eee8db;
    --muted: #a39b8a;
    --accent: #e8794d;
    --panel: #221f17;
    --border: #3a3427;
  }
}

* { box-sizing: border-box; }

body {
  margin: 0;
  background: var(--bg);
  color: var(--fg);
  line-height: 1.5;
}

main {
  max-width: 560px;
  margin: 0 auto;
  padding: 24px 16px 48px;
}

h1 { margin: 0; font-size: 1.8rem; }
h2 { font-size: 1.15rem; margin-top: 2rem; }
.tagline, #timing, footer { color: var(--muted); }
.tagline { margin-top: 4px; }

.controls {
  display: flex;
  flex-wrap: wrap;
  gap: 8px 16px;
  align-items: end;
  margin: 20px 0;
}

.controls label {
  display: flex;
  flex-direction: column;
  font-size: 0.85rem;
  color: var(--muted);
  gap: 4px;
}

select, button {
  font: inherit;
  color: var(--fg);
  background: var(--panel);
  border: 1px solid var(--border);
  border-radius: 6px;
  padding: 6px 10px;
}

button {
  background: var(--accent);
  border-color: var(--accent);
  color: #fff;
  cursor: pointer;
}

[hidden] { display: none !important; }

#board {
  display: block;
  width: 100%;
  max-width: 520px;
  aspect-ratio: 1;
  border-radius: 8px;
  cursor: pointer;
  box-shadow: 0 2px 12px rgb(0 0 0 / 0.2);
}

#board.waiting { cursor: progress; }

#status { font-weight: 600; margin: 12px 0 0; min-height: 1.5em; }
#timing { margin: 0; min-height: 1.5em; font-size: 0.9rem; }
a { color: var(--accent); }
footer { margin-top: 2rem; font-size: 0.85rem; }
```

`web/src/main.ts`:

```ts
import './style.css';
import { WorkerAi } from './ai/client';
import { Match, type MatchState } from './game/match';
import { WasmReferee } from './game/referee';
import type { Cell, Move } from './game/types';
import { createBoardView } from './ui/board';
import { bindControls, settingsFromQuery, sidesFor } from './ui/controls';
import { statusText } from './ui/status';

const SIZE = 10;
// Keeps AI-vs-AI games watchable (and recordable) when the AI answers instantly.
const MIN_AI_DELAY_MS = 400;

declare global {
  interface Window {
    __gomoku?: {
      state(): MatchState;
      moves(): Move[];
      lastAiMs(): number | null;
      cellCenter(cell: Cell): { x: number; y: number };
    };
  }
}

async function main(): Promise<void> {
  const canvas = document.querySelector<HTMLCanvasElement>('#board')!;
  const status = document.querySelector<HTMLElement>('#status')!;
  const timing = document.querySelector<HTMLElement>('#timing')!;

  let referee: WasmReferee;
  try {
    referee = await WasmReferee.load();
  } catch (err) {
    console.error(err);
    document.querySelector('#board-area')!.textContent =
      'Could not load the game engine (WebAssembly). Please try a recent browser.';
    return;
  }

  const view = createBoardView(canvas, SIZE);
  const match = new Match(
    {
      referee,
      createAi: (level) => new WorkerAi(level),
      delay: (ms) => new Promise((resolve) => setTimeout(resolve, ms)),
      minAiDelayMs: MIN_AI_DELAY_MS,
    },
    render,
  );

  function render(): void {
    const state = match.state;
    view.render(match.moves, state.phase === 'over' ? state.line : []);
    status.textContent = statusText(state);
    timing.textContent = match.lastAiMs === null ? '' : `Last AI move: ${match.lastAiMs} ms`;
    canvas.classList.toggle('waiting', state.phase !== 'humanTurn');
  }

  const controls = bindControls(document.querySelector<HTMLElement>('#controls')!, settingsFromQuery(location.search));
  const newGame = () => {
    const { black, white } = sidesFor(controls.read());
    void match.start(SIZE, black, white);
  };
  controls.onNewGame(newGame);
  view.onCellClick(({ row, col }) => match.humanPlay(row, col));

  window.__gomoku = {
    state: () => match.state,
    moves: () => match.moves,
    lastAiMs: () => match.lastAiMs,
    cellCenter: (cell) => view.cellCenter(cell),
  };
  newGame();
}

void main();
```

- [ ] **Step 6: Write the Playwright smoke test**

```bash
cd web && npx playwright install chromium && cd ..
```

`web/playwright.config.ts`:

```ts
import { defineConfig, devices } from '@playwright/test';

export default defineConfig({
  testDir: 'e2e',
  timeout: 60_000,
  use: { ...devices['Desktop Chrome'], baseURL: 'http://localhost:5173' },
  webServer: {
    command: 'npx vite --port 5173 --strictPort',
    url: 'http://localhost:5173',
    reuseExistingServer: !process.env.CI,
  },
});
```

`web/e2e/smoke.spec.ts`:

```ts
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
```

- [ ] **Step 7: Run every check**

Run: `cd web && npx vitest run && npx tsc && npm run e2e && npm run build; cd ..`
Expected: unit tests PASS, no type errors, `1 passed` from Playwright, `web/dist/` built with the three `.wasm` files and a worker chunk. If `vite build` errors on a Node built-in imported by the Emscripten loader, add that module name to `build.rollupOptions.external` in `vite.config.ts` and rebuild.

Manual check (2 minutes): `cd web && npm run dev`, open the URL, play a few moves on Hard, switch to AI vs AI and click "New game" while the AI is thinking — the board resets and no stray stone appears.

- [ ] **Step 8: Commit**

```bash
git add web/index.html web/src/style.css web/src/main.ts web/src/ui web/playwright.config.ts web/e2e
git commit -m "Add the board UI, controls and smoke test

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

---

### Task 10: Continuous integration + GitHub Pages

**Files:**
- Create: `.github/workflows/ci.yml`

**Interfaces:**
- Consumes: `make test`, `make players`, `make valgrind`, `make wasm`, `npm test`, `npm run e2e`, `npm run build`.
- Produces: CI on every pull request and push to `master`; Pages deployment of `web/dist` on `master`.

- [ ] **Step 1: Write the workflow**

`.github/workflows/ci.yml`:

```yaml
name: CI

on:
  push:
    branches: [master]
  pull_request:

permissions:
  contents: read

concurrency:
  group: ci-${{ github.ref }}
  cancel-in-progress: true

jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4

      - name: Native tests
        run: make test players

      - name: Valgrind
        run: |
          sudo apt-get update
          sudo apt-get install -y valgrind
          make valgrind

      - uses: mymindstorm/setup-emsdk@v14
        with:
          version: latest

      - name: WebAssembly build
        run: make wasm

      - uses: actions/setup-node@v4
        with:
          node-version: 22
          cache: npm
          cache-dependency-path: web/package-lock.json

      - name: Web tests and build
        working-directory: web
        run: |
          npm ci
          npm test
          npx playwright install --with-deps chromium
          npm run e2e
          npm run build

      - uses: actions/upload-pages-artifact@v3
        if: github.event_name == 'push' && github.ref == 'refs/heads/master'
        with:
          path: web/dist

  deploy:
    if: github.event_name == 'push' && github.ref == 'refs/heads/master'
    needs: build
    runs-on: ubuntu-latest
    permissions:
      pages: write
      id-token: write
    environment:
      name: github-pages
      url: ${{ steps.deployment.outputs.page_url }}
    steps:
      - id: deployment
        uses: actions/deploy-pages@v4
```

If an action major version above is deprecated at execution time, bump it to the current major (check the action's README); do not change anything else.

- [ ] **Step 2: Validate locally**

Run: `make clean && make test players && make wasm && cd web && npm ci && npm test && npm run e2e && npm run build; cd ..`
Expected: the same sequence as CI passes on a clean tree.

- [ ] **Step 3: Commit**

```bash
git add .github/workflows/ci.yml
git commit -m "Add CI and GitHub Pages deployment

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

- [ ] **Step 4: Hand over the remote steps to Laurent (do not do them)**

Tell Laurent, as a numbered list:
1. Pushing the branch and opening the PR need his explicit go (ask).
2. One-time setting he must change himself: GitHub repo → Settings → Pages → Source: **GitHub Actions**.

---

### Task 11: Recordings, performance check and README

**Files:**
- Create: `web/scripts/record.config.ts`, `web/scripts/record.spec.ts`, `web/scripts/to-gif.sh`
- Create: `docs/media/human-vs-hard.gif`, `docs/media/ai-vs-ai.gif`, `docs/media/how-it-thinks.gif` (+ `.mp4` each)
- Create: `README.md`; Delete: `README.txt`

**Interfaces:**
- Consumes: Task 9 URL parameters and `window.__gomoku`.
- Produces: `npm run record` (Playwright videos → `docs/media/raw/*.webm` + `hard-timings.json`, then GIF/MP4 in `docs/media/`).

- [ ] **Step 1: Write the recording scripts**

`web/scripts/record.config.ts`:

```ts
import { defineConfig, devices } from '@playwright/test';

const viewport = { width: 600, height: 860 };

export default defineConfig({
  testDir: '.',
  testMatch: 'record.spec.ts',
  timeout: 15 * 60_000,
  workers: 1,
  use: {
    ...devices['Desktop Chrome'],
    baseURL: 'http://localhost:5173',
    viewport,
    video: { mode: 'on', size: viewport },
  },
  webServer: {
    command: 'npx vite --port 5173 --strictPort',
    cwd: '..',
    url: 'http://localhost:5173',
    reuseExistingServer: true,
  },
});
```

`web/scripts/record.spec.ts`:

```ts
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
```

`web/scripts/to-gif.sh`:

```bash
#!/usr/bin/env bash
# Converts the Playwright recordings into GIF + MP4 for the README and the portfolio.
set -euo pipefail
cd "$(dirname "$0")/../.."

RAW=docs/media/raw
OUT=docs/media

convert() {
  local name=$1 speed=$2
  local input="$RAW/$name.webm"
  ffmpeg -loglevel error -y -i "$input" \
    -vf "setpts=PTS/$speed,fps=12,scale=480:-1:flags=lanczos,split[a][b];[a]palettegen=max_colors=64[p];[b][p]paletteuse=dither=bayer" \
    "$OUT/$name.gif"
  ffmpeg -loglevel error -y -i "$input" -an \
    -vf "setpts=PTS/$speed,scale=600:-2" -c:v libx264 -pix_fmt yuv420p -movflags +faststart \
    "$OUT/$name.mp4"
  echo "$name: $(du -k "$OUT/$name.gif" | cut -f1) KB gif, $(du -k "$OUT/$name.mp4" | cut -f1) KB mp4"
}

convert human-vs-hard 1.5
convert ai-vs-ai 2
convert how-it-thinks 1
```

```bash
chmod +x web/scripts/to-gif.sh
```

- [ ] **Step 2: Record and convert**

Run: `make wasm && cd web && npm run record; cd ..`
Expected: three tests pass, then three lines like `human-vs-hard: 1234 KB gif, 456 KB mp4`.
- Any GIF ≥ 3072 KB: lower `fps=12` to `fps=8` (and/or raise that recording's speed factor) in `to-gif.sh`, rerun `bash web/scripts/to-gif.sh`.
- Open the three GIFs and check that each shows the board, stones appearing and (for the two full games) the red winning line. Re-record if a GIF is empty or cut.

- [ ] **Step 3: Check the performance budget**

Run: `cat docs/media/raw/hard-timings.json`
Expected: `maxMs` < 5000. **If ≥ 5000: stop and report the numbers to Laurent** (proposed fix: a `set_max_childs` cap on 4.4, same pattern as `set_max_depth`); do not continue to the README.

- [ ] **Step 4: Write the README**

Delete `README.txt` (`git rm -q README.txt`) and create `README.md`, replacing `MAX_MS` with `maxMs` from `hard-timings.json` rounded up to the next 100 ms:

````markdown
# Gomoku AI

Play Gomoku against the minimax AI we wrote in C as students — now compiled to WebAssembly and running in your browser.

**[▶ Play online](https://laurentgenty.github.io/gomoku-AI/)**

![A game against the hard AI](docs/media/human-vs-hard.gif)

| AI vs AI — player 4.1 vs player 4.4 | The AI thinking |
|---|---|
| ![AI vs AI](docs/media/ai-vs-ai.gif) | ![The AI thinking](docs/media/how-it-thinks.gif) |

## How it works

- **Bitboard.** Each player's stones live in one 128-bit integer, one bit per cell (hence boards up to 11×11). Wins and full boards are checked with a few bitwise operations.
- **Negamax with alpha-beta pruning.** The AI explores a few moves ahead, assuming the opponent always plays their best answer, and skips branches that cannot change its choice.
- **Crossing-lines heuristic.** Each empty cell is scored from 0 to 13 from the lines it would create (open four, double open three…). Crossings score highest because they win games. Only the best-scored cells are searched, which keeps the tree small.
- **In the browser.** The original C is compiled with Emscripten into three WebAssembly modules: a referee (the rules) and two AI players. Each AI runs in its own Web Worker, so the page never freezes. TypeScript only draws the board and passes moves around.

On the hard level the AI answers in at most **MAX_MS ms** per move (Chromium, Apple Silicon Mac).

Full write-up, in French: [project report](doc/rapport.pdf).

## Levels

| Level | Player | Heuristic | Search depth |
|---|---|---|---|
| Easy | 4.1 | alignments only | 2 |
| Medium | 4.4 | crossing lines | 2 |
| Hard | 4.4 | crossing lines | 4 |

## Build

Native (C99, gcc or clang):

```bash
make test       # unit tests, 4.4 vs random, threaded vs sequential 4.4
make players    # the original players as shared libraries
make valgrind   # memory checks (Linux)
make doc        # Doxygen documentation
```

WebAssembly and web app (Emscripten, Node 22):

```bash
make wasm
cd web
npm ci
npm run dev     # local server
npm test        # unit + WebAssembly tests
npm run e2e     # browser smoke test
npm run record  # re-record the GIFs in docs/media
```

## Repository layout

| Path | Content |
|---|---|
| `src/server/` | Bitboard and move queue (rebuilt, see below) |
| `src/players/` | The original AI players and heuristics |
| `src/wasm/` | Thin Emscripten adapters |
| `src/test/` | C tests |
| `web/` | Vite + TypeScript front end |
| `doc/` | Report (LaTeX/PDF) and Doxygen configuration |

## About the rebuilt server code

The original `src/server/` sources were never committed. The bitboard was rebuilt from the project report, the existing tests and the way the players call it. `board__explore_line`, the pattern detection behind the 4.x heuristics, was rewritten and checked against the expectations of `src/test/test_player.c`. The original `dlopen`-based game server was not rebuilt: the web app plays its role.

## Credits

Semester 6 project at ENSEIRB-MATMECA by Emeric Duchemin, Laurent Genty, Julien Miens and Tanguy Pemeja. Special thanks to F. Herbreteau.
````

- [ ] **Step 5: Final verification**

Run: `make test && cd web && npm test && npx tsc && npm run build; cd .. && ls -l docs/media`
Expected: all green; `docs/media` holds 3 GIFs (< 3 MB each) and 3 MP4s; `git status` does not list `docs/media/raw/`.

- [ ] **Step 6: Commit**

```bash
git add web/scripts docs/media/*.gif docs/media/*.mp4 README.md
git add -u README.txt
git commit -m "Add recorded demos and rewrite the README

Co-Authored-By: Claude Opus 5.5 <noreply@anthropic.com>"
```

Then report to Laurent: what works (link the GIFs), the measured Hard timing, and ask before pushing / opening the PR. Copying the media into `~/perso/portfolio` is out of scope.
