#include <stdio.h>
#include "game.h"

game* init_game(size_t length, size_t width, rule_calc_func_type rule) {
    game *g = malloc(sizeof(game));
    g->rule = rule;
    g->w = init_world(length, width);
    return g;
}

void destroy_game(game *g) {
    destroy_world(g->w);
    free(g);
}

void game_step(game *g) {
}

