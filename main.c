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

unsigned int flipv(unsigned int board)
{
    return ((board >> 4) & ROW1) |
	   ((board >> 2) & ROW2) |
	    (board       & ROW3) |
	   ((board << 2) & ROW4) |
	   ((board << 4) & ROW5);
}

unsigned int fliph(unsigned int board)
{
    return ((board >> 20) & COL1) |
	   ((board >> 10) & COL2) |
      	    (board        & COL3) |
	   ((board << 10) & COL4) |
	   ((board << 20) & COL5);
}

unsigned int flipd(unsigned int board)
{
    return ((board >> 16) & 0x0000010) |
	   ((board >> 12) & 0x0000208) |
	   ((board >> 8)  & 0x0004104) |
           ((board >> 4)  & 0x0082082) |
	    (board        & 0x1041041) |
	   ((board << 4)  & 0x0820820) |
	   ((board << 8)  & 0x0410400) |
	   ((board << 12) & 0x0208000) |
      	   ((board << 16) & 0x0100000);
}

unsigned int rotate(unsigned int board)
{
    return fliph(flipd(board));
}

int connect(unsigned int board)
{
    unsigned int bd, bu, f24, f3;
    
    bd = (board << 1) & ROW2;
    bu = (board >> 1) & ROW4;

    f24 = board & (bd | bu);

    f24 |= board & (f24 << size);
    f24 |= board & (f24 << size);
    f24 |= board & (f24 << size);
    f24 |= board & (f24 << size);

    f24 |= board & (f24 >> size);
    f24 |= board & (f24 >> size);
    f24 |= board & (f24 >> size);
    f24 |= board & (f24 >> size);

    f3 = board & (f24 << 1) & ROW3;

    f3 |= board & (f3 >> size);
    f3 |= board & (f3 >> size);
    f3 |= board & (f3 >> size);
    f3 |= board & (f3 >> size);

    f3 |= board & (f3 << size);
    f3 |= board & (f3 << size);
    f3 |= board & (f3 << size);
    f3 |= board & (f3 << size);

    return f3 & (f24 >> 1) & ROW3;
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
	if (connect(i) != 0 || connect(rotate(i)) != 0)
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
