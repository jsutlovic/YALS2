#include "fills.h"

static void _fill0(world *w, size_t x, size_t y, world_store *cell_val) {
    *cell_val = 0x3;
}

void fill0(world *w) {
    iter_world(w, _fill0);
}

static void _fill1(world *w, size_t x, size_t y, world_store *cell_val) {
    *cell_val = (x & 1) << 1;
}

void fill1(world *w) {
    iter_world(w, _fill1);
}

static void _fill2(world *w, size_t x, size_t y, world_store *cell_val) {
    *cell_val = ((x + y) & 1) << 1;
}

void fill2(world *w) {
    iter_world(w, _fill2);
}

static void _fill3(world *w, size_t x, size_t y, world_store *cell_val) {
    *cell_val = (x + y) & 3;
}

void fill3(world *w) {
    iter_world(w, _fill3);
}

static void _fill4(world *w, size_t x, size_t y, world_store *cell_val) {
    *cell_val = ((x+1) & 1) << 1;
}

void fill4(world *w) {
    iter_world(w, _fill4);
}

static void _fill5(world *w, size_t x, size_t y, world_store *cell_val) {
    *cell_val = ((x + y + 1) & 1) << 1;
}

void fill5(world *w) {
    iter_world(w, _fill5);
}
