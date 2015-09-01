#include <stdio.h>
#include "game.h"

game* init_game(size_t xlim, size_t ylim, rule_calc_func_type rule) {
    game *g = malloc(sizeof(game));
    g->rule = rule;
    g->w = init_world(xlim, ylim);
    return g;
}

void destroy_game(game *g) {
    destroy_world(g->w);
    free(g);
}

