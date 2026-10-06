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
