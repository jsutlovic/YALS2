#include "game.h"

#define VERTS_PER_TRIANGLE 6
#define MULTISAMPLE 0
#define PADDING 1

#define GET_COL(idx) &COLOR_SCHEMES[g->color_scheme][idx]

static void _sdl_init(game *g, int win_width, int win_height) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        puts("Failed to initialize SDL");
        exit(EXIT_FAILURE);
    }

    if (TTF_Init() < 0) {
        puts("Failed to initialize SDL_ttf");
        exit(EXIT_FAILURE);
    }

    g->win = SDL_CreateWindow(PROGRAM_NAME,
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        win_width,
        win_height,
        SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
    );

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    g->gl_ctx = SDL_GL_CreateContext(g->win);
    if (g->gl_ctx == NULL) {
        puts("Could not create an OpenGL context");
        exit(EXIT_FAILURE);
    }

    const unsigned char *gl_version = glGetString(GL_VERSION);
    printf("OpenGL version: %s\n", gl_version);

    if (gl_version == NULL) {
        puts("Could not determine OpenGL version");
        exit(EXIT_FAILURE);
    }

    SDL_GL_MakeCurrent(g->win, g->gl_ctx);

    SDL_GL_SetSwapInterval(g->vsync);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

#if MULTISAMPLE
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

    glDisable(GL_MULTISAMPLE);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
#endif

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        puts("Error initializing GLEW");
        exit(EXIT_FAILURE);
    }

    g->win_w = win_width;
    g->win_h = win_height;
    g->aspect = (float) win_width / (float) win_height;
}

static GLuint _create_shader(GLenum shader_type, const char *shader_file) {
    GLuint shader_id = glCreateShader(shader_type);
    char *shader_source = read_file(shader_file);

    glShaderSource(shader_id, 1, (const GLchar**) &shader_source, NULL);

    glCompileShader(shader_id);

    GLint status;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE) {
        printf("FAILED TO COMPILE SHADER: %s\n", shader_file);
        puts(shader_source);

        GLint infoLogLength;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[4096];
        glGetShaderInfoLog(shader_id, infoLogLength, NULL, strInfoLog);

        printf("\nGLSL error: %s", strInfoLog);
        return -1;
    } else {
        printf("Shader compiled: %s\n", shader_file);
    }

    free(shader_source);

    return shader_id;
}

static GLuint _shader_init(const char *vs_path, const char *fs_path) {
    GLuint vertex_shader, fragment_shader;

    vertex_shader = _create_shader(GL_VERTEX_SHADER, vs_path);
    fragment_shader = _create_shader(GL_FRAGMENT_SHADER, fs_path);

    GLuint shader_program = glCreateProgram();

    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);

    glLinkProgram(shader_program);

    GLint status;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &status);

    if (status == GL_FALSE) {
        puts("Shader linker failure!");

        GLint infoLogLength;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[infoLogLength];
        glGetProgramInfoLog(shader_program, infoLogLength, NULL, strInfoLog);

        printf("\nGLSL error: %s", strInfoLog);
        return -1;
    } else {
        printf("Shader program linked: %d\n", shader_program);
    }

    glDetachShader(shader_program, vertex_shader);
    glDetachShader(shader_program, fragment_shader);

    return shader_program;
}

static void _destroy_gfx(game *g) {
    SDL_GL_DeleteContext(g->gl_ctx);
    SDL_DestroyWindow(g->win);
    TTF_Quit();
    SDL_Quit();
}

static void _init_gfx(game *g, int win_width, int win_height) {
    _sdl_init(g, win_width, win_height);

    char *res_path = get_res_path(""),
         *w_vs_path = join_path(res_path, "world_vert.glsl"),
         *w_fs_path = join_path(res_path, "world_frag.glsl"),
         *o_vs_path = join_path(res_path, "overlay_vert.glsl"),
         *o_fs_path = join_path(res_path, "overlay_frag.glsl");

    g->world_shader = _shader_init(w_vs_path, w_fs_path);
    g->overlay_shader = _shader_init(o_vs_path, o_fs_path);

    free(w_vs_path);
    free(w_fs_path);
    free(o_vs_path);
    free(o_fs_path);
    free(res_path);

    if (g->world_shader == -1) {
        puts("Error creating world shader!");
        _destroy_gfx(g);
        exit(EXIT_FAILURE);
    }

    if (g->overlay_shader == -1) {
        puts("Error creating overlay shader!");
        _destroy_gfx(g);
        exit(EXIT_FAILURE);
    }
}

