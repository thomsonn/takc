#ifndef _MOVE_H_
#define _MOVE_H_

#include <stdint.h>

typedef struct move_s {
    int index;
    uint32_t stones;
    uint32_t standing;
    uint32_t capstone;
    int less_normal;
    int less_capstones;
    char type;
} move_s;

#endif /*_MOVE_H_ */
