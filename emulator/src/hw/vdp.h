#ifndef VDP_H
#define VDP_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Vdp Vdp;
typedef struct Mem Mem;
typedef struct Cpu Cpu;

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
	int tileSize;
} VLayer;

Vdp* Vdp_Create(Cpu* cpu, Mem* mem, int w, int h, uint8_t* vMem);
bool Vdp_HandleScanLine(Vdp* me);
void Vdp_GetLayerData(Vdp* me, VLayer* layer, int layerNum);

#endif
