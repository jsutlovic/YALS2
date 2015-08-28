#ifndef _RULES_H
#define _RULES_H

#include "world.h"

typedef world_store (*rule_calc_func_type) (world_store, world_store);

world_store debug_count_rule(world_store state, world_store sum9);
world_store conways_life(world_store state, world_store sum9);

#endif
/* vim: set ft=c : */
