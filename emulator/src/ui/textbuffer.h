#ifndef TEXTBUFFER_H
#define TEXTBUFFER_H

#include <SDL.h>

typedef struct TextBuffer TextBuffer;

TextBuffer* TB_Create();

void TB_AddLine(TextBuffer* me, const char* text);
void TB_Draw(TextBuffer* me, SDL_Surface* target, int x, int y, int numLines);

#endif
