#include "bitboard.h"

uint32_t bitboard_flip_vert(uint32_t x)
{
    return ((x >> 4) & BITBOARD_ROW1) |
	   ((x >> 2) & BITBOARD_ROW2) |
	    (x       & BITBOARD_ROW3) |
	   ((x << 2) & BITBOARD_ROW4) |
	   ((x << 4) & BITBOARD_ROW5);
}

uint32_t bitboard_flip_horz(uint32_t x)
{
    return ((x >> 20) & BITBOARD_COL1) |
	   ((x >> 10) & BITBOARD_COL2) |
      	    (x        & BITBOARD_COL3) |
	   ((x << 10) & BITBOARD_COL4) |
	   ((x << 20) & BITBOARD_COL5);
}

uint32_t bitboard_flip_diag(uint32_t x)
{
    return ((x >> 16) & 0x0000010) |
	   ((x >> 12) & 0x0000208) |
	   ((x >> 8)  & 0x0004104) |
           ((x >> 4)  & 0x0082082) |
	    (x        & 0x1041041) |
	   ((x << 4)  & 0x0820820) |
	   ((x << 8)  & 0x0410400) |
	   ((x << 12) & 0x0208000) |
      	   ((x << 16) & 0x0100000);
}

uint32_t bitboard_rotate(uint32_t x)
{
    return bitboard_flip_horz(bitboard_flip_diag(x));
}

int bitboard_connect_vert(uint32_t x)
{
    uint32_t v12, v54, h24, h33;

    v12 = x << 1 & BITBOARD_ROW2;
    v54 = x >> 1 & BITBOARD_ROW4;

    h24 = (v12 | v54) & x;

    h24 |= h24 << SIZE & x;
    h24 |= h24 << SIZE & x;
    h24 |= h24 << SIZE & x;
    h24 |= h24 << SIZE & x;

    h24 |= h24 >> SIZE & x;
    h24 |= h24 >> SIZE & x;
    h24 |= h24 >> SIZE & x;
    h24 |= h24 >> SIZE & x;

    h33 = h24 << 1 & BITBOARD_ROW3 & x;

    h33 |= h33 >> SIZE & x;
    h33 |= h33 >> SIZE & x;
    h33 |= h33 >> SIZE & x;
    h33 |= h33 >> SIZE & x;

    h33 |= h33 << SIZE & x;
    h33 |= h33 << SIZE & x;
    h33 |= h33 << SIZE & x;
    h33 |= h33 << SIZE & x;

    return (h33 & h24 >> 1 & BITBOARD_ROW3) != 0;
}

int bitboard_connect(uint32_t x)
{
    return bitboard_connect_vert(x) || bitboard_connect_vert(bitboard_rotate(x));
}

int bitboard_popcount(uint32_t x)
{
    const uint32_t magic[] = {0x1555555, 0x1333333, 0x10f0f0f, 0x0ff00ff, 0x000ffff};
    uint32_t y;

    y = x - (x >> 1 & magic[0]);
    y = ((y >> 2) & magic[1]) + (y & magic[1]);
    y = ((y >>  4) + y) & magic[2];
    y = ((y >>  8) + y) & magic[3];
    y = ((y >> 16) + y) & magic[4];

    return y;
}
