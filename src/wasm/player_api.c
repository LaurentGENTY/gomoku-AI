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
