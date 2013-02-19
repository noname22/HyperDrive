//#define LDEBUG
#include "hyper.h"
#include "log.h"
#include <stdlib.h>
#include "common.h"

#include "mem.h"
#include "cpu.h"
#include "vdp.h"

struct HyperMachine {
	Vdp* vdp;
	Cpu* cpu;
	Mem* mem;

	int insPerScanLine;
};

void HM_SysWrite(Cpu* cpu, void* data)
{
	HyperMachine* me = data;
	uint32_t addr = Cpu_Pop(cpu);
	char c;
	while((c = Mem_Read8(me->mem, addr++))){
		fputc(c, stdout);
	}
}

HyperMachine* HM_Create(int w, int h, uint8_t* display)
{
	HyperMachine* me = calloc(1, sizeof(HyperMachine));
	LAssert(me, "could not allocate a machine");	

	me->mem = Mem_Create();
	me->cpu = Cpu_Create(me->mem);
	me->vdp = Vdp_Create(me->mem, w, h, display);

	me->insPerScanLine = (int)(8000000.0 / 60.0 / (float)h);

	Cpu_SetSysCall(me->cpu, HM_SysWrite, 1, me);

	return me;
}

void HM_Tick(HyperMachine* me)
{
	LogD("tick");
	while(Vdp_HandleScanLine(me->vdp)){
		Cpu_Execute(me->cpu, me->insPerScanLine);
	}
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
