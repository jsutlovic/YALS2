#include <stdio.h>
#include "game.h"

void _fill1(world *w, size_t x, size_t y, size_t *cell_val) {
    *cell_val = (x & 1) << 1;
}

void fill1(world *w) {
    iter_world(w, _fill1);
}

void _fill2(world *w, size_t x, size_t y, size_t *cell_val) {
    *cell_val = ((x + y) & 1) << 1;
}

void fill2(world *w) {
    iter_world(w, _fill2);
}

void _fill3(world *w, size_t x, size_t y, size_t *cell_val) {
    *cell_val = (x + y) & 3;
}

void fill3(world *w) {
    iter_world(w, _fill3);
}
