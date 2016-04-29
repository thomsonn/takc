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
    BLACK = 0,
    WHITE = 1
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
    board_s black, white;
    int player_to_move;
    int starting;
} state_s;

typedef struct move_s {
    int index;
    uint32_t stones;
    uint32_t standing;
    uint32_t capstone;
    int less_normal;
    int less_capstones;
    char type;
} move_s;

typedef struct node_s {
    move_s move;
    int player;
    int wins;
    int total;
} node_s;

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
    y = ((y >>  4) + y) & magic[2];
    y = ((y >>  8) + y) & magic[3];
    y = ((y >> 16) + y) & magic[4];

    return y;
}

int generate(int n)
{
    /* FIXME: Dumb random numbers */
    return rand()/(RAND_MAX/n + 1);
}

float generatef(float x)
{
    /* FIXME: Dumb random numbers */
    return ((float) rand())/((float) (RAND_MAX/x));
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
    return connect(board.stones ^ board.standing);
}

int count_unoccupied(state_s state)
{
    return count_bits(~(state.black.stones |
			state.white.stones) & BOARD);
}

board_s player_board(state_s state)
{
    return state.player_to_move ? state.white : state.black;
}

board_s opponent_board(state_s state)
{
    return state.player_to_move ? state.black : state.white;
}

move_s place_stone(state_s state, int index)
{
    /* TODO: Use a policy */
    switch (generate(9 + (player_board(state).num_capstones > 0))) {
    case 19:
	return (move_s) {.stones = 1 << index, .standing = 1 << index, .less_normal = 1, .type = 's'};
    case 20:
	return (move_s) {.stones = 1 << index, .capstone = 1 << index, .less_capstones = 1, .type = 'c'};
    default:
	return (move_s) {.stones = 1 << index, .less_normal = 1, .type = 'n'};
    }

    return (move_s) {.type = 'F'};
}

move_s move_stone(state_s state, int index)
{
    const int offset[] = {-1, 1, SIZE, -SIZE};
    const uint32_t nmask = 0x451;
    uint32_t legal;
    int dest;
    move_s res;

    /* Return failure if there are no legal moves */
    legal = index >= 5 ? nmask << (index - 5) : nmask >> (5 - index);
    if (1 << index & ~UMASK)
	legal &= DMASK;
    if (1 << index & ~DMASK)
	legal &= UMASK;
    legal &= ~(state.black.stones |
	       state.white.stones) & BOARD;
    /*
    legal &= ~(state.black.capstone |
	       state.white.capstone) & BOARD;
      if (!(player_board(state).capstone & 1 << index))
      legal &= ~(state.black.standing |
      state.white.standing) & BOARD;
    */
    if (!legal)
	return (move_s) {.type = 'F'};

    do
	dest = index + offset[generate(4)];
    while (!(legal & 1 << dest));

    res = (move_s) {.stones = 1 << index | 1 << dest, .type = 'm'};

    if (player_board(state).standing & 1 << index)
	res.standing = res.stones;
    if (player_board(state).capstone & 1 << index)
	res.capstone = res.stones;

    return res;
}

move_s pick_move(state_s state)
{
    move_s res;
    int index;

    do {
	index = generate(SIZE*SIZE);

	if (state.starting) {
	    if (1 << index & (~(state.black.stones | state.white.stones) & BOARD))
		res = (move_s) {.stones = 1 << index, .less_normal = 1, .type = 'n'};
	    else
		res = (move_s) {.type = 'F'};
	}
	else {
	    if (1 << index & player_board(state).stones) {
		res = move_stone(state, index);
		res = (move_s) {.type = 'F'};
	    }
	    else if (1 << index & (~(state.black.stones | state.white.stones) & BOARD) &&
		     player_board(state).num_normal > 0)
		res = place_stone(state, index);
	    else
		res = (move_s) {.type = 'F'};
	}
    }
    while (res.type == 'F');

    res.index = index;

    return res;
}

void apply_move(board_s *board, move_s move)
{
    board->stones ^= move.stones;
    board->standing ^= move.standing;
    board->capstone ^= move.capstone;
    board->num_normal -= move.less_normal;
    board->num_capstones -= move.less_capstones;
}

void step_move(state_s *state, move_s move)
{
    apply_move((board_s *) (((void *) &state->black) + (state->player_to_move != state->starting)*sizeof(board_s)), move);
    state->starting &= state->player_to_move == WHITE;
    state->player_to_move = !state->player_to_move;
}

char game_ended(state_s state)
{
    /* Check for road win */
    if (evaluate_roads(state.black))
	return '-';
    if (evaluate_roads(state.white))
	return '+';

    /* Check for flat win */
    if ((state.black.stones | state.white.stones) == BOARD ||
	(state.black.num_normal == 0 && state.white.num_normal == 0)) {
	int nblack = count_bits(state.black.stones & ~state.black.standing);
	int nwhite = count_bits(state.white.stones & ~state.white.standing);

	if (nblack == nwhite)
	    return '=';
	if (nblack > nwhite)
	    return '-';
	else
	    return '+';
    }

    return 0;
}

char rollout(state_s state)
{
    char res;

    while (!(res = game_ended(state))) {
	move_s move;

	move = pick_move(state);
	step_move(&state, move);
    }

    return res;
}

