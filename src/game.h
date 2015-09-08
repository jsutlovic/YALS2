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
#include "fills.h"
#include "colors.h"

/*** TYPES ***/

enum game_state {
    PAUSED=0,
    RUNNING=1,
    ENDED=2,
};
typedef enum game_state game_state;

enum game_sub_state {
    FULL=0,
    HALF=1,
};
typedef enum game_sub_state game_sub_state;

typedef struct game game;
struct game {
    world *w;
    SDL_Window *win;
    SDL_GLContext gl_ctx;
    GLint gl_shader;
    game_state state;
    game_sub_state sub_state;
    int color_scheme;
};

/*** FUNCTIONS ***/

game* init_game(size_t xlim, size_t ylim);
void setup_game(game *g, int width, int height);
void start_game(game *g);
void destroy_game(game *g);

#endif
/* vim: set ft=c : */
