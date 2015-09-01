#ifndef _GAME_H
#define _GAME_H

#include <stdlib.h>
#include "world.h"
#include "rules.h"


/*** TYPES ***/

typedef struct game game;
struct game {
    world *w;
    rule_calc_func_type rule;
};

/*** FUNCTIONS ***/

game* init_game(size_t xlim, size_t ylim, rule_calc_func_type rule);
void destroy_game(game *g);

#endif
/* vim: set ft=c : */
