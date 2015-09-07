#ifndef _GAME_H
#define _GAME_H

#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "fsutil.h"
#include "res_path.h"
#include "world.h"
#include "linmath.h"

/*** TYPES ***/

typedef struct game game;
struct game {
    world *w;
    SDL_Window *win;
    SDL_GLContext gl_ctx;
    GLint gl_shader;
};

/*** FUNCTIONS ***/

game* init_game(size_t xlim, size_t ylim);
void setup_game(game *g, int width, int height);
void start_game(game *g);
void destroy_game(game *g);

#endif
/* vim: set ft=c : */
