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

typedef enum {
	VB_Replace,
	VB_Add,
	VB_Subtract
} Vdp_BlendMode;


typedef struct {
	uint32_t mode, tileset, palette, data;
	int32_t  x, y, w, h;
	uint8_t colorKey;
	Vdp_BlendMode blendMode;
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

#define VMAX(_x, _y) (_y) ^ (((_x) ^ (_y)) & -((_x) < (_y)))
#define VMIN(_x, _y) (_x) ^ (((_x) ^ (_y)) & -((_x) < (_y)))

void Vdp_LayerMode1ScanLine(Vdp* me, VLayer* layer, uint8_t* px)
{
	// Bitmap mode
	for(int x = 0; x < me->w; x++){
		int xx = x - layer->x;
		if(xx < 0)
			goto no_draw;

		if(xx >= layer->w)
			break;

		int yy = me->y - layer->y;
		if(yy < 0)
			goto no_draw;

		if(yy >= layer->h)
			break;

		int idx = xx + yy * layer->w;
		if(idx >= MEM_SIZE)
			goto no_draw;

		int spx = MEM_READ8(me->mem, layer->data + idx) * 3;
		if(spx == layer->colorKey)
			goto no_draw;
		
		switch(layer->blendMode){
			case VB_Replace:
				*(px++) = *MEM_READ_PTR(me->mem, layer->palette + spx++);
				*(px++) = *MEM_READ_PTR(me->mem, layer->palette + spx++);
				*(px++) = *MEM_READ_PTR(me->mem, layer->palette + spx);
				break;
			case VB_Add:
				*(px) = VMIN((int)*px + *MEM_READ_PTR(me->mem, layer->palette + spx), 255);
				px++; spx++;
				*(px) = VMIN((int)*px + *MEM_READ_PTR(me->mem, layer->palette + spx), 255);
				px++; spx++;
				*(px) = VMIN((int)*px + *MEM_READ_PTR(me->mem, layer->palette + spx), 255);
				px++; spx++;

				break;

			case VB_Subtract:
				*(px) = VMAX((int)*px - *MEM_READ_PTR(me->mem, layer->palette + spx), 0);
				px++; spx++;
				*(px) = VMAX((int)*px - *MEM_READ_PTR(me->mem, layer->palette + spx), 0);
				px++; spx++;
				*(px) = VMAX((int)*px - *MEM_READ_PTR(me->mem, layer->palette + spx), 0);
				px++; spx++;
				break;
		}
		continue;
		no_draw:
		px += 3;
	}
}

bool Vdp_HandleScanLine(Vdp* me)
{
	static bool once[8] = {false};
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

		layer.colorKey = MEM_READ8(me->mem, lAddr); lAddr++;
		layer.blendMode = MEM_READ8(me->mem, lAddr); lAddr++;

		if(!once[i]){
			LogD("layer:     %d", i);
			LogD("mode:      %d", layer.mode);
			LogD("w, h:      %d, %d", layer.w, layer.h);
			LogD("x, y:      %d, %d", layer.x, layer.y);
			LogD("tileset:   %08x", layer.tileset);
			LogD("palette:   %08x", layer.palette);
			LogD("data:      %08x", layer.data);
			LogD("color key: %02x", layer.colorKey);
			LogD("blend mode: %02x", layer.blendMode);
			once[i] = true;
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
