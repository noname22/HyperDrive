#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct HyperMachine HyperMachine;
typedef struct Debug Debug;
typedef struct Vdp Vdp;
typedef struct Cpu Cpu;
typedef struct Mem Mem;

HyperMachine* HM_Create(int w, int h, uint8_t* display, bool debug, int freq);

void HM_Tick(HyperMachine* me);
bool HM_LoadRom(HyperMachine* me, const char* filename, const char* debugFilename);
Debug* HM_GetDebugger(HyperMachine* me);

Vdp* HM_GetVdp(HyperMachine* me);
Cpu* HM_GetCpu(HyperMachine* me); 
Mem* HM_GetMem(HyperMachine* me);


#endif
