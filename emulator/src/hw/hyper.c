#define LDEBUG
#include "hyper.h"
#include "log.h"
#include <stdlib.h>
#include "common.h"
#include <math.h>

#include "mem.h"
#include "cpu.h"
#include "vdp.h"
#include "apu.h"

#include "debug.h"

struct HyperMachine {
	Vdp* vdp;
	Apu* apu;
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

HyperMachine* HM_Create(int w, int h, double frameRate, uint8_t* display, bool debug, int freq)
{
	HyperMachine* me = calloc(1, sizeof(HyperMachine));
	LAssert(me, "could not allocate a machine");	

	me->mem = Mem_Create();
	me->cpu = Cpu_Create(me->mem);
	me->vdp = Vdp_Create(me->cpu, me->mem, w, h, display);
	me->apu = Apu_Create(me->mem);

	me->insPerScanLine = (int)((float)freq / frameRate / (float)h);

	Cpu_SetSysCall(me->cpu, HM_SysWrite, 1, me);

	if(debug)
		me->debug = Debug_Create(me->cpu, me->mem);

	return me;
}

void HM_ProcessFrame(HyperMachine* me)
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
		}else{
			// CPU finished scanline. Process Audio.
			if(me->vdpDone){
				// The VDP is also done. Frame complete.
				// Prepare for next;
				
				me->vdpDone = false;
				break;
			}
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

void HM_TriggerInput(HyperMachine* me, int controller, HM_Button button, bool state)
{
	uint32_t idx = MEM_INPUT_BASE + (4 * (controller % 4));
	uint32_t c = MEM_READ32(me->mem, idx);

	if(state)
		c |= button;
	else
		c &= ~button;

	MEM_WRITE32(me->mem, idx, c);

	Cpu_Interrupt(me->cpu, CI_Input);
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

Apu* HM_GetApu(HyperMachine* me)
{
	return me->apu;
}
