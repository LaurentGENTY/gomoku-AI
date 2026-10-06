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
