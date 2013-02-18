#include "hyper.h"
#include "log.h"
#include <stdlib.h>
#include "common.h"

#include "mem.h"
#include "cpu.h"

struct HyperMachine {
	int w, h;
	uint8_t* display;
	Cpu* cpu;
	Mem* mem;
};

HyperMachine* HM_Create(int w, int h, uint8_t* display)
{
	HyperMachine* me = calloc(1, sizeof(HyperMachine));
	LAssert(me, "could not allocate a machine");	

	me->display = display;
	me->w = w;
	me->h = h;

	me->mem = Mem_Create();
	me->cpu = Cpu_Create(me->mem);

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
	uint8_t* rom = NULL;
	int size = ReadFileAlloc(&rom, filename);

	if(size < 0) return false;

	LogV("Loaded rom with size: %d", size);
	Mem_SetROM(me->mem, rom, size);

	free(rom);

	return true;
}
