#ifndef VDP_H
#define VDP_H

#include <stdint.h>
#include <stdbool.h>

#include "htypes.h"

typedef enum {
	VB_Replace,
	VB_Add,
	VB_Subtract
} Vdp_BlendMode;

typedef enum {
	VF_MirrorX = 1,
	VF_MirrorY = 2,
	VF_LoopX = 4,
	VF_LoopY = 8
} Vdp_Flags;

typedef struct {
	uint32_t mode, tileset, palette, data;
	int32_t  x, y, w, h;
	uint8_t colorKey;
	Vdp_BlendMode blendMode;
	uint8_t flags;

	int tileSize;
	bool mirrorX, mirrorY, loopX, loopY;
} VLayer;

Vdp* Vdp_Create(Cpu* cpu, Mem* mem, int w, int h, uint8_t* vMem);
bool Vdp_HandleScanLine(Vdp* me);
void Vdp_GetLayerData(Vdp* me, VLayer* layer, int layerNum);

#endif