game* init_game(size_t xlim, size_t ylim) {
    game *g = malloc(sizeof(game));

    // Defaults
    g->state = PAUSED;
    g->step = WHOLE;
    g->color_scheme = 0;
    g->vsync = 1;
    g->d.ortho = 1;
    g->d.padding = 1;
    g->d.wp = (Plane) {{0, 0, -1}, {0, 0, 1}};
    g->d.zoom = 1;
    g->d.zoom_level = 0;
    g->d.zoom_amount = 1.1;

    g->d.trans_amount = 0.9;

    g->w = init_world(xlim, ylim);
    return g;
}

static inline void _print_vec3(vec3 vec, const char *desc) {
    printf("%sx: %4.3f, y: %4.3f, z: %4.3f\n", desc == NULL ? "" : desc, vec[0], vec[1], vec[2]);
}

static Uint32 _map_surface_colors(SDL_PixelFormat *format, const float *cols) {
    return SDL_MapRGBA(format, cols[0]*0xff, cols[1]*0xff, cols[2]*0xff, cols[3]*0xff);
}

static SDL_Color _map_sdl_colors(const float *cols) {
    SDL_Color col = {cols[0] * 0xff, cols[1] * 0xff, cols[2] * 0xff, cols[3] * 0xff};
    return col;
}

static void _overlay_draw_text(overlay *o, char *text, int centered, int line, surf_coord *text_coord) {
    SDL_Rect text_rect;
    SDL_Surface *temp_surf = TTF_RenderText_Solid(o->font, text, o->font_col);
    if (centered == 1) {
        text_rect.x = (o->bg->w / 2) - (temp_surf->w / 2);
    } else {
        text_rect.x = (o->bg->w / 2) - temp_surf->w;
    }
    text_rect.y = (o->text_pad * o->font_surf->h) + (line * o->font_spacing * o->font_surf->h);
    text_rect.w = temp_surf->w;
    text_rect.h = temp_surf->h;

    if (text_coord != NULL) {
        text_coord->x = o->bg->w / 2;
        text_coord->y = text_rect.y;
    }

    SDL_BlitSurface(temp_surf, NULL, o->bg, &text_rect);
    free(temp_surf);
}

static void _overlay_static_text(game *g) {
    overlay *o = g->o;
    char *temp_text = calloc(o->label_text_max, sizeof(char));

    // Overlay background has a border
    SDL_FillRect(o->bg, NULL, _map_surface_colors(o->bg->format, GET_COL(BORDER_OFFSET)));
    SDL_Rect bg_rect = {1, 1, o->bg->w-2, o->bg->h-2};
    SDL_FillRect(o->bg, &bg_rect, o->bg_col);

    SDL_FillRect(o->font_surf, NULL, o->bg_col);

    // Draw world map size
    snprintf(temp_text, o->label_text_max, "World %lux%lu", g->w->xlim, g->w->ylim);
    _overlay_draw_text(o, temp_text, 1, 0, NULL);

    // Draw world data size
    snprintf(temp_text, o->label_text_max, "World data size: %lu", g->w->data_size * sizeof(world_store));
    _overlay_draw_text(o, temp_text, 1, 1, NULL);

    // Draw FPS label
    snprintf(temp_text, o->label_text_max, "FPS: ");
    _overlay_draw_text(o, temp_text, 0, 5, &o->fps_loc);

    // Draw generation label
    snprintf(temp_text, o->label_text_max, "Generation: ");
    _overlay_draw_text(o, temp_text, 0, 6, &o->gen_loc);

    // Draw state label
    snprintf(temp_text, o->label_text_max, "State: ");
    _overlay_draw_text(o, temp_text, 0, 7, &o->state_loc);

    // Draw sub state label
    snprintf(temp_text, o->label_text_max, "Step: ");
    _overlay_draw_text(o, temp_text, 0, 8, &o->step_loc);
}

