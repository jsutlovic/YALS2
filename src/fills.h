#ifndef _FILLS_H
#define _FILLS_H

#include <stdlib.h>
#include "world.h"

enum fill_type {
    FULL=0,
    EVEN_IN_ROW,
    EVEN_IN_WORLD,
    MOD_4,
    ODD_IN_ROW,
    ODD_IN_WORLD,
    EVEN_ROW,
    ODD_ROW,
};
typedef enum fill_type fill_type;

void fill0(world *w); // Full fill (all 3s)
void fill1(world *w); // Every even cell (in-row) is alive
void fill2(world *w); // Every even cell (in-world) is alive
void fill3(world *w); // cell_val = 0, 1, 2, 3, 0, 1, ...
void fill4(world *w); // Every odd cell (in-row) is alive
void fill5(world *w); // Every odd cell (in-world) is alive
void fill6(world *w); // Every even row is alive
void fill7(world *w); // Every odd row is alive
void fill(world *w, fill_type fill);

#endif
/* vim: set ft=c : */
