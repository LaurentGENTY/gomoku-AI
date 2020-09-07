#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include "../server/game.h"
#include "../server/move.h"
#include "test_utils.h"

int test_board__initialize(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);

  int err = 0;
  for(size_t i = 0; i < board->capacity; i ++){
      if(board->b[i] != -1){
	err+=1;
      }
  }
  /* Test if each case has been initialized */
  TEST("initialization of the cases of the board",!err,act);
  /* Test if the capacity has been initialized */
  TEST("initialization of the capacity of the board",
       (board->capacity==SIZE*SIZE),act);
  /* Test if the size has been initialized */
  TEST("initialization of the size of the board",
       (board->size==0),act);

  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int test_board__is_full(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);

  board->size=board->capacity-1;
  // Test if the board is not full
  TEST("board is not full",\
       !board__is_full(board),act);

  board->size+=1;
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
  struct move_t move= {-1,-1};
  // Test if a non valide move isn't added in the board
  TEST("a non valid move don't change the board",
       (!(board__is_valid_move(board,move))),act);
  move.row=0;move.col=0;
  // Test if a valide move is added in the board
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,BLACK);
  TEST("a valid move change the board",
       (board->b[SIZE*move.row+move.col]==BLACK),act);
  // Test if when a move is played, he become invalid
  TEST("same move isn't valid",\
       (!board__is_valid_move(board,move)),act);
  move.row=4;move.col=2;
  (assert(board__is_valid_move(board,move)));
  board__add_move(board,move,BLACK);
  // Test if a valide move change the size of the board
  TEST("a valid move change the size of the board",
       (board->size=(SIZE+1)),act);

  END_TEST("-",act);
  assert(res!=0);
  board__free(board);
  return (res==tot);
}

int test_board__won(int act){
  INIT_TEST("-",act);
  struct board* board = board__initialize(SIZE);
  struct move_t move= {0,0};
  // Test if a empty board wins
  TEST("empty board doesn't win",
       !board__won(board,move),act);
  board__free(board);

  enum winning_lane { ROW=0, COL=1, LDIAG=2, RDIAG=3 };

  /* ROW,COL,LDIAG,RDIAG */
  int succ[] = {0,0,0,0};

  for(int i = 0;i<SIZE;i++){
    for(int j = 0;j<SIZE-NB_WIN;j++){
      /* Generate every winning row */
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
	board__add_move(board,move,BLACK);
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
	 board__add_move(board,move,BLACK);
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



int main(int argc,char* argv[]){
  FULL;
  INIT_TEST("*",1);
  TEST("initialization",test_board__initialize(act),1);
  TEST("board__is_full",test_board__is_full(act),1);
  TEST("board__is_valid_move",test_board__is_valid_move(act),1);
  TEST("board__won",test_board__won(act),1);
  END_TEST("*",1);
  return 0;
}
