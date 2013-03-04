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

	int smp;
	int smpPerLine;
	int extraSmpNth;
	int smpPerFrame;
	int scanLine;
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
	me->apu = Apu_Create(me->mem, 1024);

	me->insPerScanLine = (int)((float)freq / frameRate / (float)h);

	me->smpPerFrame = (double)APU_RATE / frameRate;

	double smpPerLine = (double)APU_RATE / frameRate / (double)h;
	me->smpPerLine = floor(smpPerLine);
	me->extraSmpNth = round(1.0 / (smpPerLine - (double)me->smpPerLine));

	LogD("extra: %d", me->extraSmpNth);

	if(me->extraSmpNth == 0)
		me->extraSmpNth = 1;

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
			int nSamples = me->smpPerLine + ((me->scanLine % me->extraSmpNth) == 0 ? 1 : 0);

			if(nSamples + me->smp < me->smpPerFrame){
				Apu_ProcessAudio(me->apu, nSamples);
				me->smp += nSamples;
			}

			me->scanLine++;

			if(me->vdpDone){
				// The VDP is also done. Frame complete.
				// Prepare for next;
				
				me->vdpDone = false;
				me->scanLine = 0;

				// Handle any samples that wern't processed during the frame
				// but should have been to keep audio sync

				nSamples = me->smpPerFrame - me->smp;
				if(nSamples > 0){
					Apu_ProcessAudio(me->apu, nSamples);
					me->smp += nSamples;
				}
				
				//LogD("%d / %d samples handled this frame", me->smp, me->smpPerFrame);
				me->smp = 0;
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
