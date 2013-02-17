#ifndef HYPER_H
#define HYPER_H

#include <stdint.h>

typedef struct HyperMachine HyperMachine;

HyperMachine* HM_Create(int w, int h, uint8_t* display);

void HM_Tick(HyperMachine* me);

#endif
