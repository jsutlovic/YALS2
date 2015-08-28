#include <stdio.h>
#include "rules.h"

world_store debug_count_rule(world_store state, world_store sum9) {
    printf("%d ", sum9);
    return state & 1;
}

world_store conways_life(world_store state, world_store sum9) {
    switch(sum9) {
        case 3: return 1;
        case 4: return state & 1;
        default: return 0;
    }
}

