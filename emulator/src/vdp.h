#ifndef VDP_H
#define VDP_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Vdp Vdp;
typedef struct Mem Mem;

Vdp* Vdp_Create(Mem* mem, int w, int h, uint8_t* vMem);
bool Vdp_HandleScanLine(Vdp* me);

#endif
