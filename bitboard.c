#include "bitboard.h"

uint32_t bitboard_flip_horz(uint32_t x)
{
    uint32_t y;

    y = (x ^ x >> 20) & 0x000001f;
    x ^= y ^ y << 20;
    y = (x ^ x >> 10) & 0x00003e0;
    x ^= y ^ y << 10;

    return x;
}

uint32_t bitboard_flip_vert(uint32_t x)
{
    uint32_t y;

    y = (x ^ x >> 4) & 0x0108421;
    x ^= y ^ y << 4;
    y = (x ^ x >> 2) & 0x0210842;
    x ^= y ^ y << 2;

    return x;
}

uint32_t bitboard_flip_diag(uint32_t x)
{
    uint32_t y;

    y = (x ^ x >> 12) & 0x0000318;
    x ^= y ^ y << 12;
    y = (x ^ x >> 8) & 0x0004004;
    x ^= y ^ y << 8;
    y = (x ^ x >> 4) & 0x0092092;
    x ^= y ^ y << 4;

    return x;
}

uint32_t bitboard_rotate(uint32_t x)
{
    return bitboard_flip_diag(bitboard_flip_horz(x));
}

static uint32_t connect_cols(uint32_t x, uint32_t y)
{
    uint32_t result = 0;

    while (y != 0) {
	uint32_t w, r, s;

	/* x = 0b0100100100100010 */
	/* y = 0b0111111000001100 */
	/* Isolate the lowest 1 bit in y */
	s = y & -y;
	/* s = 0b0000000000000100 */
	r = y + s;
	/* r = 0b0111111000010000 */
	/* Isolate the lowest consecutive group of 1s in y */
	w = y ^ r ^ (r & -r);
	/* w = 0b0000000000001100 */
	if ((x & w) != 0)
	    result |= w;
	/* Unset w from y */
	y ^= w;
	/* y = 0b0111111000000000 */
    }

    return result;
}

static int connect_horz(uint32_t x)
{
    uint32_t cols1254;
    uint32_t cols23;

    /* Connect roads from column 1 to 2 and from column 5 to 4, in parallel */
    cols1254 = connect_cols((x << SIZE & BITBOARD_COL2) | (x >> SIZE & BITBOARD_COL4),
			    (x & BITBOARD_COL2) | (x & BITBOARD_COL4));
    /* Connect roads from column 2 to 3 */
    cols23 = connect_cols(cols1254 << SIZE, x & BITBOARD_COL3);

    /* Return the bitwise AND of columns 3 and 4 */
    return (cols23 << SIZE & cols1254) != 0;
}

int bitboard_connect(uint32_t x)
{
    return connect_horz(x) || connect_horz(bitboard_rotate(x));
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

static void test_flip_horz()
{
    g_assert(bitboard_flip_horz(PASTE(0b, 11100, 00100, 00111, 00000, 00000)) ==
                        	PASTE(0b, 00000, 00000, 00111, 00100, 11100));
    g_assert(bitboard_flip_horz(PASTE(0b, 00100, 00110, 00010, 00011, 00001)) ==
                 	        PASTE(0b, 00001, 00011, 00010, 00110, 00100));
}

static void test_flip_vert()
{
    g_assert(bitboard_flip_vert(PASTE(0b, 10000, 11110, 00010, 00010, 00010)) ==
	                        PASTE(0b, 00001, 01111, 01000, 01000, 01000));
    g_assert(bitboard_flip_vert(PASTE(0b, 00001, 00011, 00010, 00110, 00100)) ==
	                        PASTE(0b, 10000, 11000, 01000, 01100, 00100));
}

static void test_flip_diag()
{
    g_assert(bitboard_flip_diag(PASTE(0b, 00001, 00001, 00011, 01110, 01000)) ==
	                        PASTE(0b, 00000, 00011, 00010, 00110, 11100));
    g_assert(bitboard_flip_diag(PASTE(0b, 01111, 01000, 11000, 00000, 00000)) ==
	                        PASTE(0b, 00100, 11100, 10000, 10000, 10000));
}

static void test_rotate()
{
    g_assert(bitboard_rotate(PASTE(0b, 00000, 01110, 01010, 11011, 00000)) ==
	                     PASTE(0b, 01000, 01110, 00010, 01110, 01000));
    g_assert(bitboard_rotate(PASTE(0b, 00100, 00100, 00110, 00011, 00001)) ==
	                     PASTE(0b, 00000, 00000, 00111, 01100, 11000));
}

static void test_connect()
{
    g_assert( bitboard_connect(PASTE(0b, 00100, 11110, 01100, 11111, 11111)));
    g_assert( bitboard_connect(PASTE(0b, 00111, 00101, 10100, 10101, 01111)));
    g_assert(!bitboard_connect(PASTE(0b, 01001, 01100, 11100, 11001, 00010)));
    g_assert(!bitboard_connect(PASTE(0b, 00110, 11100, 01110, 01110, 00000)));
    g_assert(!bitboard_connect(PASTE(0b, 00010, 01110, 11101, 10010, 00111)));
}

static void test_popcount()
{
    g_assert(bitboard_popcount(PASTE(0b, 00100, 01100, 11000, 10000, 10000)) == 7);
    g_assert(bitboard_popcount(PASTE(0b, 00000, 00000, 00011, 11110, 00000)) == 6);
    g_assert(bitboard_popcount(PASTE(0b, 10000, 10000, 10000, 10000, 10000)) == 5);
}

int main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_test_add_func("/bitboard/flip_horz", test_flip_horz);
    g_test_add_func("/bitboard/flip_vert", test_flip_vert);
    g_test_add_func("/bitboard/flip_diag", test_flip_diag);
    g_test_add_func("/bitboard/rotate", test_rotate);
    g_test_add_func("/bitboard/connect", test_connect);
    g_test_add_func("/bitboard/popcount", test_popcount);

    return g_test_run();
}

#endif
