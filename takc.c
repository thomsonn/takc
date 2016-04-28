#include <assert.h>
#include <glib.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_STACK 7

const int SIZE = 5;

const int MAX_NORMAL = 21;
const int MAX_CAPSTONES = 1;

const uint32_t BOARD = 0x1ffffff;
const uint32_t DMASK = 0x1ef7bde;
const uint32_t UMASK = 0x0f7bdef;

const uint32_t ROW1 = 0x0108421;
const uint32_t ROW2 = 0x0210842;
const uint32_t ROW3 = 0x0421084;
const uint32_t ROW4 = 0x0842108;
const uint32_t ROW5 = 0x1084210;

const uint32_t COL1 = 0x000001f;
const uint32_t COL2 = 0x00003e0;
const uint32_t COL3 = 0x0007c00;
const uint32_t COL4 = 0x00f8000;
const uint32_t COL5 = 0x1f00000;

typedef enum colour {
    BLACK,
    WHITE
} colour;

/* The bitboard representation for stones is as follows:
 *
 * ----------------
 * |00|05|10|15|20|
 * ----------------
 * |01|06|11|16|21|
 * ----------------
 * |02|07|12|17|22|
 * ----------------
 * |03|08|13|18|23|
 * ----------------
 * |04|09|14|19|24|
 * ----------------
 */

typedef struct board_s {
    uint32_t stones;
    uint64_t stacks[MAX_STACK];
    uint32_t standing;
    uint32_t capstone;
    int num_normal;
    int num_capstones;
} board_s;

typedef struct state_s {
    board_s *boards;
    int player_to_move;
    board_s *player_board;
    board_s *opponent_board;
} state_s;

typedef struct move_s {
    uint32_t from, to;
    char type;
} move_s;

unsigned int flip_vert(unsigned int x)
{
    return ((x >> 4) & ROW1) |
	   ((x >> 2) & ROW2) |
	    (x       & ROW3) |
	   ((x << 2) & ROW4) |
	   ((x << 4) & ROW5);
}

unsigned int flip_horz(unsigned int x)
{
    return ((x >> 20) & COL1) |
	   ((x >> 10) & COL2) |
      	    (x        & COL3) |
	   ((x << 10) & COL4) |
	   ((x << 20) & COL5);
}

unsigned int flip_diag(unsigned int x)
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
    return flip_horz(flip_diag(board));
}

int count_bits(unsigned int x)
{
    const unsigned int magic[] = {0x1555555, 0x1333333, 0x10f0f0f, 0x0ff00ff, 0x000ffff};
    unsigned int y;

    y = x - (x >> 1 & magic[0]);
    y = ((y >> 2) & magic[1]) + (y & magic[1]);
    y = (y >>  4) + (y & magic[2]);
    y = (y >>  8) + (y & magic[3]);
    y = (y >> 16) + (y & magic[4]);

    return y;
}

int generate(int n)
{
    /* FIXME: Dumb random numbers */
    return rand()/(RAND_MAX/n + 1);
}

int connect_vert(uint32_t x)
{
    uint32_t v12, v54, h24, h33;
    
    v12 = x << 1 & ROW2;
    v54 = x >> 1 & ROW4;

    h24 = (v12 | v54) & x;

    h24 |= h24 << SIZE & x;
    h24 |= h24 << SIZE & x;
    h24 |= h24 << SIZE & x;
    h24 |= h24 << SIZE & x;

    h24 |= h24 >> SIZE & x;
    h24 |= h24 >> SIZE & x;
    h24 |= h24 >> SIZE & x;
    h24 |= h24 >> SIZE & x;

    h33 = h24 << 1 & ROW3 & x;

    h33 |= h33 >> SIZE & x;
    h33 |= h33 >> SIZE & x;
    h33 |= h33 >> SIZE & x;
    h33 |= h33 >> SIZE & x;
	                 
    h33 |= h33 << SIZE & x;
    h33 |= h33 << SIZE & x;
    h33 |= h33 << SIZE & x;
    h33 |= h33 << SIZE & x;

    return (h33 & h24 >> 1 & ROW3) != 0;
}

int connect(uint32_t x)
{
    return connect_vert(x) || connect_vert(rotate(x));
}

int evaluate_roads(board_s board) {
    return connect(board.stones & ~board.standing);
}

