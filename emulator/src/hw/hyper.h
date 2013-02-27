#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct HyperMachine HyperMachine;
typedef struct Debug Debug;

HyperMachine* HM_Create(int w, int h, uint8_t* display, bool debug, int freq);

void HM_Tick(HyperMachine* me);
bool HM_LoadRom(HyperMachine* me, const char* filename, const char* debugFilename);
Debug* HM_GetDebugger(HyperMachine* me);

#endif
