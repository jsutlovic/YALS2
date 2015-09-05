#include "game.h"

#define VERTS_PER_TRIANGLE 6
#define MULTISAMPLE 1
#define ORTHO 0

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

    SDL_GL_SetSwapInterval(0);

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
        puts("FAILED TO COMPILE SHADER:");
        puts(shader_source);

        GLint infoLogLength;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[4096];
        glGetShaderInfoLog(shader_id, infoLogLength, NULL, strInfoLog);

        printf("\nGLSL error: %s", strInfoLog);
        return -1;
    } else {
        puts("Shader compiled!");
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
        return -1;
    } else {
        puts("Shaders linked");
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
         *vs_path = join_path(res_path, "vs1.glsl"),
         *fs_path = join_path(res_path, "fs1.glsl");

    g->gl_shader = _shader_init(vs_path, fs_path);

    free(vs_path);
    free(fs_path);
    free(res_path);

    if (g->gl_shader == -1) {
        puts("ACK");
        _destroy_gfx(g);
        exit(EXIT_FAILURE);
    }
}

game* init_game(size_t xlim, size_t ylim) {
    game *g = malloc(sizeof(game));
    g->w = init_world(xlim, ylim);
    return g;
}

void setup_game(game *g, int win_width, int win_height) {
    _init_gfx(g, win_width, win_height);
}

static GLsizei _world_vertices(world *w, GLfloat **v) {
    // Triangle has 6 points, each a float
    // Each cell has 2 triangles to make a square
    GLsizei vcount = 2 * VERTS_PER_TRIANGLE * w->xlim * w->ylim;
    *v = SDL_malloc(vcount * sizeof(GLfloat));

    GLfloat orig_x, orig_y, ratio, csize, psize;

    // Ratio of cell size to padding: 20% (divisor)
    ratio = 1.0 / 0.2;

    size_t count = w->xlim > w->ylim ? w->xlim : w->ylim;
    // Calculate cell size:
    // 2 is the size of the screen (-1 to 1)
    // csize is the solution to the equation: (count)x + (count + 1)(x * 0.2)
    csize = (2 * ratio) / (ratio * count + (count + 1));
    // psize is 20% of csize
    psize = csize / ratio;

    size_t idx_base = 0;
    GLfloat top = 1.0 - psize,
            left = -1.0 + psize;

    for (int y = 0; y < w->ylim; ++y) {
        GLfloat bottom = top - csize;
        for (int x = 0; x < w->xlim; ++x) {
            GLfloat right = left + csize;

#if DEBUG
            printf("x: % 3d y: % 3d t: % 4.3f l: % 4.3f b: % 4.3f r: % 4.3f\n", x, y, top, left, bottom, right);
#endif

            (*v)[idx_base+0] = (*v)[idx_base+6] = left; // left
            (*v)[idx_base+1] = (*v)[idx_base+7] = top; // top

            (*v)[idx_base+4] = (*v)[idx_base+8] = right; // right
            (*v)[idx_base+5] = (*v)[idx_base+9] = bottom; // bottom

            (*v)[idx_base+2] = left; // left
            (*v)[idx_base+3] = bottom; // bottom

            (*v)[idx_base+10] = right; // right
            (*v)[idx_base+11] = top; // top

            left += csize + psize;
            idx_base += 2 * VERTS_PER_TRIANGLE;
        }

        top -= csize + psize;
        left = -1.0 + psize;
#if DEBUG
        putchar('\n');
#endif
    }

    return vcount;
}

void start_game(game *g) {
    GLfloat *world_vertices;
    GLsizei world_vertices_count = _world_vertices(g->w, &world_vertices);

    float triangle_colours[] = {
        0.0f, 0.0f, 1.0f, 0.6f,
        0.0f, 1.0f, 0.0f, 0.6f,
        1.0f, 0.0f, 0.0f, 0.6f,
    };

    int ww, wh;
    SDL_GetWindowSize(g->win, &ww, &wh);

    mat4x4 MVP;
    float aspect = (float) ww / (float) wh,
          size = 1.0;

#if ORTHO
    mat4x4_ortho(MVP, -(aspect * size), aspect * size, -size, size, 0, 100);
#else
    mat4x4 Projection, View, Model, temp;

    vec3 eye    = {0.0, 0.0, 1.8},
         center = {0, 0, 0},
         up     = {0, 1, 0};

    mat4x4_perspective(Projection, 45.0f, aspect, 0.1f, 100.0f);
    mat4x4_look_at(View, eye, center, up);
    mat4x4_identity(Model);
    mat4x4_mul(temp, Projection, View);
    mat4x4_mul(MVP, temp, Model);
#endif

    GLuint matrix_id = glGetUniformLocation(g->gl_shader, "MVP");
    GLuint colors_id = glGetUniformLocation(g->gl_shader, "colors");

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint triangle_buffer;
    glGenBuffers(1, &triangle_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
    glBufferData(GL_ARRAY_BUFFER, world_vertices_count*sizeof(GLfloat), world_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    Uint32 start_loop = SDL_GetTicks();

    SDL_Event e;
    int game_running = 1;
    unsigned int count = 0, col = 0;
    while (game_running) {
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game_running = 0;
            }
            if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE) {
                game_running = 0;
            }
        }

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(g->gl_shader);

        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);

        glUniform4fv(colors_id, 3, &triangle_colours[col*4]);

        glBindBuffer(GL_ARRAY_BUFFER, triangle_buffer);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

        glDrawArrays(GL_TRIANGLES, 0, world_vertices_count / 2);

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);

        glUseProgram(0);

        SDL_GL_SwapWindow(g->win);
        /* SDL_Delay(20); */
        world_step(g->w);
#if 0
        ++count;
        if (count % 10 == 0) {
            ++col;
            if (col > 2) {
                col = 0;
            }
        }
#endif
    }
    Uint32 end_loop = SDL_GetTicks();
    float total = (end_loop - start_loop) / 1000.0;
    printf("World generations: %lu in %.4f seconds, %.4f gen/s\n", g->w->generation, total, g->w->generation / total);
}

void destroy_game(game *g) {
    _destroy_gfx(g);
    free_data_path();
    destroy_world(g->w);
    free(g);
}

