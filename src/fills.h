#ifndef _FILLS_H
#define _FILLS_H

#include <stdlib.h>
#include "world.h"

/**
 * EMPTY:          Clear all cells
 * EVEN_IN_ROW:    Every even cell (in-row) is alive
 * EVEN_IN_WORLD:  Every even cell (in-world) is alive
 * MOD_4:          cell_val = 0, 1, 2, 3, 0, 1, ...
 * ODD_IN_ROW:     Every odd cell (in-row) is alive
 * ODD_IN_WORLD:   Every odd cell (in-world) is alive
 * EVEN_ROW:       Every even row is alive
 * ODD_ROW:        Every odd row is alive
 * TEST_CELL:      Single cell at (0,0) in world is alive
 * FULL:           Full fill (all 3s)
 */
enum fill_type {
    EMPTY=0,
    TEST_CELL,
    EVEN_IN_ROW,
    EVEN_IN_WORLD,
    MOD_4,
    ODD_IN_ROW,
    ODD_IN_WORLD,
    EVEN_ROW,
    ODD_ROW,
    FULL
};
typedef enum fill_type fill_type;

void fill(world *w, fill_type fill);

#endif
/* vim: set ft=c : */
