#include <stdlib.h>

#include "vdp.h"
#include "mem.h"
#include "log.h"

struct Vdp {
	int w, h;
	int y;
	uint8_t* vMem;
	uint8_t* p;
	Mem* mem;
};

typedef struct {
	uint32_t mode, w, h, tileset, palette, data;
	uint32_t palSize, dataSize, tilesetSize;
	int32_t  x, y;
	uint8_t* tilesetPtr, *dataPtr, *palPtr;
} VLayer;

Vdp* Vdp_Create(Mem* mem, int w, int h, uint8_t* vMem)
{
	Vdp* me = calloc(1, sizeof(Vdp));
	LAssert(me, "could not allocate ram for vdp");

	me->w = w;
	me->h = h;
	me->vMem = vMem;
	me->p = vMem;
	me->mem = mem;

	return me;
}

void Vdp_LayerMode1ScanLine(Vdp* me, VLayer* layer, uint8_t* px)
{
	// Bitmap mode
	for(int x = 0; x < me->w; x++){
		int idx = x - layer->x + (me->y + layer->y) * layer->w;
		uint8_t spx = idx > 0 && idx < layer->dataSize ? layer->dataPtr[idx] : 0;
		if(!spx) continue;
		
		*(px++) = layer->palPtr[spx % layer->palSize];
		*(px++) = layer->palPtr[(spx + 1) % layer->palSize];
		*(px++) = layer->palPtr[(spx + 2) % layer->palSize];
	}
}

bool Vdp_HandleScanLine(Vdp* me)
{
	for(int i = 0; i < 8; i++){
		uint8_t* px = me->p;
		VLayer layer;

		uint32_t lAddr = MEM_VDP_BASE + i * 8 * 4;

		layer.mode = Mem_Read32(me->mem, lAddr); lAddr += 4;

		if(!layer.mode) continue;

		layer.w = Mem_Read32(me->mem, lAddr); lAddr += 4;
		layer.h = Mem_Read32(me->mem, lAddr); lAddr += 4;
		
		layer.x = Mem_Read32(me->mem, lAddr); lAddr += 4;
		layer.y = Mem_Read32(me->mem, lAddr); lAddr += 4;

		layer.tileset = Mem_Read32(me->mem, lAddr); lAddr += 4;
		layer.palette = Mem_Read32(me->mem, lAddr); lAddr += 4;
		layer.data = Mem_Read32(me->mem, lAddr); lAddr += 4;

		layer.tilesetPtr = Mem_GetPtr(me->mem, layer.tileset, &layer.tilesetSize);
		layer.palPtr = Mem_GetPtr(me->mem, layer.palette, &layer.palSize);
		layer.dataPtr = Mem_GetPtr(me->mem, layer.data, &layer.dataSize);

		Vdp_LayerMode1ScanLine(me, &layer, px);
	}

	if(++me->y >= me->h){
		me->p = me->vMem;
		me->y = 0;
		return false;
	}

	return true;
}
