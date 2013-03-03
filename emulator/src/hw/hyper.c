#define LDEBUG
#include "hyper.h"
#include "log.h"
#include <stdlib.h>
#include "common.h"

#include "mem.h"
#include "cpu.h"
#include "vdp.h"

#include "debug.h"

struct HyperMachine {
	Vdp* vdp;
	Cpu* cpu;
	Mem* mem;

	Debug* debug;

	int insPerScanLine;
	int insLeft;
	bool vdpDone;
};

void HM_SysWrite(Cpu* cpu, void* data)
{
	HyperMachine* me = data;
	uint32_t addr = Cpu_Pop(cpu);
	char c;
	while((c = MEM_READ8(me->mem, addr++))){
		fputc(c, stdout);
	}
}

HyperMachine* HM_Create(int w, int h, uint8_t* display, bool debug, int freq)
{
	HyperMachine* me = calloc(1, sizeof(HyperMachine));
	LAssert(me, "could not allocate a machine");	

	me->mem = Mem_Create();
	me->cpu = Cpu_Create(me->mem);
	me->vdp = Vdp_Create(me->cpu, me->mem, w, h, display);

	me->insPerScanLine = (int)((float)freq / 60.0 / (float)h);

	Cpu_SetSysCall(me->cpu, HM_SysWrite, 1, me);

	if(debug)
		me->debug = Debug_Create(me->cpu, me->mem);

	return me;
}

void HM_Tick(HyperMachine* me)
{
	while(true){
		if(!me->vdpDone && me->insLeft <= 0){
			// no instructions left to execute for this scanline, start a new one
			me->vdpDone = !Vdp_HandleScanLine(me->vdp);
			me->insLeft = me->insPerScanLine;
		}

		me->insLeft -= Cpu_Execute(me->cpu, me->insLeft);

		if(me->insLeft > 0){
			// CPU didn't finish the specified number of cycles, it's paused.
			// Continue executing on the next call
			break;
		}else if(me->vdpDone){
			// the CPU did finish, and the VDP is also done. Frame complete.
			// Prepare for next;
			
			me->vdpDone = false;
			break;
		}
	}
}

bool HM_LoadRom(HyperMachine* me, const char* filename, const char* debugFilename)
{
	uint8_t* rom = NULL;
	int size = ReadFileAlloc(&rom, filename);

	if(size < 0) return false;

	LogV("Loaded rom with size: %d", size);
	Mem_SetROM(me->mem, rom, size);

	free(rom);
	
	if(debugFilename)
		Debug_LoadSymbols(me->debug, debugFilename);

	return true;
}

Debug* HM_GetDebugger(HyperMachine* me)
{
	return me->debug; 
}

Vdp* HM_GetVdp(HyperMachine* me)
{
	return me->vdp;
}

Cpu* HM_GetCpu(HyperMachine* me)
{
	return me->cpu;
}

Mem* HM_GetMem(HyperMachine* me)
{
	return me->mem;
}
