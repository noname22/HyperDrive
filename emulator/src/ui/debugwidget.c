#include "debugwidget.h"

#include "log.h"
#include "debug.h"
#include "textbuffer.h"
#include "utils.h"
#include "vdp.h"
#include "cpu.h"
#include "mem.h"

typedef struct {
	char buffer[1024];
	int at;
} InputBox;

struct DebugWidget {
	Debug* debug;
	HyperMachine* hm;
	Vdp* vdp;
	Cpu* cpu;
	Mem* mem;
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
	me->vdp = HM_GetVdp(hm);
	me->cpu = HM_GetCpu(hm);
	me->mem = HM_GetMem(hm);

	return me;
}

void DW_DrawVdpLayerInfo(DebugWidget* me, SDL_Surface* target, int x, int y, int layerNum)
{
	VLayer layer;
	Vdp_GetLayerData(me->vdp, &layer, layerNum);

	SDL_Color color = {255, 255, 255, 255};
	int yy = y;

	#define DTEXT(__t, ...) DrawText(target, x, y += 8, color, __t, __VA_ARGS__);
			
	DTEXT("layer:     %d", layerNum);
	DTEXT("mode:      %d", layer.mode);
	DTEXT("w, h:      %d, %d", layer.w, layer.h);
	DTEXT("x, y:      %d, %d", layer.x, layer.y);

	x += 192;
	y = yy;
 
	DTEXT("tileset:   %08x", layer.tileset);
	DTEXT("palette:   %08x", layer.palette);
	DTEXT("data:      %08x", layer.data);
	DTEXT("color key: %02x", layer.colorKey);
	DTEXT("blend mode: %02x", layer.blendMode);

	uint8_t* pal = calloc(1, 256 * 3 * 8);

	if(layer.mode != 0){
		for(int ty = 0; ty < 8; ty++){
			for(int tx = 0; tx < 768; tx++){
				pal[tx + ty * 768] = MEM_READ8(me->mem, layer.palette + tx);
			}
		}
	}

	SDL_Surface* ps = SDL_CreateRGBSurfaceFrom(pal, 256, 8, 24, 768, 0xff, 0xff00, 0xff0000, 0);
	SDL_Rect r = {x - 192, y + 8};

	SDL_BlitSurface(ps, NULL, target, &r);

	SDL_FreeSurface(ps);
	free(pal);
}

void DW_Draw(DebugWidget* me, SDL_Surface* target, int ow, int oh)
{
	SDL_Color white = {255, 255, 255, 255};
	TB_Draw(me->dbgLog, target, 8, oh, (target->h - oh) / 8 - 2);

	me->active = !Debug_GetRunning(me->debug);

	for(int i = 0; i < 8; i++){
		DW_DrawVdpLayerInfo(me, target, ow + 8, 8 + i * 7 * 8, i);
	}

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
