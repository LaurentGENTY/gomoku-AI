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
