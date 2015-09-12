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

void setup_game(game *g, int win_width, int win_height) {
    _init_gfx(g, win_width, win_height);
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
    int ww, wh;
    SDL_GetWindowSize(g->win, &ww, &wh);

    float aspect = (float) ww / (float) wh;

    GLfloat *world_vertices;
    GLsizei world_vertices_count = _world_vertices(g->w, aspect, &world_vertices);

    mat4x4 MVP;

#if ORTHO
    float size = 1.0;
    mat4x4_ortho(MVP, -(aspect * size), aspect * size, -size, size, 0, 100);
#else
    mat4x4 Projection, View, Model, temp;

    vec3 eye    = {-1.8, -1.0, 0.4},
         center = {-0.6, 0.0, 0.0},
         up     = {0.0, 0.0, 1.0};

    mat4x4_perspective(Projection, 45.0f, aspect, 0.1f, 100.0f);
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


    // Overlay stuff
    GLfloat overlay_size = 0.6;
    mat4x4 overlay_mvp;
    mat4x4_ortho(overlay_mvp, -aspect, aspect, -1.0, 1.0, 0, 10);

    float overlay_triangles[] = {
        -aspect*overlay_size,  overlay_size, // Top left
        -aspect*overlay_size, -overlay_size, // Bottom left
         aspect*overlay_size, -overlay_size, // Bottom right
         aspect*overlay_size,  overlay_size, // Top right
    };

    GLubyte overlay_elements[] = {
        0, 1, 2,
        0, 2, 3
    };

    float overlay_tex_coords[] = {
        0.0, 0.0, // Top left
        0.0, 1.0, // Bottom left
        1.0, 1.0, // Bottom right
        1.0, 0.0, // Top right
    };

    GLuint overlay_color_id = glGetUniformLocation(g->overlay_shader, "color");
    GLuint overlay_matrix_id = glGetUniformLocation(g->overlay_shader, "MVP");
    GLint overlay_tex_coords_id = glGetAttribLocation(g->overlay_shader, "texCoord");

    GLuint overlay_vert_buf;
    glGenBuffers(1, &overlay_vert_buf);

    glBindBuffer(GL_ARRAY_BUFFER, overlay_vert_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(overlay_triangles), &overlay_triangles, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint overlay_tex_coord_buf;
    glGenBuffers(1, &overlay_tex_coord_buf);

    glBindBuffer(GL_ARRAY_BUFFER, overlay_tex_coord_buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(overlay_tex_coords), &overlay_tex_coords, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    GLuint overlay_el_buf;
    glGenBuffers(1, &overlay_el_buf);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, overlay_el_buf);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(overlay_elements), &overlay_elements, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    SDL_Surface *overlay_surf = SDL_CreateRGBSurface(
        0, 100, 100, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff
    );
    if (overlay_surf == NULL) {
        printf("BLARGH: %s\n", SDL_GetError());
        exit(EXIT_FAILURE);
    }
    SDL_FillRect(overlay_surf, NULL, SDL_MapRGBA(overlay_surf->format, 0x00, 0xff, 0x00, 0xff)); // Fill red
    printf("Bytes per pixel: %d\n", overlay_surf->format->BytesPerPixel);
    printf("Rmask format: %8x\n", overlay_surf->format->Rmask);
    printf("Pitch: %d\n", overlay_surf->pitch);
    int alignment = 8;
    printf("exp pitch: %d\n", (overlay_surf->w*overlay_surf->format->BytesPerPixel+alignment-1)/alignment*alignment);
    GLenum overlay_tex_format = GL_RGBA,
           overlay_int_format = GL_RGBA8,
           overlay_tex_type = GL_UNSIGNED_INT_8_8_8_8;


    GLuint overlay_tex;
    glGenTextures(1, &overlay_tex);
    glBindTexture(GL_TEXTURE_2D, overlay_tex);

    glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

    glTexImage2D(GL_TEXTURE_2D, 0, overlay_int_format, overlay_surf->w, overlay_surf->h, 0, overlay_tex_format, overlay_tex_type, overlay_surf->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // reset
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    // Start game
    Uint32 start_loop = SDL_GetTicks();

    SDL_Event e;
    int overlay_enabled = 0;
    size_t count = 0;
    while (g->state != ENDED) {
        glClearColor(
                COLOR_SCHEMES[g->color_scheme][0],
                COLOR_SCHEMES[g->color_scheme][1],
                COLOR_SCHEMES[g->color_scheme][2],
                COLOR_SCHEMES[g->color_scheme][3]);
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

        if (overlay_enabled) {
            // Draw overlay
            glUseProgram(g->overlay_shader);
            glUniformMatrix4fv(overlay_matrix_id, 1, GL_FALSE, &overlay_mvp[0][0]);
            glUniform4fv(overlay_color_id, 1, &COLOR_SCHEMES[g->color_scheme][4]);

            glBindBuffer(GL_ARRAY_BUFFER, overlay_vert_buf);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, overlay_el_buf);
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

            glBindBuffer(GL_ARRAY_BUFFER, overlay_tex_coord_buf);
            glEnableVertexAttribArray(overlay_tex_coords_id);
            glVertexAttribPointer(overlay_tex_coords_id, 2, GL_FLOAT, GL_FALSE, 0, 0);

            glBindTexture(GL_TEXTURE_2D, overlay_tex);
            glActiveTexture(GL_TEXTURE0);

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);

            glDisableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

            glUseProgram(0);
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
    Uint32 end_loop = SDL_GetTicks();
    float total = (end_loop - start_loop) / 1000.0;
    printf("Frames: %lu in %.4f seconds, %.4f f/s\n", count, total, count / total);
    printf("World generations: %lu in %.4f seconds, %.4f gen/s\n", g->w->generation, total, g->w->generation / total);
}

void destroy_game(game *g) {
    _destroy_gfx(g);
    free_data_path();
    destroy_world(g->w);
    free(g);
}

