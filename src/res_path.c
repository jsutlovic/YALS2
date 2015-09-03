#include "res_path.h"

const char PATH_SEP = '/';
static char* data_path = NULL;
static int data_path_len;

void free_data_path() {
    SDL_free(data_path);
}

char* get_res_path(const char *sub_dir) {
    if (!data_path) {
        char *base_path = SDL_GetBasePath();
        if (base_path) {
            data_path = SDL_strdup(base_path);
            data_path_len = SDL_strlen(data_path);
            SDL_free(base_path);
        } else {
            fputs("Couldn't get SDL base path!\n", stderr);
            data_path = NULL;
            data_path_len = 0;
            return NULL;
        }

        // This is bad, but we don't have rindex for substrings
        char *replace = NULL;
        char *match = data_path;
        while (match != NULL) {
            match = SDL_strstr(match, "bin");

            if (match != NULL) {
                replace = match;
                match += 3;
            }
        }

        if (!replace) {
            fputs("Couldn't find path to replace!\n", stderr);
            data_path = NULL;
            data_path_len = 0;
            return NULL;
        }

        SDL_memcpy(replace, "res", 3);
    }

    size_t sub_dir_len = SDL_strlen(sub_dir);
    size_t new_dir_len = data_path_len + sub_dir_len + 1;
    char *new_dir = SDL_malloc(new_dir_len);
    SDL_strlcpy(new_dir, data_path, new_dir_len);
    SDL_strlcpy(&new_dir[data_path_len], sub_dir, sub_dir_len + 1);

    if (new_dir[data_path_len + sub_dir_len - 1] != '/') {
        SDL_strlcpy(&new_dir[data_path_len + sub_dir_len], "/", 2);
    }

    return new_dir;
}

char* join_path(char *path1, char *path2) {
    size_t path1_len = SDL_strlen(path1);
    size_t path2_len = SDL_strlen(path2);
    size_t joined_len = path1_len + path2_len;

    char *joined = SDL_malloc(joined_len);
    SDL_strlcpy(joined, path1, joined_len);
    SDL_strlcpy(&joined[path1_len], path2, joined_len);

    return joined;
}

