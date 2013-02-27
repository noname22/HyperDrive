#include "debugwidget.h"

#include "log.h"
#include "debug.h"
#include "textbuffer.h"
#include "utils.h"

typedef struct {
	char buffer[1024];
	int at;
} InputBox;

struct DebugWidget {
	Debug* debug;
	HyperMachine* hm;
	TextBuffer* dbgLog;
	bool active;

	InputBox input;
};

void DW_DebugPrinter(Debug* d, void* vMe, const char* format, ...)
{
	DebugWidget* me = (DebugWidget*)vMe;
	char buffer[1024];
	memset(buffer, '\0', sizeof(buffer));

	va_list fmtargs;
	va_start(fmtargs, format);

	vsnprintf(buffer, sizeof(buffer) - 1, format, fmtargs);

	va_end(fmtargs);

	TB_AddLine(me->dbgLog, buffer);
}

DebugWidget* DW_Create(HyperMachine* hm)
{
	DebugWidget* me = calloc(1, sizeof(DebugWidget));
	LAssert(me, "could not allocate ram for debugwidget");

	me->hm = hm;
	me->debug = HM_GetDebugger(hm);
	
	Debug_SetPrintfHandler(me->debug, DW_DebugPrinter, me);

	me->dbgLog = TB_Create();

	return me;
}

void DW_Draw(DebugWidget* me, SDL_Surface* target, int ow, int oh)
{
	SDL_Color white = {255, 255, 255, 255};
	TB_Draw(me->dbgLog, target, 8, oh, (target->h - oh) / 8 - 2);

	me->active = !Debug_GetRunning(me->debug);
	

	if(me->active){
		DrawText(target, 8, target->h - 16, white, "> %s_", me->input.buffer);
	}
}

bool DW_HandleEvent(DebugWidget* me, SDL_Event* event)
{
	if(me->active && event->type == SDL_KEYDOWN){
		switch(event->key.keysym.sym){
			case SDLK_BACKSPACE: if(me->input.at > 0) me->input.buffer[--me->input.at] = 0; break;
			case SDLK_RETURN:
				Debug_HandleInput(me->debug, me->input.buffer);
				memset(me->input.buffer, 0, 1024);
				me->input.at = 0;
				return true;
			default:
				if(event->key.keysym.unicode){
					me->input.buffer[me->input.at++] = event->key.keysym.unicode;
					return true;
				}
		}
	}

	else if(!me->active && event->type == SDL_KEYDOWN){
		switch(event->key.keysym.sym){
			case SDLK_c:
				if(event->key.keysym.mod & KMOD_CTRL){
					LogV("debug mode");
					Debug_Break(me->debug);
					return true;
				}
			default: break;
		}
	}

	return false;
}
