#ifndef _BITBOARD_H_
#define _BITBOARD_H_

#include <glib.h>
#include <stdint.h>

#include "globals.h"

#define BOARD 0x1ffffff
#define DMASK 0x1ef7bde
#define UMASK 0x0f7bdef

#define BITBOARD_ROW1 0x0108421
#define BITBOARD_ROW2 0x0210842
#define BITBOARD_ROW3 0x0421084
#define BITBOARD_ROW4 0x0842108
#define BITBOARD_ROW5 0x1084210

#define BITBOARD_COL1 0x000001f
#define BITBOARD_COL2 0x00003e0
#define BITBOARD_COL3 0x0007c00
#define BITBOARD_COL4 0x00f8000
#define BITBOARD_COL5 0x1f00000

uint32_t bitboard_flip_horz(uint32_t);
uint32_t bitboard_flip_vert(uint32_t);
uint32_t bitboard_flip_diag(uint32_t);
uint32_t bitboard_rotate(uint32_t);
int bitboard_connect(uint32_t);
int bitboard_popcount(uint32_t);

#endif /*_BITBOARD_H_ */
