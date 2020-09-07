/**
 * @file player2.c
 * Implementation of the interactive player: you choose in the terminal which position you want to play.
 * This player was useful at the beginning of the project in order to debug and try to play. Not useful anymore...
 */
#include <stdio.h>
#include <stdlib.h>

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
};


/* Struct for containing data by the player */
static struct player me;

/* List of colors of players */
static char * colors[] = {"BLACK", "WHITE"};

/* Internal functions */

/**
 * @brief Debugging function in order to print a move
 * @param[in] mv move
 */
void print_move(struct col_move_t mv){
  printf("%s in %ld %ld\n", colors[mv.c], mv.m.row, mv.m.col);
}

/* External functions */

char const* get_player_name(){
  return "interactive player";
}

struct col_move_t* propose_opening(size_t size){
  struct col_move_t * ret = malloc(sizeof(*ret) * 3);
  printf("You can propose an opening\n");
  printf("first black move : \n");
  for(int i = 0; i < 3;i++){
    ret[i].c = i % 2;
    printf("x = ");
    scanf("%ld", &ret[i].m.row);
    printf("y = ");
    scanf("%ld", &ret[i].m.col);
  }
  return ret;
}

int accept_opening(size_t size, const struct col_move_t* opening){
  printf("Do you accept this opening ? (y/n)\n");
  for(size_t i = 0; i < 3; i++){
    print_move(opening[i]);
  }
  char rep[10];
  while(1){
    scanf("%s", rep);
    switch (rep[0]){
        case 'y' :
          return 1;
          break;
        case 'n' :
          return 0;
          break;
    }
  }
}

void initialize(size_t size, enum color_t id){
  me.size = size;
  me.my_color = id;
}

struct move_t play(struct col_move_t const previous_moves[], size_t n_moves){
  struct move_t my_move = {0,0};
  if(n_moves ==1 ){
    printf("the other player just played in %ld %ld\n", previous_moves[0].m.row,
                                                    previous_moves[0].m.col);
  }
  else {
    printf("the other player just played in %ld %ld\n", previous_moves[1].m.row,
                                                    previous_moves[1].m.col);
  }
  printf("Where do you want to play ?\n");
  printf("x = ");
  scanf("%ld", &my_move.row);
  printf("y = ");
  scanf("%ld", &my_move.col);
  return my_move;
}

void finalize(){}
