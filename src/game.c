#include "game.h"

#define VERTS_PER_TRIANGLE 6
#define MULTISAMPLE 1
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

        GLint infoLogLength;
        glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar strInfoLog[infoLogLength];
        glGetProgramInfoLog(shader_program, infoLogLength, NULL, strInfoLog);

        printf("\nGLSL error: %s", strInfoLog);
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
        left = -(total_size/2.0) + psize;
#if DEBUG
        putchar('\n');
#endif
    }

    return vcount;
}

void start_game(game *g) {
    int ww, wh;
    SDL_GetWindowSize(g->win, &ww, &wh);

    float aspect = (float) ww / (float) wh;

    GLfloat *world_vertices;
    GLsizei world_vertices_count = _world_vertices(g->w, aspect, &world_vertices);

    float triangle_colours[] = {
        0.01568, 0.01568, 0.01568, 1.0,  // Background
        0.03137, 0.03137, 0.03137, 1.0,  // Dead
        1.00000, 1.00000, 1.00000, 1.0,  // Born
        0.26666, 0.26666, 0.26666, 1.0,  // Dying
        0.00000, 0.73333, 0.00000, 1.0,  // (Still) Alive
        0.00000, 0.73333, 0.00000, 1.0,  // (Still) Alive
    };

    mat4x4 MVP;

#if ORTHO
    float size = 1.0;
    mat4x4_ortho(MVP, -(aspect * size), aspect * size, -size, size, 0, 100);
#else
    mat4x4 Projection, View, Model, temp;

    vec3 eye    = {0.0, 0.0, 1.8},
         center = {0.0, 0.0, 0.0},
         up     = {0.0, 1.0, 0.0};

    mat4x4_perspective(Projection, 45.0f, aspect, 0.1f, 100.0f);
    mat4x4_look_at(View, eye, center, up);
    mat4x4_identity(Model);
    mat4x4_mul(temp, Projection, View);
    mat4x4_mul(MVP, temp, Model);
#endif

    int world_texture_id = 0;

    GLuint matrix_id = glGetUniformLocation(g->gl_shader, "MVP");
    GLuint colors_id = glGetUniformLocation(g->gl_shader, "colors");
    GLuint inv_state_id = glGetUniformLocation(g->gl_shader, "inv_state");
    GLuint world_texture_buffer = glGetUniformLocation(g->gl_shader, "world_texture_buffer");

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

    glUseProgram(g->gl_shader);
    glUniform4fv(colors_id, 5, &triangle_colours[4]);
    glUseProgram(0);


    Uint32 start_loop = SDL_GetTicks();
    glClearColor(
            triangle_colours[0],
            triangle_colours[1],
            triangle_colours[2],
            triangle_colours[3]);

    SDL_Event e;
    int game_running = 1;
    size_t count = 0;
    while (game_running) {
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(g->gl_shader);

        glUniformMatrix4fv(matrix_id, 1, GL_FALSE, &MVP[0][0]);
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
        glDisableVertexAttribArray(1);

        glUseProgram(0);

        SDL_GL_SwapWindow(g->win);

        // Events
        if (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game_running = 0;
            }
            if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_ESCAPE) {
                game_running = 0;
            }
            if (e.type == SDL_KEYUP && e.key.keysym.sym == SDLK_n) {
                world_half_step(g->w);
            }
        }

        // Update the world
        ++count;
        world_half_step(g->w);

        glBindBuffer(GL_TEXTURE_BUFFER, world_data_buffer);
        glBufferSubData(GL_TEXTURE_BUFFER, 0, g->w->data_size*sizeof(world_store), g->w->data);
        glBindBuffer(GL_TEXTURE_BUFFER, 0);
    }
    Uint32 end_loop = SDL_GetTicks();
    float total = (end_loop - start_loop) / 1000.0;
    printf("World generations: %lu in %.4f seconds, %.4f gen/s\n", count, total, count / total);
}

void destroy_game(game *g) {
    _destroy_gfx(g);
    free_data_path();
    destroy_world(g->w);
    free(g);
}

