/**
 * @file list.h
 */

#ifndef LIST_H
#define LIST_H

#include "../server/move.h"

/**
 * @brief Implementation of a list with an extending table. It is usefull for players' internal structure in order to store played moves. Internal moves are move_t. In this structure you can just add elements but not remove them
 */
struct list{
  /**
   * Current length of the list
   */
  int len;
  /**
   * Maximal capacity of the list
   */
  int capacity;
  /**
   * Pointer for the list of move_t (in the heap)
   */
  struct move_t * s;
};

/**
 * @brief Return an empty list of move_t
 * @return an empty list of move_t
 */
struct list * list_generate();

/**
 * @brief Check if the list given in parameter is an empty list of move_t
 * @param[in] l list of moves
 * @return 1 if the list is empty, 0 otherwise
 */
int list_empty(struct list *l);

/**
 * @brief Return the length of list given in parameter
 * @param[in] l list
 * @return the size of the list given in parameter
 */
int list_length(struct list * l);

/**
 * @brief Add the move, given in parameter, in the list, also given in parameter. If the list is not big enough, the capacity will grow and the length too
 * @param[in/out] l list where the move will be added
 * @param[in] i row position
 * @param[in] j row position
 * @return the list given in parameter where the move created will be added
 */
void list_add(struct list * l, size_t i, size_t j);

/**
 * @brief Return the move number i in the list
 * @param[in] l list
 * @param[in] i position i in the list of moves
 * @return move_t if i is in en total length of the list, NULL otherwise
 */
struct move_t list_get(struct list * l, int i);

/**
 * @brief Verify if a specified move is in the list
 * @param[in] l list where we try to find the move
 * @param[in] mv move_t we are trying to find
 * @return 1 if the move_t is in the list, 0 otherwise
 */
int list_find(struct list* l, struct move_t mv);

/**
 * @brief Concatenate 2 lists of move_t into the first list without duplicates of the 2 lists. The second list is not freed
 * @param[in/out] l1 list where all moves will be added
 * @param[in] l2 list where all moves are taken. Not freed
 * @return l1 with all moves of the 2 lists fiven in parameter
 */
void list_concat(struct list * l1, struct list * l2);

/**
 * @brief Free the list given in parameter
 * @param[in/out] l1 list
 * @return the list is removed from the heap
 */
void list_free(struct list * l);

/**
 * @brief Cut the list given in parameter and remove the last nb moves in it
 * @param[in/out] l list
 * @param[in] nb the number of moves that will be removed
 * @return list without the last nb moves
 */
void list_cut(struct list * l, int nb);


#endif
