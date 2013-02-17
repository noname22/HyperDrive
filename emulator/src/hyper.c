#include "hyper.h"
#include "log.h"
#include <stdlib.h>
#include "cpu.h"
#include "common.h"

struct HyperMachine {
	int w, h;
	uint8_t* display;
	Cpu* cpu;
	uint8_t* ram;
	uint32_t memSize;
};

HyperMachine* HM_Create(int w, int h, uint8_t* display, uint32_t memSize)
{
	HyperMachine* me = calloc(1, sizeof(HyperMachine));
	LAssert(me, "could not allocate a machine");	

	me->display = display;
	me->w = w;
	me->h = h;

	me->ram = calloc(1, memSize);
	LAssert(me->ram, "could not allocate memory for machine RAM (%u bytes)", memSize);

	me->cpu = Cpu_Create(me->ram, memSize);
	me->memSize = memSize;

	return me;
}

void HM_Tick(HyperMachine* me)
{
	uint8_t* pill = me->display;
	for(int y = 0; y < me->h; y++){
		for(int x = 0; x < me->w * 3; x++){
			*(pill++) = rand() % 256;
		}
	}
	Cpu_Execute(me->cpu, 10);
}

bool HM_LoadRom(HyperMachine* me, const char* filename)
{
	if(ReadFile(me->ram, me->memSize, filename) < 0){
		LogW("could not load file: %s", filename);
		return false;
	}
	return true;
}