int count_unoccupied(state_s state)
{
    return count_bits(~(state.boards[BLACK].stones |
			state.boards[WHITE].stones));
}

move_s place_stone(state_s state, int index)
{
    char type;

    /* TODO: Use a policy */
    switch (generate(2 + (state.player_board->num_capstones > 0))) {
    case 0:
	type = 'n';
	break;
    case 1:
	type = 's';
	break;
    case 2:
	type = 'c';
	break;
    default:
	type = 'F';
	break;
    }

    return (move_s) {.to = 1 << index, .type = type};
}

move_s move_stone(state_s state, int index)
{
    const int offset[] = {-1, 1, SIZE, -SIZE};
    const uint32_t nmask = 0x451;
    uint32_t legal;
    int dest;

    /* Return failure if there are no legal moves */
    legal = (nmask << index & (~(state.boards[BLACK].standing |
				 state.boards[WHITE].standing) & BOARD) << 5) >> 5;
    if (!legal)
	return (move_s) {.type = 'F'};
    
    do
	dest = index + offset[generate(4)];
    while (!(1 << dest & legal));
    
    return (move_s) {.from = index, .to = dest, .type = 'm'};
}

move_s pick_move(state_s state)
{
    move_s res;
    int index;

    /* TODO: Remember to handle if player has no stones */
    /* TODO: Handle forced place by rules on first round */
    do {
	index = generate(SIZE*SIZE);

	if (1 << index & state.player_board->stones)
	    res = move_stone(state, index);
	else if (1 << index & ~(state.boards[BLACK].stones | state.boards[WHITE].stones))
	    res = place_stone(state, index);
	else
	    continue;
    }
    while (res.type == 'F');

    return res;
}

board_s apply_move(board_s board, move_s move) {
    board_s new_board = board;
    
    switch (move.type) {
    case 'n':
	new_board.stones |= move.to;
	new_board.num_normal--;
	break;
    case 's':
	new_board.stones |= move.to;
	new_board.standing |= move.to;
	new_board.num_normal--;
	break;
    case 'c':
	new_board.stones |= move.to;
	new_board.capstone |= move.to;
	new_board.num_capstones--;
	break;
    case 'm':
	new_board.stones ^= move.from;
	new_board.stones |= move.to;
	new_board.standing ^= board.standing & move.from;
	new_board.standing |= board.standing & move.to;
	new_board.capstone ^= board.capstone & move.from;
	new_board.capstone |= board.capstone & move.to;
    }

    return new_board;
}

state_s step_move(state_s state, move_s move) {
    state_s new_state = state;

    *new_state.player_board = apply_move(*state.player_board, move);
    new_state.player_to_move = !new_state.player_to_move;
    new_state.player_board = &new_state.boards[state.player_to_move];
    new_state.opponent_board = &new_state.boards[!state.player_to_move];

    return new_state;
}

char game_ended(state_s state) {
    /* Check for road win */
    if (evaluate_roads(state.boards[BLACK]))
	return '-';
    if (evaluate_roads(state.boards[WHITE]))
	return '+';

    /* Check for flat win */
    if (state.boards[BLACK].num_normal == 0 &&
	state.boards[WHITE].num_normal == 0) {
	int nblack = count_bits(state.boards[BLACK].stones & ~state.boards[BLACK].standing);
	int nwhite = count_bits(state.boards[WHITE].stones & ~state.boards[WHITE].standing);

	if (nblack == nwhite)
	    return '=';
	if (nblack > nwhite)
	    return '-';
	else
	    return '+';
    }

    return 0;
}

int main()
{
    /* TODO: Implement stacks */
    board_s start = {
	.num_normal = MAX_NORMAL,
	.num_capstones = MAX_CAPSTONES
    };
    state_s game_state = {
	.boards = (board_s[2]) {start, start},
	.player_to_move = BLACK
    };

    /* Seed the random number generator */
    srand(4894);
    
    game_state.player_board = &game_state.boards[game_state.player_to_move];
    game_state.player_board = &game_state.boards[!game_state.player_to_move];

    /* TODO: Be careful of the first round */
    do {
	move_s move;

	move = pick_move(game_state);
	game_state = step_move(game_state, move);

	printf("B: %d\nW: %d\n\n",
	       game_state.boards[BLACK].stones,
	       game_state.boards[WHITE].stones);
    }
    while (!game_ended(game_state));

    return 0;
}
