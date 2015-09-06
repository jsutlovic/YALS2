#ifndef _FILLS_H
#define _FILLS_H

#include <stdlib.h>
#include "world.h"

void fill0(world *w); // Full fill (all 3s)
void fill1(world *w); // Every even cell (in-row) is alive
void fill2(world *w); // Every even cell (in-world) is alive
void fill3(world *w); // cell_val = 0, 1, 2, 3, 0, 1, ...
void fill4(world *w); // Every odd cell (in-row) is alive
void fill5(world *w); // Every odd cell (in-world) is alive

#endif
/* vim: set ft=c : */
