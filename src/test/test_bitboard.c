#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include "../server/bitboard.h"
#include "../server/move.h"
#include "test_utils.h"

//Select the value of the bit of the board in the position
extern __uint128_t board__select_bit(__uint128_t* b,int pos);

int test_board__initialize(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  int err_w = 0;
  int err_b = 0;
  for(size_t i = 1; i < SIZE*SIZE+1; i ++){
      if(board__select_bit(board->b_w,i)!=0){
	err_w+=1;
      }
      if(board__select_bit(board->b_b,i)!=0){
	err_b+=1;
      }
  }
  for(size_t i = SIZE*SIZE+1; i < 129; i ++){
    if(board__select_bit(board->b_w,i)!=1){
      err_w+=1;
    }
    if(board__select_bit(board->b_b,i)!=1){
	err_b+=1;
      }
  }
  // Test if each case of the white board has been initialized
  TEST("initialization of the cases of the white board",\
       !err_w,act);
  // Test if each case of the black board has been initialized
  TEST("initialization of the cases of the white board",\
       !err_b,act);
  // Test if the capacity has been initialized
  TEST("initialization of the capacity of the board",\
       (board->capacity==SIZE*SIZE),act);
  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int test_board__is_full(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  *(board->b_w)=-1-7;
  *(board->b_b)=1;
  // Test if the board is not full
  TEST("board is not full",\
       !board__is_full(board),act);
  *(board->b_b)+=6;
  // Test if the board is full
  TEST("board is full",\
       board__is_full(board),act);

  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int test_board__is_valid_move(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  struct move_t move1= {-1,-1};
  struct move_t move2= {3,2};
  struct move_t move3= {0,0};
  struct move_t move4= {9,9};
  // Test if a non valide move isn't added in the board
  TEST("a non valid move don't change the board",\
       (!(board__is_valid_move(board,move1))),act);
  // Test if a valide black move is added only in the black board
  board__add_move(board,move2,BLACK);
  TEST("a valid black move change only the black board",
       ((board__select_bit(board->b_w,SIZE*move2.row+move2.col+1)==0)&&
	(board__select_bit(board->b_b,SIZE*move2.row+move2.col+1)==1)),act);
  // Test if when a move is played, he become invalid
  TEST("same move isn't valid",\
       (!(board__is_valid_move(board,move2))),act);
  // Test if a valide white move is added only in the white board
  // and didn't change the black board
  board__add_move(board,move3,WHITE);
  TEST("a valid white move change only the white board",		\
       ((board__select_bit(board->b_w,SIZE*move3.row+move3.col+1)==1)&&	\
	(board__select_bit(board->b_b,SIZE*move3.row+move3.col+1)==0)),act);
  // Test if a valide move don't erase the previous moves
  board__add_move(board,move4,BLACK);
  TEST("a valid move don't erase the previous moves",			\
       ((board__select_bit(board->b_b,SIZE*move4.row+move4.col+1)==1)&&		\
	(board__select_bit(board->b_b,SIZE*move2.row+move2.col+1)==1)),act);
  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int test_board__won(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  struct move_t move= {-1,-1};
  // Test if a empty board wins
  TEST("empty board doesn't win",\
       !board__won(board,move),act);
  board__free(board);

  enum winning_lane { ROW=0, COL=1, LDIAG=2, RDIAG=3 };
  int succ[] = {0,0,0,0}; // ROW,COL,LDIAG,RDIAG
  for(int i = 0;i<SIZE;i++){
    for(int j = 0;j<SIZE-NB_WIN;j++){

      // Generate every winning row
      board = board__initialize(SIZE);
      for(int k = j;k<j+NB_WIN;k++){
	move.row=i;move.col=k;
	(assert(board__is_valid_move(board,move)));
	board__add_move(board,move,BLACK);
      }
      succ[ROW]+=board__won(board,move); // Test if the current winning row wins
      board__free(board);
      // Generate every winning column

      board = board__initialize(SIZE);
      for(int k = j;k<j+NB_WIN;k++){
	move.row=k;move.col=i;
	(assert(board__is_valid_move(board,move)));
	board__add_move(board,move,WHITE);
      }
      succ[COL]+=board__won(board,move); // Test if the current winning column wins
      board__free(board);
    }
  }
  // Test if a board with a winning row wins
  TEST("winning row board wins",		\
       (succ[ROW]==(SIZE-NB_WIN)*SIZE),act);
  // Test if a board with a winning row wins
  TEST("winning column board wins",		\
       (succ[COL]==(SIZE-NB_WIN)*SIZE),act);
  for(int i = NB_WIN-1;i<SIZE;i++){
    for(int j = NB_WIN-1;j<SIZE;j++){
      // Generate every winning North-West diagonal
      board = board__initialize(SIZE);
      for(int k = 0;k<NB_WIN;k++){
	move.row=i-k;move.col=j-k;
	(assert(board__is_valid_move(board,move)));
	board__add_move(board,move,BLACK);
      }
      succ[LDIAG]+=board__won(board,move); // Test if the current winning
                               // North-West diagonal wins
      board__free(board);
    }
  }
  // Test if a board with a winning North-West diagonal wins
  TEST("winning North-West diagonal board wins",		\
       (succ[LDIAG]==(SIZE-NB_WIN+1)*(SIZE-NB_WIN+1)),act);

   for(int i = NB_WIN-1;i<SIZE;i++){
     for(int j = 0;j<SIZE-NB_WIN+1;j++){
       // Generate every winning column
       board = board__initialize(SIZE);
       for(int k = 0;k<NB_WIN;k++){
	 move.row=i-k;move.col=j+k;
	 (assert(board__is_valid_move(board,move)));
	 board__add_move(board,move,WHITE);
       }
       succ[RDIAG]+=board__won(board,move); // Test if the current winning
                                // North-East diagonal wins
       board__free(board);
     }
   }
   // Test if a board with a winning North-West diagonal wins
   TEST("winning North-East diagonal board wins",	\
	(succ[RDIAG]==(SIZE-NB_WIN+1)*(SIZE-NB_WIN+1)),act);
   END_TEST("-",act);
   //printf("%d %d %d %d\n",succ[ROW],succ[COL],succ[LDIAG],succ[RDIAG]);
   assert(res!=0);
   return (res==tot);
}

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

int main(int argc,char* argv[]){
  //clock_t temps;
  //srand(time(NULL));
  FULL;
  INIT_TEST("*",1);
  TEST("initialization",test_board__initialize(act),1);
  TEST("board__is_full",test_board__is_full(act),1);
  TEST("board__is_valid_move",test_board__is_valid_move(act),1);
  TEST("board__won",test_board__won(act),1);
  TEST("board__explore_line",test_explore__line(act),1);
  END_TEST("*",1);
  // printf("Bravo !\nTu as mis %f secondes a trouver le nombre.\n", (double) temps/CLOCKS_PER_SEC);
  return (res == tot) ? EXIT_SUCCESS : EXIT_FAILURE;
}
