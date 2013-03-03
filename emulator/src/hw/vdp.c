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

void Vdp_GetLayerData(Vdp* me, VLayer* layer, int layerNum)
{
	uint32_t lAddr = MEM_VDP_BASE + layerNum * 16 * 4;

	layer->mode = MEM_READ32(me->mem, lAddr); lAddr += 4;

	layer->w = MEM_READ32(me->mem, lAddr); lAddr += 4;
	layer->h = MEM_READ32(me->mem, lAddr); lAddr += 4;

	layer->x = MEM_READ32(me->mem, lAddr); lAddr += 4;
	layer->y = MEM_READ32(me->mem, lAddr); lAddr += 4;

	layer->tileset = MEM_READ32(me->mem, lAddr); lAddr += 4;
	layer->palette = MEM_READ32(me->mem, lAddr); lAddr += 4;
	layer->data = MEM_READ32(me->mem, lAddr); lAddr += 4;

	layer->colorKey = MEM_READ8(me->mem, lAddr); lAddr++;
	layer->blendMode = MEM_READ8(me->mem, lAddr); lAddr++;
	layer->flags = MEM_READ8(me->mem, lAddr); lAddr++;

	layer->mirrorX = (layer->flags & VF_MirrorX) != 0;
	layer->mirrorY = (layer->flags & VF_MirrorX) != 0;
	layer->loopX = (layer->flags & VF_LoopX) != 0;
	layer->loopY = (layer->flags & VF_LoopY) != 0;

	if(layer->mode > 2){
		layer->tileSize = 1 << layer->mode;
	}else{
		layer->tileSize = 0;
	}
}

#define PX(px_, spx_)\
		switch(layer->blendMode){\
			case VB_Replace:\
				*(px_++) = *MEM_READ_PTR(me->mem, layer->palette + spx_++);\
				*(px_++) = *MEM_READ_PTR(me->mem, layer->palette + spx_++);\
				*(px_++) = *MEM_READ_PTR(me->mem, layer->palette + spx_);\
				break;\
			case VB_Add:\
				*(px_) = VMIN((int)*px_ + *MEM_READ_PTR(me->mem, layer->palette + spx_), 255);\
				px_++; spx_++;\
				*(px_) = VMIN((int)*px_ + *MEM_READ_PTR(me->mem, layer->palette + spx_), 255);\
				px_++; spx_++;\
				*(px_) = VMIN((int)*px_ + *MEM_READ_PTR(me->mem, layer->palette + spx_), 255);\
				px_++; spx_++;\
				break;\
\
			case VB_Subtract:\
				*(px_) = VMAX((int)*px_ - *MEM_READ_PTR(me->mem, layer->palette + spx_), 0);\
				px_++; spx_++;\
				*(px_) = VMAX((int)*px_ - *MEM_READ_PTR(me->mem, layer->palette + spx_), 0);\
				px_++; spx_++;\
				*(px_) = VMAX((int)*px_ - *MEM_READ_PTR(me->mem, layer->palette + spx_), 0);\
				px_++; spx_++;\
				break;\
		}

void Vdp_LayerMode1ScanLine(Vdp* me, VLayer* layer, uint8_t* px)
{
	// Bitmap mode
	for(int x = 0; x < me->w; x++){
		int xx = x - layer->x;
		if(layer->mirrorX)
			xx = (xx - layer->w + 1) * -1;
		
		if(layer->loopX){
			xx %= layer->w;
			if(xx < 0)
				xx += layer->w;
		}else{
			if(xx < 0)
				goto no_draw;

			if(xx >= layer->w)
				break;
		}

		int yy = me->y - layer->y;

		if(layer->loopY){
			yy %= layer->h;
			if(yy < 0)
				yy += layer->h;
		}else{
			if(layer->mirrorY)
				yy = (yy - layer->h + 1) * -1;

			if(yy < 0)
				goto no_draw;

			if(yy >= layer->h)
				break;
		}

		int idx = xx + yy * layer->w;
		if(idx >= MEM_SIZE)
			goto no_draw;

		int spx = MEM_READ8(me->mem, layer->data + idx) * 3;
		if(spx == layer->colorKey * 3)
			goto no_draw;
		
		PX(px, spx)
		continue;

		no_draw:
		px += 3;
	}
}

void Vdp_LayerScanLine(Vdp* me, VLayer* layer, uint8_t* px)
{
	int wPx = layer->w * layer->tileSize;
	int hPx = layer->h * layer->tileSize;

	for(int x = 0; x < me->w; x++){
		int xx = x - layer->x;
		if(layer->mirrorX)
			xx = (xx - wPx + 1) * -1;
		
		if(layer->loopX){
			xx %= wPx;
			if(xx < 0)
				xx += wPx;
		}else{
			if(xx < 0)
				goto no_draw;

			if(xx >= wPx)
				break;
		}

		int yy = me->y - layer->y;
		if(layer->mirrorY)
			yy = (xx - hPx + 1) * -1;

		if(layer->loopY){
			yy %= hPx;
			if(yy < 0)
				yy += hPx;
		}else{
			if(yy < 0)
				goto no_draw;

			if(yy >= layer->h * layer->tileSize)
				break;
		}
		
		int tilePos = xx / layer->tileSize + yy / layer->tileSize * layer->w;

		int idx = layer->data + tilePos * 3;
		int tileNum = MEM_READ16(me->mem, idx);
		uint8_t flags = MEM_READ8(me->mem, idx + 2);

		uint32_t tile = MEM_READ32(me->mem, layer->tileset + tileNum * 4);

		//LogD("%d -> 0x%x", tileNum, tile);

		int xTile = xx % layer->tileSize;
		int yTile = yy % layer->tileSize;

		if(flags & VF_MirrorX)
			xTile = (xTile - layer->tileSize + 1) * -1;
		
		if(flags & VF_MirrorY)
			yTile = (yTile - layer->tileSize + 1) * -1;

		uint32_t spx = MEM_READ8(me->mem, tile + xTile + yTile * layer->tileSize) * 3;

		if(spx == layer->colorKey * 3)
			goto no_draw;
		
		PX(px, spx)
		continue;

		no_draw:
		px += 3;
	}
}

bool Vdp_HandleScanLine(Vdp* me)
{
	static bool once[256] = {false};
	memset(me->p, 0, 3 * me->w);

	for(int i = 0; i < 256; i++){
		uint8_t* px = me->p;
		VLayer layer;

		Vdp_GetLayerData(me, &layer, i);
		
		if(layer.mode == 0)
			continue;

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
			LogD("flags:");
			LogD("  mirrorX: %d", layer.mirrorX);
			LogD("  mirrorY: %d", layer.mirrorY);
			LogD("  loopX:   %d", layer.loopX);
			LogD("  loopY:   %d", layer.loopY);
			once[i] = true;
		}

		if(layer.mode == 1)
			Vdp_LayerMode1ScanLine(me, &layer, px);
		else
			Vdp_LayerScanLine(me, &layer, px);
		
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
