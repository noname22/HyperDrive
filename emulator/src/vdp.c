#define LDEBUG
#include <stdlib.h>
#include <string.h>

#include "vdp.h"
#include "mem.h"
#include "cpu.h"
#include "log.h"

struct Vdp {
	int w, h;
	int y;
	uint8_t* vMem;
	uint8_t* p;
	Mem* mem;
	Cpu* cpu;
};

typedef struct {
	uint32_t mode, tileset, palette, data;
	int32_t  x, y, w, h;
} VLayer;

Vdp* Vdp_Create(Cpu* cpu, Mem* mem, int w, int h, uint8_t* vMem)
{
	Vdp* me = calloc(1, sizeof(Vdp));
	LAssert(me, "could not allocate ram for vdp");

	me->w = w;
	me->h = h;
	me->vMem = vMem;
	me->p = vMem;
	me->mem = mem;
	me->cpu = cpu;

	return me;
}

void Vdp_LayerMode1ScanLine(Vdp* me, VLayer* layer, uint8_t* px)
{
	// Bitmap mode
	px -= 3;
	for(int x = 0; x < me->w; x++){
		px += 3;

		int xx = x - layer->x;
		if(xx < 0) continue;
		if(xx >= layer->w) break;

		int yy = me->y - layer->y;
		if(yy < 0) continue;
		if(yy >= layer->h) break;

		int idx = xx + yy * layer->w;
		if(idx >= MEM_SIZE) continue;

		uint8_t spx = MEM_READ8(me->mem, layer->data + idx) * 3;
		
		*(px + 2) = *MEM_READ_PTR(me->mem, layer->palette + spx++);
		*(px + 1) = *MEM_READ_PTR(me->mem, layer->palette + spx++);
		*(px + 0) = *MEM_READ_PTR(me->mem, layer->palette + spx);
	}
}

bool Vdp_HandleScanLine(Vdp* me)
{
	static bool once = false;
	memset(me->p, 0, 3 * me->w);

	for(int i = 0; i < 8; i++){
		uint8_t* px = me->p;
		VLayer layer;

		uint32_t lAddr = MEM_VDP_BASE + i * 8 * 4;

		layer.mode = MEM_READ32(me->mem, lAddr); lAddr += 4;

		if(!layer.mode) continue;

		layer.w = MEM_READ32(me->mem, lAddr); lAddr += 4;
		layer.h = MEM_READ32(me->mem, lAddr); lAddr += 4;
		
		layer.x = MEM_READ32(me->mem, lAddr); lAddr += 4;
		layer.y = MEM_READ32(me->mem, lAddr); lAddr += 4;

		layer.tileset = MEM_READ32(me->mem, lAddr); lAddr += 4;
		layer.palette = MEM_READ32(me->mem, lAddr); lAddr += 4;
		layer.data = MEM_READ32(me->mem, lAddr); lAddr += 4;

		if(!once){
			LogD("layer:   %d", i);
			LogD("mode:    %d", layer.mode);
			LogD("w, h:    %d, %d", layer.w, layer.h);
			LogD("x, y:    %d, %d", layer.x, layer.y);
			LogD("tileset: %08x", layer.tileset);
			LogD("palette: %08x", layer.palette);
			LogD("data:    %08x", layer.data);
			once = true;
		}

		Vdp_LayerMode1ScanLine(me, &layer, px);
		
		Cpu_Interrupt(me->cpu, CI_Hblank);
	}
	me->p += me->w * 3;

	if(++me->y >= me->h){
		me->p = me->vMem;
		me->y = 0;

		Cpu_Interrupt(me->cpu, CI_Vblank);
		return false;
	}

	return true;
}
