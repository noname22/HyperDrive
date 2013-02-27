#ifndef DEBUGWIDGET_H
#define DEBUGWIDGET_H

#include "hyper.h"
#include <SDL.h>

typedef struct DebugWidget DebugWidget;

DebugWidget* DW_Create(HyperMachine* hm);
void DW_Draw(DebugWidget* me, SDL_Surface* target, int ow, int oh);
bool DW_HandleEvent(DebugWidget* me, SDL_Event* event);

#endif