static void _update_colors(game *g, int color_scheme) {
    if (color_scheme >= COLOR_SCHEME_COUNT) {
        g->color_scheme = 0;
    } else if (color_scheme < 0) {
        g->color_scheme = COLOR_SCHEME_COUNT-1;
    } else {
        g->color_scheme = color_scheme;
    }

    glClearColor(
            COLOR_SCHEMES[g->color_scheme][BG_OFFSET],
            COLOR_SCHEMES[g->color_scheme][BG_OFFSET + 1],
            COLOR_SCHEMES[g->color_scheme][BG_OFFSET + 2],
            COLOR_SCHEMES[g->color_scheme][BG_OFFSET + 3]);

    g->o->bg_col = _map_surface_colors(g->o->bg->format, GET_COL(OVERLAY_OFFSET));
    g->o->font_col = _map_sdl_colors(GET_COL(FONT_OFFSET));

    _overlay_static_text(g);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g->o->bg->w, g->o->bg->h, g->o->tex_format, g->o->tex_type, g->o->bg->pixels);
}

static void _render_overlay_live_text(overlay *o, surf_coord *text_coord) {
    SDL_Surface *temp_font_surf = TTF_RenderText_Solid(o->font, o->font_text, o->font_col);
    SDL_FillRect(o->font_surf, NULL, o->bg_col);
    SDL_BlitSurface(temp_font_surf, NULL, o->font_surf, NULL);

    glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            text_coord->x,
            text_coord->y,
            o->font_surf->w,
            o->font_surf->h,
            o->tex_format,
            o->tex_type,
            o->font_surf->pixels
            );
    free(temp_font_surf);
}

