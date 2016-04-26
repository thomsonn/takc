#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

const int size = 5;

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

const int MAX_NORMAL = 21;

#define MAX_STACK 7

typedef enum move_type_s {PLACE_NORMAL, PLACE_STANDING, PLACE_CAPSTONE} move_type_s;

typedef struct board_s {
    unsigned int stones;
    unsigned int standing;
    unsigned int capstone;
    int cap_played;
    unsigned int stack[MAX_STACK];
} board_s;

typedef struct state_s {
    board_s black;
    board_s white;
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

    fill24 |= x & (fill24 << size);
    fill24 |= x & (fill24 << size);
    fill24 |= x & (fill24 << size);
    fill24 |= x & (fill24 << size);

    fill24 |= x & (fill24 >> size);
    fill24 |= x & (fill24 >> size);
    fill24 |= x & (fill24 >> size);
    fill24 |= x & (fill24 >> size);

    fill33 = x & (fill24 << 1) & ROW3;

    fill33 |= x & (fill33 >> size);
    fill33 |= x & (fill33 >> size);
    fill33 |= x & (fill33 >> size);
    fill33 |= x & (fill33 >> size);

    fill33 |= x & (fill33 << size);
    fill33 |= x & (fill33 << size);
    fill33 |= x & (fill33 << size);
    fill33 |= x & (fill33 << size);

    return (fill33 & (fill24 >> 1) & ROW3) != 0;
}

int connect(unsigned int x)
{
    return connect(x) || connect(rotate(x));
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

int count_unoccupied(state_s board)
{
    return count_bits(~(board.black.stones | board.white.stones));
}

int generate(int n)
{
    /* FIXME: Dumb random numbers */
    return rand()/(RAND_MAX/n + 1);
}

move_s pick_stone_pos(state_s state)
{
    move_type_s type;
    board_s board = !state.player_to_move ? state.black: state.white;
    int index;

    switch (generate(3 - board.cap_played)) {
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
    while ((1 << index) & (state.black.stones | state.white.stones));

    return (move_s) {.type = type, .stone_pos = index};
}

move_s pick_move(state_s state)
{
    move_s res;

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
	break;
    case PLACE_STANDING:
	board.stones |= 1 << move.stone_pos;
	board.standing |= 1 << move.stone_pos;
	break;
    case PLACE_CAPSTONE:
	board.stones |= 1 << move.stone_pos;
	board.capstone |= 1 << move.stone_pos;
	board.cap_played = 1;
	break;
    }

    return new_board;
}

int step_move(state_s state, move_s move) {
    state_s new_state = state;

    if (!state.player_to_move)
	new_state.black = apply_move(state.black, move);
    else
	new_state.white = apply_move(state.white, move);
    
    new_board.player_to_move = !new_board.player_to_move;

    return new_state;
}

int main(int argc, char *argv[])
{
    unsigned int *winning;
    int i, j;
    int total = 0;
    int cur;
    int removed = 0;

    winning = malloc(MAX_COUNT*sizeof(unsigned int));

    for (i = 0; i < MAX_COUNT; i++)
	if (connect(i) || connect(rotate(i)))
	    winning[total++] = i;

    printf("found: %d\n", total);

    for (cur = 0; cur < total; cur++) {
	int match = 0;
	for (j = 0; j < total; j++) {
	    match += (winning[cur] | winning[j]) == winning[cur];
	    if (match > 1) {
		// Mark for deletion
		winning[cur] = 0xdeadbeef;
		removed++;
		goto skip;
	    }
	}
	if (match == 1)
	    printf("%d\n", winning[cur]);

    skip:
	if (removed > 1e4) {
	    int next = 0;
	    for (i = 0; i < total; i++)
		if (winning[i] != 0xdeadbeef)
		    winning[next++] = winning[i];

	    assert(total == next + removed);
	    total = next;
	    cur -= removed;
	    removed = 0;
	}
    }
    
    free(winning);

    return 0;
}
