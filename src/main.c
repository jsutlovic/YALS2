#include <stdio.h>
#include "world.h"
#include "rules.h"
#include "fills.h"

#define PROFILE 0

int main() {
#if !PROFILE
    world *w = init_world(10, 10);
#else
    world *w = init_world(200, 200);
#endif
    printf("World size: %lu\n", w->data_size);
    fill1(w);
#if !PROFILE
    print_world(w);
    for (int i = 0; i < 5; i++) {
        world_half_step(w);
        print_world(w);
        world_half_step(w);
        print_world(w);
    }
#else
    puts("Start!");
    for (int i = 0; i < 1000; i++) {
        world_step(w);
    }
    puts("End!");
#endif
    destroy_world(w);
    return EXIT_SUCCESS;
}