static void _init_overlay(game *g) {
    overlay *o = malloc(sizeof(overlay));
    g->o = o;

    // Texture values
    o->tex_format = GL_RGBA;
    o->int_format = GL_RGBA8;
    o->tex_type = GL_UNSIGNED_INT_8_8_8_8;
    o->alignment = 4;

    // Font values
    o->update_text_max = 8;
    o->label_text_max = 32;
    o->text_pad = 0.5;
    o->font_spacing = 1.2;

    // Surface values
    Uint32 rmask = 0xff000000;
    Uint32 gmask = 0x00ff0000;
    Uint32 bmask = 0x0000ff00;
    Uint32 amask = 0x000000ff;

    // Overlay stuff
    o->size = 0.4;
    o->mvp = malloc(sizeof(mat4x4));
    mat4x4_ortho(*o->mvp, -g->aspect, g->aspect, -1.0, 1.0, 0, 10);

    int win_width, win_height, overlay_width, overlay_height;
    SDL_GetWindowSize(g->win, &win_width, &win_height);

    overlay_width = win_width * o->size;
    overlay_height = win_height * o->size;

    float overlay_triangles[8] = {
        -g->aspect*o->size,  o->size, // Top left
        -g->aspect*o->size, -o->size, // Bottom left
         g->aspect*o->size, -o->size, // Bottom right
         g->aspect*o->size,  o->size, // Top right
    };

    GLubyte overlay_elements[6] = {
        0, 1, 2,
        0, 2, 3
    };

    float overlay_tex_coords[8] = {
        0.0, 0.0, // Top left
        0.0, 1.0, // Bottom left
        1.0, 1.0, // Bottom right
        1.0, 0.0, // Top right
    };

    o->matrix_id = glGetUniformLocation(g->overlay_shader, "MVP");
    o->tex_coords_id = glGetAttribLocation(g->overlay_shader, "tex_coord");

    glUseProgram(g->overlay_shader);
    glUniformMatrix4fv(o->matrix_id, 1, GL_FALSE, (GLfloat *) o->mvp);
    glUseProgram(0);

    glGenBuffers(1, &o->vert_buf);

    glBindBuffer(GL_ARRAY_BUFFER, o->vert_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(overlay_triangles), &overlay_triangles, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &o->tex_coord_buf);

    glBindBuffer(GL_ARRAY_BUFFER, o->tex_coord_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(overlay_tex_coords), &overlay_tex_coords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &o->el_buf);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, o->el_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(overlay_elements), &overlay_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // Set up font things
    char *res_path = get_res_path(""),
         *font_path = join_path(res_path, "DejaVuSansMono.ttf");

    o->font = TTF_OpenFont(font_path, 14);

    free(res_path);
    free(font_path);

    o->font_text = calloc(o->update_text_max+1, sizeof(char));

    o->bg = SDL_CreateRGBSurface(
        0, overlay_width, overlay_height, 32, rmask, gmask, bmask, amask
    );
    if (o->bg == NULL) {
        printf("BLARGH: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    o->font_text[0] = 'W';
    SDL_Surface *temp_surf;
    temp_surf = TTF_RenderText_Solid(o->font, o->font_text, o->font_col);
    int font_w = temp_surf->w;
    int font_h = temp_surf->h;
    SDL_free(temp_surf);

    o->font_surf = SDL_CreateRGBSurface(
        0, font_w * o->update_text_max + 1, font_h, 32, rmask, gmask, bmask, amask
    );

    // Render static text
    _update_colors(g, g->color_scheme);

    glGenTextures(1, &o->tex);
    glBindTexture(GL_TEXTURE_2D, o->tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, o->alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, o->int_format, o->bg->w, o->bg->h, 0, o->tex_format, o->tex_type, o->bg->pixels);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
}

static void _destroy_overlay(game *g) {
    free(g->o->mvp);
    free(g->o->bg);
    free(g->o->font_surf);
    free(g->o->font);
    free(g->o->font_text);
}

static void _init_world_display(game *g) {
    // Triangle has 6 points, each a float
    // Each cell has 2 triangles to make a square
    g->d.vcount = 2 * VERTS_PER_TRIANGLE * g->w->xlim * g->w->ylim;
    g->d.vertices = SDL_malloc(g->d.vcount * sizeof(GLfloat));
}

static void _destroy_world_display(game *g) {
    free(g->d.vertices);
}

void setup_game(game *g, int win_width, int win_height) {
    _init_gfx(g, win_width, win_height);
    _init_overlay(g);
    _init_world_display(g);
}

static void _norm_mouse_coords(vec3 coords, int win_x, int win_y, int win_w, int win_h) {
    coords[0] = ( 2. * win_x) / (float) win_w - 1.;
    coords[1] = (-2. * win_y) / (float) win_h + 1.;
    coords[2] = 0.0f;
}

static void _norm_point_to_ray(game *g, Ray *r, float x, float y) {
    vec4 near_point_ndc = {x, y, 1, 1};
    vec4 far_point_ndc = {x, y, -1, 1};

    mat4x4 inv_mvp;
    mat4x4_invert(inv_mvp, g->d.mvp);

    vec4 near_point_world, far_point_world;
    mat4x4_mul_vec4(near_point_world, inv_mvp, near_point_ndc);
    mat4x4_mul_vec4(far_point_world, inv_mvp, far_point_ndc);

    divide_by_w(near_point_world);
    divide_by_w(far_point_world);

    vec3 between;
    vec3_sub(between, far_point_world, near_point_world);
    Ray ray = {
        {near_point_world[0], near_point_world[1], near_point_world[2]},
        {between[0], between[1], between[2]}
    };
    memcpy(r, &ray, sizeof(ray));
}

static inline void _world_coords_to_cell_pos(game *g, world_cell_pos *pos, vec3 coords) {
    float full_size = g->d.cell_size + g->d.pad_size;
    float half_pad = g->d.pad_size / 2;

    pos->x = ( coords[0] - half_pad - g->d.left) / full_size;
    pos->y = (-coords[1] - half_pad + g->d.top) / full_size;
}

static inline void _handle_mouse_click(game *g, int win_x, int win_y) {
    vec3 mwc, mnc;
    Ray mouse_ray;
    world_cell_pos pos;
    pos.w = g->w;
    pos.cell_val = NULL;

    if (g->w->state == SHIFT) {
        world_half_step(g->w);
    }

    _norm_mouse_coords(mnc, win_x, win_y, g->win_w, g->win_h);
    _norm_point_to_ray(g, &mouse_ray, mnc[0], mnc[1]);
    ray_intersection_point(&mwc, mouse_ray, g->d.wp);
    _world_coords_to_cell_pos(g, &pos, mwc);
    if (pos.x >= g->w->xlim || pos.y >= g->w->ylim) {
        return;
    }
    invert_cell(&pos);
}

static void _world_vertices(game *g) {
    world *w = g->w;

    GLfloat ratio;

    // Ratio of cell size to padding: 20% (divisor)
    ratio = 1.0 / 0.2;

    size_t count;
    float total_size, world_aspect;
    world_aspect = (float) w->xlim / (float) w->ylim;
    if (world_aspect >= g->aspect) {
        count = w->xlim;
        total_size = 2 * g->aspect;
    } else if (world_aspect <= 1.0) {
        count = w->ylim;
        total_size = 2.0;
    } else {
        count = w->xlim > w->ylim ? w->xlim : w->ylim;
        total_size = 2.0 * world_aspect;
    }

    if (g->d.padding) {
        // Calculate cell size:
        // 2 is the size of the screen (-1 to 1)
        // cell_size is the solution to the formula: (count)x + (count + 1)(x * 0.2)
        g->d.cell_size = (total_size * ratio) / (ratio * count + (count + 1));
        // pad_size is 20% of cell_size
        g->d.pad_size = g->d.cell_size / ratio;
    } else {
        g->d.cell_size = total_size / count;
        g->d.pad_size = 0;
    }

    g->d.top = 1.0;
    g->d.left = -(total_size/2);
    g->d.bottom = w->ylim * g->d.cell_size + (w->ylim + 1) * g->d.pad_size;
    g->d.right = w->xlim * g->d.cell_size + (w->xlim + 1) * g->d.pad_size;

    size_t idx_base = 0;
    GLfloat top = g->d.top - g->d.pad_size,
            left = g->d.left + g->d.pad_size;

    for (size_t y = 0; y < w->ylim; ++y) {
        GLfloat bottom = top - g->d.cell_size;
        for (size_t x = 0; x < w->xlim; ++x) {
            GLfloat right = left + g->d.cell_size;


            g->d.vertices[idx_base+0] = g->d.vertices[idx_base+6] = left; // left
            g->d.vertices[idx_base+1] = g->d.vertices[idx_base+7] = top; // top

            g->d.vertices[idx_base+2] = left; // left
            g->d.vertices[idx_base+3] = bottom; // bottom

            g->d.vertices[idx_base+4] = g->d.vertices[idx_base+8] = right; // right
            g->d.vertices[idx_base+5] = g->d.vertices[idx_base+9] = bottom; // bottom

            g->d.vertices[idx_base+10] = right; // right
            g->d.vertices[idx_base+11] = top; // top

            left += g->d.cell_size + g->d.pad_size;
            idx_base += 2 * VERTS_PER_TRIANGLE;
        }

        top -= g->d.cell_size + g->d.pad_size;
        left = -(total_size/2.0) + g->d.pad_size;
    }
}

static inline void _update_translations(game *g) {
    g->d.trans = g->d.trans_amount * pow(g->d.trans_amount, g->d.zoom_level + 3);
}

static inline void _calc_zoom(game *g, int dir) {
    g->d.zoom_level += dir;
    g->d.zoom = pow(g->d.zoom_amount, g->d.zoom_level - (g->d.ortho ? 0 : 6));
    _update_translations(g);

    float zoom = 1 / g->d.zoom;
    if (g->d.ortho) {
        mat4x4_ortho(g->d.proj, -g->aspect * zoom, g->aspect * zoom, -1 * zoom, 1 * zoom, 1, 1000);
    } else {
        vec3 temp;
        memcpy(&g->d.eye_zoom, &g->d.eye, sizeof(g->d.eye_zoom));
        vec3_scale(temp, g->d.view_f, zoom);
        vec3_sub(g->d.eye_zoom, g->d.center, temp);
    }
}

static inline void _zoom_view(game *g) {
    if (!g->d.ortho) {
        mat4x4_look_at(g->d.view, g->d.eye_zoom, g->d.center, g->d.up);
    }
}

static inline void _reset_camera(game *g) {
    vec3_set(g->d.eye, 0.0, 0.0, 1.0);
    vec3_set(g->d.center, 0.0, 0.0, -1.0);
    vec3_set(g->d.up, 0.0, 1.0, 0.0);
    g->d.zoom_level = 0;
    _calc_zoom(g, 0);
}

static inline void _setup_camera(game *g) {
    if (g->d.ortho) {
        mat4x4_ortho(g->d.proj, -g->aspect, g->aspect, -1, 1, 0.001, 1000);
    } else {
        mat4x4_perspective(g->d.proj, 45.0, g->aspect, 0.001, 1000.0);
    }

    vec3_sub(g->d.view_f, g->d.center, g->d.eye);
    vec3_norm(g->d.view_f, g->d.view_f);

    vec3_mul_cross(g->d.view_r, g->d.view_f, g->d.up);
    vec3_norm(g->d.view_r, g->d.view_r);

    vec3_mul_cross(g->d.view_u, g->d.view_r, g->d.view_f);

    _calc_zoom(g, 0); // Update zoom for this view type
}

static inline void _set_view(game *g) {
    mat4x4_look_at(g->d.view, g->d.eye, g->d.center, g->d.up);
}

static inline void _update_camera(game *g) {
    _set_view(g);
    _zoom_view(g);
    mat4x4_mul(g->d.mvp, g->d.proj, g->d.view);
}

static inline void _move_camera(game *g, direction d) {
    vec3 temp;
    switch (d) {
        case (UP):
            vec3_scale(temp, g->d.view_u, g->d.trans);
            break;
        case(DOWN):
            vec3_scale(temp, g->d.view_u, -g->d.trans);
            break;
        case(LEFT):
            vec3_scale(temp, g->d.view_r, -g->d.trans);
            break;
        case(RIGHT):
            vec3_scale(temp, g->d.view_r, g->d.trans);
            break;
    }
    vec3_add(g->d.eye, g->d.eye, temp);
    vec3_add(g->d.eye_zoom, g->d.eye_zoom, temp);
    vec3_add(g->d.center, g->d.center, temp);
}

void start_game(game *g) {
    _world_vertices(g);

    _reset_camera(g);
    _setup_camera(g);
    _update_camera(g);

    int world_texture_id = 0;

    GLuint matrix_id = glGetUniformLocation(g->world_shader, "MVP");
    GLuint colors_id = glGetUniformLocation(g->world_shader, "colors");
    GLuint inv_state_id = glGetUniformLocation(g->world_shader, "inv_state");
    GLuint world_texture_buffer = glGetUniformLocation(g->world_shader, "world_texture_buffer");

    // Vertex arrays
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // World vertices
    GLuint triangle_buffer;
    glGenBuffers(1, &triangle_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
    glBufferData(GL_ARRAY_BUFFER, g->d.vcount*sizeof(GLfloat), g->d.vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // World data buffer (texture buffer object)
    GLuint world_data_buffer;
    glGenBuffers(1, &world_data_buffer);
    glBindBuffer(GL_TEXTURE_BUFFER, world_data_buffer);
    glBufferData(GL_TEXTURE_BUFFER, g->w->data_size*sizeof(world_store), g->w->data, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_TEXTURE_BUFFER, 0);

    // World texture
    GLuint world_texture;
    glGenTextures(1, &world_texture);

    // Start game
    Uint32 start_loop = SDL_GetTicks();
    SDL_Delay(1);
    Uint32 cur_ticks = SDL_GetTicks();
    int fps_upd = 1;

    SDL_Event e;
    int overlay_enabled = 0;
    size_t count = 0;

    while (g->state != ENDED) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(g->world_shader);

        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &g->d.mvp[0][0]);
        glUniform4fv(colors_id, 5, GET_COL(COLORS_OFFSET));
        glUniform1ui(inv_state_id, !g->w->state);

        glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glActiveTexture(GL_TEXTURE0 + world_texture_id);
        glBindTexture(GL_TEXTURE_BUFFER, world_texture);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, world_data_buffer);
        glUniform1i(world_texture_buffer, world_texture_id);

        glDrawArrays(GL_TRIANGLES, 0, g->d.vcount / 2);

        glDisableVertexAttribArray(0);
        glBindTexture(GL_TEXTURE_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glUseProgram(0);

        cur_ticks = SDL_GetTicks();

        if (overlay_enabled) {
            // Draw overlay
            glUseProgram(g->overlay_shader);

            glBindBuffer(GL_ARRAY_BUFFER, g->o->vert_buf);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g->o->el_buf);
            glEnableVertexAttribArray(0); // This should be a var
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0); // This should be a var

            glBindBuffer(GL_ARRAY_BUFFER, g->o->tex_coord_buf);
            glEnableVertexAttribArray(g->o->tex_coords_id);
            glVertexAttribPointer(g->o->tex_coords_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

            glBindTexture(GL_TEXTURE_2D, g->o->tex);
            glActiveTexture(GL_TEXTURE0);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

            glDisableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glUseProgram(0);

            // World generations
            snprintf(g->o->font_text, g->o->update_text_max + 1, "%8lu", g->w->generation);
            _render_overlay_live_text(g->o, &g->o->gen_loc);

            // Game state
            snprintf(g->o->font_text, g->o->update_text_max + 1, "%8s",
                    GET_STATE_TEXT(g->state));
            _render_overlay_live_text(g->o, &g->o->state_loc);

            // Game step
            snprintf(g->o->font_text, g->o->update_text_max + 1, "%8s",
                    GET_STEP_TEXT(g->step));
            _render_overlay_live_text(g->o, &g->o->step_loc);

            // Render FPS and world generations
            if (fps_upd & 1) {
                float fps = count / ((cur_ticks - start_loop) / 1000.f);
                snprintf(g->o->font_text, g->o->update_text_max + 1, "%8.2f", fps);
                _render_overlay_live_text(g->o, &g->o->fps_loc);

                fps_upd = 0;
            }

            if ((cur_ticks / 100) % 5 == 0) {
                fps_upd = 2;
            } else {
                fps_upd >>= 1;
            }
        }

        SDL_GL_SwapWindow(g->win);

        // Events
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                g->state = ENDED;
            }
            if (e.type == SDL_KEYUP) {
                switch(e.key.keysym.sym) {
                    // Fills
                    case(SDLK_0):
                    case(SDLK_1):
                    case(SDLK_2):
                    case(SDLK_3):
                    case(SDLK_4):
                    case(SDLK_5):
                    case(SDLK_6):
                    case(SDLK_7):
                    case(SDLK_8):
                    case(SDLK_9):
                        fill(g->w, e.key.keysym.sym - SDLK_0);
                        g->state = PAUSED;
                        break;

                    // Quit
                    case(SDLK_ESCAPE):
                    case(SDLK_q): g->state = ENDED; break;

                    // Overlay
                    case(SDLK_TAB): overlay_enabled = 0; break;

                    // Toggle vsync
                    case(SDLK_v):
                        g->vsync = !g->vsync;
                        SDL_GL_SetSwapInterval(g->vsync);
                        break;

                    // End running
                    case(SDLK_n): g->state = PAUSED; break;
                    // Toggle running
                    case(SDLK_SPACE):
                        g->state = g->state == RUNNING ? PAUSED : RUNNING;
                        break;
                    // Toggle sub-state
                    case(SDLK_h):
                        if (g->w->state != CALC) {
                            world_half_step(g->w);
                        }
                        g->step = g->step == WHOLE ? HALF : WHOLE;
                        break;
                    // Change color scheme
                    case(SDLK_c):
                        if (e.key.keysym.mod & KMOD_SHIFT) {
                            _update_colors(g, g->color_scheme - 1);
                        } else {
                            _update_colors(g, g->color_scheme + 1);
                        }
                        fps_upd = 1;
                        break;
                    // Switch projection type
                    case(SDLK_o):
                        g->d.ortho = !g->d.ortho;
                        _setup_camera(g);
                        _set_view(g);
                        break;
                    // Reset camera
                    case(SDLK_r):
                        _reset_camera(g);
                        break;
                }
            } else if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    // Start running
                    case(SDLK_n): g->state = RUNNING; break;
                    // Single step
                    case(SDLK_m):
                        g->state = PAUSED;
                        world_half_step(g->w);
                        break;
                    // Translate up
                    case(SDLK_w):
                    case(SDLK_UP):
                        _move_camera(g, UP);
                        break;
                    // Translate left
                    case(SDLK_a):
                    case(SDLK_LEFT):
                        _move_camera(g, LEFT);
                        break;
                    // Translate up
                    case(SDLK_s):
                    case(SDLK_DOWN):
                        _move_camera(g, DOWN);
                        break;
                    // Translate right
                    case(SDLK_d):
                    case(SDLK_RIGHT):
                        _move_camera(g, RIGHT);
                        break;

                    // Overlay
                    case(SDLK_TAB): overlay_enabled = 1; break;
                }
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                switch (e.button.button) {
                    // Invert cell under cursor
                    case(SDL_BUTTON_LEFT):
                        g->state = PAUSED;
                        _handle_mouse_click(g, e.button.x, e.button.y);
                        break;
                }
            } else if (e.type == SDL_MOUSEWHEEL) {
                _calc_zoom(g, e.wheel.y > 0 ? 1 : -1);
            }
        }

        // Update camera
        _update_camera(g);

        // Update the world
        ++count;
        if (g->state == RUNNING) {
            switch (g->step) {
                case(WHOLE): world_step(g->w); break;
                case(HALF): world_half_step(g->w); break;
            }
        }

        glBindBuffer(GL_TEXTURE_BUFFER, world_data_buffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, g->w->data_size*sizeof(world_store), g->w->data);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
    }
}

void destroy_game(game *g) {
    _destroy_gfx(g);
    _destroy_overlay(g);
    _destroy_world_display(g);
    free_data_path();
    destroy_world(g->w);
    free(g);
}

