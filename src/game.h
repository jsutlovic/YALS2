#ifndef _GAME_H
#define _GAME_H

#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <GL/glew.h>

#include "fsutil.h"
#include "res_path.h"
#include "world.h"
#include "linmath.h"
#include "fills.h"
#include "colors.h"

#define GET_STATE_TEXT(state) state == RUNNING ? "Running" : "Paused"
#define GET_STEP_TEXT(step) step == WHOLE ? "Whole" : "Half"

/*** TYPES ***/

enum game_state {
    PAUSED=0,
    RUNNING=1,
    ENDED=2,
};
typedef enum game_state game_state;

enum game_step {
    WHOLE=0,
    HALF=1,
};
typedef enum game_step game_step;

struct surf_coord {
    int x;
    int y;
};
typedef struct surf_coord surf_coord;

struct overlay {
    GLfloat size;
    mat4x4 *mvp;

    // Overlay surface
    SDL_Surface *bg;

    // Overlay texture properties
    Uint32 bg_col;
    GLenum tex_format;
    GLenum int_format;
    GLenum tex_type;
    int alignment;

    // Font things
    SDL_Surface *font_surf;
    TTF_Font *font;
    SDL_Color font_col;
    int max_text;
    char *font_text;

    // Uniform IDs
    GLuint color_id;
    GLuint matrix_id;
    GLuint tex_scale_id;
    GLuint tex_coords_id;

    // Buffer IDs
    GLuint vert_buf;
    GLuint tex_coord_buf;
    GLuint el_buf;

    // Texture IDs
    GLuint tex;

    // Drawing locations
    surf_coord fps_loc;
    surf_coord gen_loc;
    surf_coord state_loc;
    surf_coord step_loc;
};
typedef struct overlay overlay;

struct game {
    world *w;
    overlay *o;
    SDL_Window *win;
    SDL_GLContext gl_ctx;
    GLint world_shader;
    GLint overlay_shader;
    game_state state;
    game_step step;
    int color_scheme;
    float aspect;
};
typedef struct game game;

/*** FUNCTIONS ***/

game* init_game(size_t xlim, size_t ylim);
void setup_game(game *g, int width, int height);
void start_game(game *g);
void destroy_game(game *g);

#endif
/* vim: set ft=c : */
