#ifndef UTILS_H
#define UTILS_H

#include <SDL.h>

void DrawStr(SDL_Surface* target, int x, int y, SDL_Color color, const char* string);
void DrawText(SDL_Surface* target, int x, int y, SDL_Color color, const char* format, ...);

#endif