int select_ucb1(GNode *node)
{
    int best;
    float max = 0;

    for (int i = 0; i < (int) g_node_n_children(node); i++) {
	node_s *data;
	float eval;

	data = (node_s *) g_node_nth_child(node, i)->data;
	eval = ((float) data->wins)/((float) data->total);
	eval += 1.4*sqrtf(log2f((float) ((node_s *) node->data)->total)/((float) data->total));
	if (i == 0 || eval > max) {
	    max = eval;
	    best = i;
	}
    }

    return best;
}

int select_random(GNode *node)
{
    return generate(g_node_n_children(node));
}

void step_evaluation(GNode *root, state_s state)
{
    GNode *node;
    node_s *data;
    int player;

    /* Find a leaf */
    for (node = root; !G_NODE_IS_LEAF(node); step_move(&state, data->move)) {
	if ((int) g_node_n_children(node) >= count_unoccupied(state))
	    break;
	node = g_node_nth_child(node, select_ucb1(node));
	data = (node_s *) node->data;
    }

    if (!game_ended(state)) {
	move_s move;

	/* Add a new child */
	data = calloc(1, sizeof(node_s));
	data->player = !((node_s *) node->data)->player;

	do {
	    move = pick_move(state);

	    for (int i = 0; i < (int) g_node_n_children(node); i++) {
		data = (node_s *) g_node_nth_child(node, i)->data;
		if (!memcmp(&data->move, &move, sizeof(move_s)))
		    move.type = 'F';
	    }
	}
	while (move.type == 'F');

	data->move = move;
	step_move(&state, data->move);

	node = g_node_insert(node, 0, g_node_new(data));
    }

    player = data->player;

    /* Rollout the position */
    char result = rollout(state);

    /* Propagate back up the tree */
    while (!G_NODE_IS_ROOT(node)) {
	data = (node_s *) node->data;
	data->wins += (data->player == player)*(player ? result == '+' : result == '-');
	data->total++;
	node = node->parent;
    }
}

GNode *evaluate(GNode *root, state_s state)
{
    GNode *node;
    GNode *res = NULL;
    float max = 0;

    for (int i = 0; i < 20000; i++)
	step_evaluation(root, state);

    for (int i = 0; i < (int) g_node_n_children(root); i++) {
	node_s *data;
	float eval;

	node = g_node_nth_child(root, i);
	data = (node_s *) node->data;

	eval = ((float) data->wins)/((float) data->total);
	if (i == 0 || eval > max) {
	    max = eval;
	    res = node;
	}
    }

    return res;
}

GNode *make_move(GNode *root, state_s state)
{
    GNode *best;
    node_s *data;

    best = evaluate(root, state);
    data = (node_s *) root->data;
    printf("{n: %8u, w: %8u, t: %8u, p: %d} ", g_node_n_nodes(root, G_TRAVERSE_ALL), data->wins, data->total, data->player);
    g_node_unlink(best);

    free(root->data);
    g_node_destroy(root);

    return best;
}

char *print_move(move_s move)
{
    const char direction[] = {'<', 0, 0, 0, '+', 0, '-', 0, 0, 0, '>'};
    char *string = "";
    char *mstr = "";
    int row, col;

    col = move.index/5;
    row = 4 - move.index % 5;

    if (move.type == 'c')
	string = "C";
    if (move.type == 's')
	string = "S";

    if (move.type == 'm') {
	int board, n;

	board = move.stones ^ 1 << move.index;
	n = log2l(move.index >= 5 ? board >> (move.index - 5) : board << (5 - move.index));
	asprintf(&mstr, "%c", direction[n]);
    }

    asprintf(&string, "%s%c%c%s", string, 'a' + col, '1' + row, mstr);

    if (move.type == 'm')
	free(mstr);

    return string;
}

int main()
{
    GNode *search_tree;
    board_s empty = {
	.num_normal = MAX_NORMAL,
	.num_capstones = MAX_CAPSTONES
    };
    state_s game_state = {
	.black = empty,
	.white = empty,
	.player_to_move = WHITE,
	.starting = 1
    };
    node_s *data;
    int ply = 0;

    /* Seed the random number generator */
    //srand(4894);
    //srand(18062);
    srand(time(NULL));

    /* Create the root of the search tree */
    data = calloc(1, sizeof(node_s));
    data->player = BLACK;
    search_tree = g_node_new(data);

    printf("[Size \"5\"]\n\n");

    /* TODO: Be careful of the first round */
    do {
	char *string;

	if (ply % 2 == 0)
	    printf("%d. %s", ply/2 + 1, ply/2 + 1 < 10 ? " " : "");

	search_tree = make_move(search_tree, game_state);
	data = (node_s *) search_tree->data;
	step_move(&game_state, data->move);

	string = print_move(data->move);
	/*
	asprintf(&string, "%s {B: %u  W: %u}", string, game_state.black.stones, game_state.white.stones);
	// */

	printf("%s ", string);
	free(string);

	if (ply % 2 != 0)
	    printf("\n");

	fflush(stdout);

	ply++;
    }
    while (!game_ended(game_state));

    free(search_tree->data);
    g_node_destroy(search_tree);

    printf("\n\nB: %u\nW: %u\n",
	   game_state.black.stones,
	   game_state.white.stones);

    printf("\n\nBr: %u\nWr: %u\n",
	   game_state.black.stones ^ game_state.black.standing,
	   game_state.white.stones ^ game_state.white.standing);

    return 0;
}
