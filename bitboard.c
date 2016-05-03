#include "bitboard.h"

uint32_t bitboard_flip_vert(uint32_t x)
{
    return ((x >> 20) & BITBOARD_COL1) |
	   ((x >> 10) & BITBOARD_COL2) |
      	    (x        & BITBOARD_COL3) |
	   ((x << 10) & BITBOARD_COL4) |
	   ((x << 20) & BITBOARD_COL5);
}

uint32_t bitboard_flip_horz(uint32_t x)
{
    return ((x >> 4) & BITBOARD_ROW1) |
	   ((x >> 2) & BITBOARD_ROW2) |
	    (x       & BITBOARD_ROW3) |
	   ((x << 2) & BITBOARD_ROW4) |
	   ((x << 4) & BITBOARD_ROW5);
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

#ifdef TEST_BITBOARD

#define PASTE(a, b, c, d, e, f) a ## b ## c ## d ## e ## f

void test_flip_vert()
{
    g_assert(bitboard_flip_vert(PASTE(0b,
				      10110,
				      10001,
				      11011,
				      11101,
				      01100)) ==
	     PASTE(0b,
		   01100,
		   11101,
		   11011,
		   10001,
		   10110));
    g_assert(bitboard_flip_vert(PASTE(0b,
				      11001,
				      10011,
				      10111,
				      10110,
				      10001)) ==
	     PASTE(0b,
		   10001,
		   10110,
		   10111,
		   10011,
		   11001));
}

void test_flip_horz()
{
    g_assert(bitboard_flip_horz(PASTE(0b,
				      10110,
				      10001,
				      11100,
				      00011,
				      01110)) ==
	     PASTE(0b,
		   01101,
		   10001,
		   00111,
		   11000,
		   01110));
    g_assert(bitboard_flip_horz(PASTE(0b,
				      00001,
				      10011,
				      10101,
				      01110,
				      11001)) ==
	     PASTE(0b,
		   10000,
		   11001,
		   10101,
		   01110,
		   10011));
}

void test_flip_diag()
{
    g_assert(bitboard_flip_diag(PASTE(0b,
				      01010,
				      00000,
				      11100,
				      01010,
				      00111)) ==
	     PASTE(0b,
		   00100,
		   10110,
		   00101,
		   10011,
		   00001));
    g_assert(bitboard_flip_diag(PASTE(0b,
				      11100,
				      01000,
				      11001,
				      00100,
				      01000)) ==
	     PASTE(0b,
		   10100,
		   11101,
		   10010,
		   00000,
		   00100));
}

void test_rotate()
{
    g_assert(bitboard_rotate(PASTE(0b,
				   01101,
				   11101,
				   00111,
				   01000,
				   00111)) ==
	     PASTE(0b,
		   00010,
		   01011,
		   10111,
		   10100,
		   10111));
    g_assert(bitboard_rotate(PASTE(0b,
				   01100,
				   11001,
				   11000,
				   11001,
				   00100)) ==
	     PASTE(0b,
		   01110,
		   01111,
		   10001,
		   00000,
		   01010));
}

void test_connect()
{
    g_assert( bitboard_connect(PASTE(0b,
				     00100,
				     11110,
				     01100,
				     11111,
				     11111)));
    g_assert(!bitboard_connect(PASTE(0b,
				     01001,
				     01100,
				     11100,
				     11001,
				     00010)));
    g_assert( bitboard_connect(PASTE(0b,
				     00111,
				     00101,
				     10100,
				     10101,
				     01111)));
    g_assert(!bitboard_connect(PASTE(0b,
				     00110,
				     11100,
				     01110,
				     01110,
				     00000)));
    g_assert(!bitboard_connect(PASTE(0b,
				     00010,
				     01110,
				     11101,
				     10010,
				     00111)));
}

void test_popcount()
{
    g_assert(bitboard_popcount(PASTE(0b,
				     01111,
				     00010,
				     01011,
				     00011,
				     11101)) == 14);
    g_assert(bitboard_popcount(PASTE(0b,
				     11111,
				     11001,
				     00001,
				     00100,
				     10101)) == 13);
}

int main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/bitboard/flip_vert", test_flip_vert);
    g_test_add_func("/bitboard/flip_horz", test_flip_horz);
    g_test_add_func("/bitboard/flip_diag", test_flip_diag);
    g_test_add_func("/bitboard/rotate", test_rotate);
    g_test_add_func("/bitboard/connect", test_connect);
    g_test_add_func("/bitboard/popcount", test_popcount);

    return g_test_run();
}

#endif
