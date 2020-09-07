/**
 * @file player_test.c
 * Player that plays alone and does both sides of the board. This player was useful in order to debug the server test in order to make statistics about wins and games in general. This player actually plays like player1.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "player.h"
#include "../server/move.h"

/**
 * @brief Random player who plays anywhere on the board
 */
struct player{
  /**
   * color of the player: id
   */
  enum color_t my_color;
  /**
   * size of the board
   */
  size_t size;
  /**
   * all moves allowed on the board
   */
  struct allowed_moves *a_m;
};

/**
 * @brief Table of moves allowed in the board. Its size changes through the game
 */
struct allowed_moves{
  /**
   * table of moves
   */
  struct move_t * move;
  /**
   * current size of the table
   */
  size_t size;
};

/* Struct for containing data by the player */
static struct player me;
int n = 0;

/* Internal functions */

/**
 * @brief Search for a specific move in an array and return the position in this array
 * @param[in] size size of the array
 * @param[in] T array of moves
 * @param[in] m move
 * @return position of the move in the array T if it exists, -1 otherwise
 */
int search_move(size_t size,struct move_t T[],struct move_t m){
  int count = 0;
  int tr = 1;
  while(tr){
    if(T[count].row == m.row && T[count].col == m.col){
      tr = 0;
    }
    count = count + 1;
  }
  return count-1;
}

/**
 * @brief Update the board of the player: fill all allowed moves for the player
 * @param[in] previous_moves last n moves done
 * @param[in] n_moves size of the previous_moves array
 * @return allowed moves of the player updated
 */
void update_own_board(struct col_move_t const previous_moves[], size_t n_moves){
  int n =0;
  for(int i=0; i<n_moves; i++){
    n = search_move(me.a_m->size,me.a_m->move,previous_moves[i].m);
    me.a_m->move[n] = me.a_m->move[me.a_m->size-1];
    me.a_m->size --;
  }
}

/**
 * @brief Update the previous move done in order to let the player plays in both sides
 * @param[in] previous_move previous move done
 * @return self player variable updated with the previous move added
 */
void update_one(struct move_t previous_move){
  n = search_move(me.a_m->size,me.a_m->move,previous_move);
  me.a_m->move[n] = me.a_m->move[me.a_m->size-1];
  me.a_m->size --;
}

/* External functions */

char const* get_player_name(){
  return "stupid player";
}

struct col_move_t* propose_opening(size_t size){
  struct col_move_t * ret = malloc(sizeof(*ret) * 3);
  //DO NOT FORGET TO FREE BY SERVER
  ret[0].m.row = 0;
  ret[0].m.col = 0;
  ret[0].c = BLACK;
  ret[1].m.row = 0;
  ret[1].m.col = 1;
  ret[1].c = WHITE;
  ret[2].m.row = 0;
  ret[2].m.col = 2;
  ret[2].c = BLACK;
  return ret;
}

int accept_opening(size_t size, const struct col_move_t* opening){
  return 1;
}

void initialize(size_t size, enum color_t id){
  me.a_m = malloc(sizeof(struct allowed_moves));
  me.size = size;
  me.my_color = id;
  me.a_m->size = size*size;
  me.a_m->move = malloc(sizeof(struct move_t)*size*size);

  for(int i=0;i<size;i++){
    for(int j=0;j<size;j++){
      me.a_m->move[size*i+j].row = i;
      me.a_m->move[size*i+j].col = j;
    }
  }
}
  
/*
void update_own_board(struct col_move_t const previous_moves[], size_t n_moves){
  int n =0;
  for(int i=0; i<n_moves; i++){
    n = search_move(me.a_m->size,me.a_m->move,previous_moves[i].m);
    me.a_m->move[n] = me.a_m->move[me.a_m->size-1];
    me.a_m->size --;
  }
}

void update_one(struct move_t previous_move){
  n = search_move(me.a_m->size,me.a_m->move,previous_move);
  me.a_m->move[n] = me.a_m->move[me.a_m->size-1];
  me.a_m->size --;
}
*/
struct move_t play(struct col_move_t const previous_moves[], size_t n_moves){
  if(me.a_m->size != 0){
    n = rand() % me.a_m->size;
  }
  else {
    n = 0;
  }
  struct move_t my_move = me.a_m->move[n];
  if(me.a_m->size != 0){
    update_one(my_move);
  }
  return my_move;
}


void finalize(){
  free(me.a_m->move);
  free(me.a_m);
}

