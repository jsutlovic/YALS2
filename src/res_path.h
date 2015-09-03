#ifndef _RES_PATH_H
#define _RES_PATH_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>

void free_data_path();
char* get_res_path(const char *sub_dir);
char* join_path(char *path1, char *path2);

#endif

/* vim: set ft=c : */
