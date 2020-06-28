#ifndef _RES_PATH_H
#define _RES_PATH_H

#ifdef __unix__
#include <SDL2/SDL.h>
#include <SDL2/SDL_stdinc.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>
#include <SDL_stdinc.h>
#endif

void free_data_path();
char* get_res_path(const char *sub_dir);
char* join_path(char *path1, char *path2);

#endif

/* vim: set ft=c : */
