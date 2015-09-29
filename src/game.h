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
#include "geom.h"
#include "fills.h"
#include "colors.h"

#define GET_STATE_TEXT(state) state == RUNNING ? "Running" : "Paused"
#define GET_STEP_TEXT(step) step == WHOLE ? "Whole" : "Half"

/*** TYPES ***/

enum direction {
    UP=0,
    LEFT,
    DOWN,
    RIGHT,
};
typedef enum direction direction;

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
    int label_text_max;
    int update_text_max;
    float text_pad;
    float font_spacing;
    char *font_text;

    // Uniform IDs
    GLuint matrix_id;
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

struct world_display {
    int padding;

    GLfloat top;
    GLfloat left;
    GLfloat bottom;
    GLfloat right;

    GLfloat cell_size;
    GLfloat pad_size;

    GLsizei vcount;
    GLfloat *vertices;

    mat4x4 view;
    mat4x4 proj;
    mat4x4 mvp;

    Plane wp;

    vec3 eye;
    vec3 center;
    vec3 up;

    vec3 eye_zoom;

    vec3 view_f;
    vec3 view_r;
    vec3 view_u;

    int zoom_level;

    float trans;
    float trans_amount;

    float zoom;
    float zoom_amount;

    int ortho;
};
typedef struct world_display world_display;

struct game {
    world *w;
    overlay *o;
    world_display d;
    SDL_Window *win;
    SDL_GLContext gl_ctx;
    GLint world_shader;
    GLint overlay_shader;
    game_state state;
    game_step step;
    int color_scheme;
    float aspect;
    int win_w;
    int win_h;
    int vsync;
};
typedef struct game game;

/*** FUNCTIONS ***/

game* init_game(size_t xlim, size_t ylim);
void setup_game(game *g, int width, int height);
void start_game(game *g);
void destroy_game(game *g);

#endif
/* vim: set ft=c : */
