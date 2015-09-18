#include "game.h"

#define VERTS_PER_TRIANGLE 6
#define MULTISAMPLE 0
#define ORTHO 0
#define PADDING 1
#define VSYNC 1

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

    SDL_GL_SetSwapInterval(VSYNC);

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
    g->sub_state = WHOLE;
    g->color_scheme = 0;

    g->w = init_world(xlim, ylim);
    return g;
}

static Uint32 _map_colorscheme(SDL_PixelFormat *format, const float *cols) {
    return SDL_MapRGBA(format, cols[0]*0xff, cols[1]*0xff, cols[2]*0xff, cols[3]*0xff);
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
    o->fontcol.r = 0xff;
    o->fontcol.g = 0xff;
    o->fontcol.b = 0xff;
    o->fontcol.a = 0xff;
    o->max_text = 8;

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

    float tex_scale = 100.0;

    o->color_id = glGetUniformLocation(g->overlay_shader, "color");
    o->matrix_id = glGetUniformLocation(g->overlay_shader, "MVP");
    o->tex_scale_id = glGetUniformLocation(g->overlay_shader, "scale");
    o->tex_coords_id = glGetAttribLocation(g->overlay_shader, "texCoord");

    glUseProgram(g->overlay_shader);
    glUniformMatrix4fv(o->matrix_id, 1, GL_FALSE, (GLfloat *) o->mvp);
    glUniform4fv(o->color_id, 1, &COLOR_SCHEMES[g->color_scheme][4]);
    glUniform1f(o->tex_scale_id, tex_scale);
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

    o->font_text = calloc(o->max_text+1, sizeof(char));

    o->bg = SDL_CreateRGBSurface(
        0, overlay_width, overlay_height, 32, rmask, gmask, bmask, amask
    );
    if (o->bg == NULL) {
        printf("BLARGH: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    o->bg_col = _map_colorscheme(o->bg->format, &COLOR_SCHEMES[g->color_scheme][4]);

    o->font_text[0] = 'W';
    SDL_Surface *temp_surf;
    temp_surf = TTF_RenderText_Solid(o->font, o->font_text, o->fontcol);
    int font_w = temp_surf->w;
    int font_h = temp_surf->h;
    SDL_free(temp_surf);

    o->font_surf = SDL_CreateRGBSurface(
        0, font_w * o->max_text + 1, font_h, 32, rmask, gmask, bmask, amask
    );

    SDL_FillRect(o->bg, NULL, o->bg_col);
    SDL_FillRect(o->font_surf, NULL, o->bg_col);

    // TODO: Render static text to overlay surface

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

void setup_game(game *g, int win_width, int win_height) {
    _init_gfx(g, win_width, win_height);
    _init_overlay(g);
}

static GLsizei _world_vertices(world *w, GLfloat aspect, GLfloat **v) {
    // Triangle has 6 points, each a float
    // Each cell has 2 triangles to make a square
    GLsizei vcount = 2 * VERTS_PER_TRIANGLE * w->xlim * w->ylim;
    *v = SDL_malloc(vcount * sizeof(GLfloat));

    GLfloat ratio, csize, psize;

    // Ratio of cell size to padding: 20% (divisor)
    ratio = 1.0 / 0.2;

    size_t count;
    float total_size, world_aspect;
    world_aspect = (float) w->xlim / (float) w->ylim;
    if (world_aspect >= aspect) {
        count = w->xlim;
        total_size = 2 * aspect;
    } else if (world_aspect <= 1.0) {
        count = w->ylim;
        total_size = 2.0;
    } else {
        count = w->xlim > w->ylim ? w->xlim : w->ylim;
        total_size = 2.0 * world_aspect;
    }

#if PADDING
    // Calculate cell size:
    // 2 is the size of the screen (-1 to 1)
    // csize is the solution to the equation: (count)x + (count + 1)(x * 0.2)
    csize = (total_size * ratio) / (ratio * count + (count + 1));
    // psize is 20% of csize
    psize = csize / ratio;
#else
    csize = total_size / count;
    psize = 0;
#endif

    size_t idx_base = 0;
    GLfloat top = 1.0 - psize,
            left = -(total_size/2.0) + psize;

    for (size_t y = 0; y < w->ylim; ++y) {
        GLfloat bottom = top - csize;
        for (size_t x = 0; x < w->xlim; ++x) {
            GLfloat right = left + csize;


            (*v)[idx_base+0] = (*v)[idx_base+6] = left; // left
            (*v)[idx_base+1] = (*v)[idx_base+7] = top; // top

            (*v)[idx_base+2] = left; // left
            (*v)[idx_base+3] = bottom; // bottom

            (*v)[idx_base+4] = (*v)[idx_base+8] = right; // right
            (*v)[idx_base+5] = (*v)[idx_base+9] = bottom; // bottom

            (*v)[idx_base+10] = right; // right
            (*v)[idx_base+11] = top; // top

            left += csize + psize;
            idx_base += 2 * VERTS_PER_TRIANGLE;
        }

        top -= csize + psize;
        left = -(total_size/2.0) + psize;
    }

    return vcount;
}

void start_game(game *g) {
    GLfloat *world_vertices;
    GLsizei world_vertices_count = _world_vertices(g->w, g->aspect, &world_vertices);

    mat4x4 MVP;

#if ORTHO
    float size = 1.0;
    mat4x4_ortho(MVP, -(g->aspect * size), g->aspect * size, -size, size, 0, 100);
#else
    mat4x4 Projection, View, Model, temp;

    vec3 eye    = {-1.8, -1.0, 0.4},
         center = {-0.6, 0.0, 0.0},
         up     = {0.0, 0.0, 1.0};

    mat4x4_perspective(Projection, 45.0f, g->aspect, 0.1f, 100.0f);
    mat4x4_look_at(View, eye, center, up);
    mat4x4_identity(Model);
    mat4x4_mul(temp, Projection, View);
    mat4x4_mul(MVP, temp, Model);
#endif

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
    glBufferData(GL_ARRAY_BUFFER, world_vertices_count*sizeof(GLfloat), world_vertices, GL_STATIC_DRAW);
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
    Uint32 cur_ticks = SDL_GetTicks();
    int fps_upd = 1;

    glClearColor(
            COLOR_SCHEMES[g->color_scheme][0],
            COLOR_SCHEMES[g->color_scheme][1],
            COLOR_SCHEMES[g->color_scheme][2],
            COLOR_SCHEMES[g->color_scheme][3]);

    SDL_Event e;
    int overlay_enabled = 1;
    size_t count = 0;
    SDL_Surface *temp_font_surf;

    while (g->state != ENDED) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(g->world_shader);

        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
        glUniform4fv(colors_id, 5, &COLOR_SCHEMES[g->color_scheme][8]);
        glUniform1ui(inv_state_id, !g->w->state);

        glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glActiveTexture(GL_TEXTURE0 + world_texture_id);
        glBindTexture(GL_TEXTURE_BUFFER, world_texture);
        glTexBuffer(GL_TEXTURE_BUFFER, GL_R32UI, world_data_buffer);
        glUniform1i(world_texture_buffer, world_texture_id);

        glDrawArrays(GL_TRIANGLES, 0, world_vertices_count / 2);

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
            snprintf(g->o->font_text, g->o->max_text + 1, "%8lu", g->w->generation);
            temp_font_surf = TTF_RenderText_Solid(g->o->font, g->o->font_text, g->o->fontcol);
            SDL_FillRect(g->o->font_surf, NULL, g->o->bg_col);
            SDL_BlitSurface(temp_font_surf, NULL, g->o->font_surf, NULL);

            glTexSubImage2D(GL_TEXTURE_2D, 0, 100, 100+g->o->font_surf->h, g->o->font_surf->w, g->o->font_surf->h, g->o->tex_format, g->o->tex_type, g->o->font_surf->pixels);
            free(temp_font_surf);

            // Render FPS and world generations
            if (fps_upd & 1) {
                /* printf("cur_ticks: %5u, %4u\n", cur_ticks, cur_ticks/100); */
                /* printf("count: %lu, ms: %u\n", count, (cur_ticks - start_loop)); */
                snprintf(g->o->font_text, g->o->max_text + 1, "%8.2f", count / ((cur_ticks - start_loop) / 1000.f));
                temp_font_surf = TTF_RenderText_Solid(g->o->font, g->o->font_text, g->o->fontcol);
                SDL_FillRect(g->o->font_surf, NULL, g->o->bg_col);
                SDL_BlitSurface(temp_font_surf, NULL, g->o->font_surf, NULL);

                glTexSubImage2D(GL_TEXTURE_2D, 0, 100, 100, g->o->font_surf->w, g->o->font_surf->h, g->o->tex_format, g->o->tex_type, g->o->font_surf->pixels);
                free(temp_font_surf);

                fps_upd = 0;
            }

            if ((cur_ticks / 100) % 5 == 0) {
                /* printf("OFF cur_ticks: %u\n", cur_ticks / 100); */
                fps_upd = 2;
            } else {
                fps_upd >>= 1;
            }
        }

        SDL_GL_SwapWindow(g->win);

        // Events
        if (SDL_PollEvent(&e)) {
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
                        fill(g->w, e.key.keysym.sym - SDLK_0);
                        g->state = PAUSED;
                        break;

                    // Quit
                    case(SDLK_ESCAPE):
                    case(SDLK_q): g->state = ENDED; break;

                    // Overlay
                    case(SDLK_TAB): overlay_enabled = 0; break;

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
                         g->sub_state = g->sub_state == WHOLE ? HALF : WHOLE;
                         break;
                    case(SDLK_c):
                         g->color_scheme++;
                         if (g->color_scheme >= COLOR_SCHEME_COUNT) {
                             g->color_scheme = 0;
                         }
                         glClearColor(
                                 COLOR_SCHEMES[g->color_scheme][0],
                                 COLOR_SCHEMES[g->color_scheme][1],
                                 COLOR_SCHEMES[g->color_scheme][2],
                                 COLOR_SCHEMES[g->color_scheme][3]);
                         g->o->bg_col = _map_colorscheme(g->o->bg->format, &COLOR_SCHEMES[g->color_scheme][4]);
                         SDL_FillRect(g->o->bg, NULL, g->o->bg_col);
                         glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, g->o->bg->w, g->o->bg->h, g->o->tex_format, g->o->tex_type, g->o->bg->pixels);
                         fps_upd = 1;
                         break;
                }
            } else if (e.type == SDL_KEYDOWN) {
                switch(e.key.keysym.sym) {
                    // Start running
                    case(SDLK_n): g->state = RUNNING; break;
                    case(SDLK_s): if (g->state == PAUSED) { world_half_step(g->w); }; break;
                    // Overlay
                    case(SDLK_TAB): overlay_enabled = 1; break;
                }
            }
        }

        // Update the world
        ++count;
        if (g->state == RUNNING) {
            switch (g->sub_state) {
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
    free_data_path();
    destroy_world(g->w);
    free(g);
}

