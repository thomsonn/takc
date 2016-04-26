#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_STACK 7

const int SIZE = 5;

const int MAX_NORMAL = 21;
const int MAX_CAPSTONES = 1;

const unsigned int DMASK = 0x1ef7bde;
const unsigned int UMASK = 0x0f7bdef;

const unsigned int ROW1 = 0x0108421;
const unsigned int ROW2 = 0x0210842;
const unsigned int ROW3 = 0x0421084;
const unsigned int ROW4 = 0x0842108;
const unsigned int ROW5 = 0x1084210;

const unsigned int COL1 = 0x000001f;
const unsigned int COL2 = 0x00003e0;
const unsigned int COL3 = 0x0007c00;
const unsigned int COL4 = 0x00f8000;
const unsigned int COL5 = 0x1f00000;

const unsigned int MAX_COUNT = 0x2000000;

typedef enum move_type_s {
    PLACE_NORMAL,
    PLACE_STANDING,
    PLACE_CAPSTONE
} move_type_s;

typedef enum end_type_s {
    END_ONGOING,
    END_BLACK,
    END_WHITE,
    END_TIE
} end_type_s;

typedef enum player_s {
    PLAYER_BLACK,
    PLAYER_WHITE,
} player_s;

typedef struct board_s {
    unsigned int stones;
    unsigned int standing;
    unsigned int capstone;
    int num_normal;
    int num_capstones;
    unsigned int stack[MAX_STACK];
} board_s;

typedef struct state_s {
    board_s boards[2];
    int player_to_move;
} state_s;

typedef struct move_s {
    move_type_s type;
    unsigned int stone_pos;
} move_s;

unsigned int flipv(unsigned int x)
{
    return ((x >> 4) & ROW1) |
	   ((x >> 2) & ROW2) |
	    (x       & ROW3) |
	   ((x << 2) & ROW4) |
	   ((x << 4) & ROW5);
}

unsigned int fliph(unsigned int x)
{
    return ((x >> 20) & COL1) |
	   ((x >> 10) & COL2) |
      	    (x        & COL3) |
	   ((x << 10) & COL4) |
	   ((x << 20) & COL5);
}

unsigned int flipd(unsigned int x)
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

unsigned int rotate(unsigned int board)
{
    return fliph(flipd(board));
}

int connect_vertical(unsigned int x)
{
    unsigned int prop12, prop45, fill24, fill33;
    
    prop12 = (x << 1) & ROW2;
    prop45 = (x >> 1) & ROW4;

    fill24 = x & (prop12 | prop45);

    fill24 |= x & (fill24 << SIZE);
    fill24 |= x & (fill24 << SIZE);
    fill24 |= x & (fill24 << SIZE);
    fill24 |= x & (fill24 << SIZE);

    fill24 |= x & (fill24 >> SIZE);
    fill24 |= x & (fill24 >> SIZE);
    fill24 |= x & (fill24 >> SIZE);
    fill24 |= x & (fill24 >> SIZE);

    fill33 = x & (fill24 << 1) & ROW3;

    fill33 |= x & (fill33 >> SIZE);
    fill33 |= x & (fill33 >> SIZE);
    fill33 |= x & (fill33 >> SIZE);
    fill33 |= x & (fill33 >> SIZE);

    fill33 |= x & (fill33 << SIZE);
    fill33 |= x & (fill33 << SIZE);
    fill33 |= x & (fill33 << SIZE);
    fill33 |= x & (fill33 << SIZE);

    return (fill33 & (fill24 >> 1) & ROW3) != 0;
}

int connect(unsigned int x)
{
    return connect_vertical(x) || connect_vertical(rotate(x));
}

int evaluate_roads(board_s board) {
    return connect(board.stones & ~board.standing);
}

int count_bits(unsigned int x)
{
    const unsigned int magic[] = {0x1555555, 0x1333333, 0x10f0f0f, 0x0ff00ff, 0x000ffff};
    unsigned int y;

    y = x - (x >> 1 & magic[0]);
    y = ((y >> 2) & magic[1]) + (y & magic[1]);
    y = (y >>  4) + y & magic[2];
    y = (y >>  8) + y & magic[3];
    y = (y >> 16) + y & magic[4];

    return y;
}

int count_unoccupied(state_s state)
{
    return count_bits(~(state.boards[PLAYER_BLACK].stones | state.boards[PLAYER_WHITE].stones));
}

int generate(int n)
{
    /* FIXME: Dumb random numbers */
    return rand()/(RAND_MAX/n + 1);
}

move_s pick_stone_pos(state_s state)
{
    move_type_s type;
    int index;

    switch (generate(2 + (state.boards[state.player_to_move].num_capstones > 0))) {
    case 0:
	type = PLACE_NORMAL;
	break;
    case 1:
	type = PLACE_STANDING;
	break;
    case 2:
	type = PLACE_CAPSTONE;
	break;
    }

    do {
	index = generate(25);
    }
    while ((1 << index) & (state.boards[PLAYER_BLACK].stones | state.boards[PLAYER_WHITE].stones));

    return (move_s) {.type = type, .stone_pos = index};
}

move_s pick_move(state_s state)
{
    move_s res;

    /* TODO: Remember to handle if player has no stones */
    switch (generate(1)) {
    case 0:
	res = pick_stone_pos(state);
	break;
    }

    return res;
}

board_s apply_move(board_s board, move_s move) {
    board_s new_board = board;
    
    switch (move.type) {
    case PLACE_NORMAL:
	board.stones |= 1 << move.stone_pos;
	board.num_normal--;
	break;
    case PLACE_STANDING:
	board.stones |= 1 << move.stone_pos;
	board.standing |= 1 << move.stone_pos;
	board.num_normal--;
	break;
    case PLACE_CAPSTONE:
	board.stones |= 1 << move.stone_pos;
	board.capstone |= 1 << move.stone_pos;
	board.num_capstones--;
	break;
    }

    return new_board;
}

state_s step_move(state_s state, move_s move) {
    state_s new_state = state;

    new_state.boards[state.player_to_move] = apply_move(state.boards[state.player_to_move], move);
    new_state.player_to_move = !new_state.player_to_move;

    return new_state;
}

end_type_s game_ended(state_s state) {
    /* Check for road win */
    if (evaluate_roads(state.boards[0]))
	return END_BLACK;
    if (evaluate_roads(state.boards[1]))
	return END_WHITE;

    /* Check for flat win */
    if (state.boards[0].num_normal == 0 && state.boards[1].num_normal == 0) {
	int nblack = count_bits(state.boards[0].stones & ~state.boards[0].standing);
	int nwhite = count_bits(state.boards[1].stones & ~state.boards[1].standing);

	if (nblack == nwhite)
	    return END_TIE;
	if (nblack > nwhite)
	    return END_BLACK;
	else
	    return END_WHITE;
    }

    return END_ONGOING;
}

int main(int argc, char *argv[])
{
    /* TODO: Implement stacks */
    board_s start = {
	.stones = 0,
	.standing = 0,
	.capstone = 0,
	.num_normal = MAX_NORMAL,
	.num_capstones = MAX_CAPSTONES
    };
    state_s game_state = {
	.boards[0] = start,
	.boards[1] = start,
	.player_to_move = 0
    };

    /* TODO: Be careful of the first round */

    return 0;
}
