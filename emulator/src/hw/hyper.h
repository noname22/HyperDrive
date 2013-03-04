#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>
#include <stdbool.h>

#include "htypes.h"

typedef enum {
	HM_A = 1, HM_B = 2, HM_C = 4, HM_D = 8, HM_Select = 16, HM_Start = 32, MM_L = 64, HM_R = 128, HM_Up = 256, HM_Down = 512, HM_Left = 1024, HM_Right = 2048
} HM_Button;

HyperMachine* HM_Create(int w, int h, uint8_t* display, bool debug, int freq);

void HM_Tick(HyperMachine* me);
bool HM_LoadRom(HyperMachine* me, const char* filename, const char* debugFilename);
Debug* HM_GetDebugger(HyperMachine* me);

Vdp* HM_GetVdp(HyperMachine* me);
Cpu* HM_GetCpu(HyperMachine* me); 
Mem* HM_GetMem(HyperMachine* me);

void HM_TriggerInput(HyperMachine* me, int controller, HM_Button button, bool state);

#endif
