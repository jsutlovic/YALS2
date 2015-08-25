#include <limits.h>
#include "world.h"

world* init_world(size_t length, size_t width) {
    world *w = malloc(sizeof(world));
    w->world_length = length;
    w->world_width = width;
    w->generation = 0;

    // TODO: Check if length and width are >= sqrt(SIZE_MAX/2)

    w->data_size = ( length * width * 2.0 ) / ( sizeof(*w->data) * CHAR_BIT ) + .969;

    w->data = calloc(w->data_size, sizeof(*w->data));
    return w;
}

void destroy_world(world *w) {
    free(w->data);
    free(w);
}
