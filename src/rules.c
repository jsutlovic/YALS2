#include "rules.h"

world_store conways_life(world_store state, world_store sum9) {
    switch(sum9) {
        case 3: return 1;
        case 4: return state;
        default: return 0;
    }
}

