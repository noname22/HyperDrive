#define LDEBUG
#include "log.h"
#include "apu.h"
#include "mem.h"
#include <math.h>

#include <stdbool.h>
#include <string.h>

typedef enum { AF_NBits = 1, AF_Loop = 2, AF_LoopMode = 4 } AF_Flags;

typedef struct {
	float logVol;
	uint8_t vol;
	bool ppBackwards;
} AChannel;

struct Apu {
	Mem* mem;
	int nChannels;
	AChannel channels[32];
};

Apu* Apu_Create(Mem* mem)
{
	Apu* me = calloc(1, sizeof(Apu));
	LAssert(me, "could not allocate ram for APU");

	me->mem = mem;
	me->nChannels = 32;
	
	return me;
}

void Apu_HandleChannel(Apu* me, int chnNum, int16_t* stream, int nSamples)
{
	// sample addr
	// position
	// flags
	//	0 -> 8/16 bit
	//      1 -> loop disabled/enabled
	//      2 -> loop repeat/ping-pong

	// loop start
	// loop end/sample end
	// sample frequency
	
	// volume
	// panning

	AChannel* chn = me->channels + chnNum;
	uint32_t addr = (MEM_APU_BASE + chnNum * 64);

	uint32_t smpPtr = MEM_READ32(me->mem, addr); addr += 4;
	if(smpPtr == 0) return;

	float pos = MEM_READ32(me->mem, addr); addr += 4;
	uint32_t flags = MEM_READ32(me->mem, addr); addr += 4;

	bool n16b = flags & AF_NBits;
	bool loop = flags & AF_Loop;
	bool loopPingPong = flags & AF_LoopMode;

	int loopStart = MEM_READ32(me->mem, addr); addr += 4;
	int loopEnd = MEM_READ32(me->mem, addr); addr += 4;
	
	uint32_t freq = MEM_READ32(me->mem, addr); addr += 4;

	float tt = (float)freq / (float)APU_RATE;
	float t = chn->ppBackwards ? -tt : tt;

	uint8_t vol = MEM_READ8(me->mem, addr); addr += 1;
	if(vol != chn->vol){
		chn->vol = vol;
		chn->logVol = log10f((float)vol / 255.0f * 9 + 1);
		LogD("logVol %f", chn->logVol);
		LogD("vol %d", vol);
	}

	float pan = (float)(MEM_READ8(me->mem, addr)) / 255.0f; addr += 1;
	LogD("loop start: %d", loopStart);
	LogD("loop end: %d", loopEnd);

	for(unsigned i = 0; i < nSamples; i++){
		if(pos >= loopEnd){
			if(loop){
				if(loopPingPong){
					chn->ppBackwards = true;
					t = -tt;
				}else{
					pos = loopStart;
				}
			}
			else break;
		}

		if(pos <= loopStart){
			t = tt;
			chn->ppBackwards = false;
		}

		float smp;
		if(n16b)  smp = (int16_t)MEM_READ16(me->mem, (smpPtr + ((uint32_t)pos * 2)));
		else      smp = (float)((int8_t)MEM_READ8(me->mem, (smpPtr + (uint32_t)pos)) * 256.0f);

		stream[i * 2] += smp * chn->logVol * pan;
		stream[i * 2 + 1] += smp * chn->logVol * (1.0 - pan);

		pos += t;
	}
	
	LogD("%f", t);
	LogD("%f", pos);

	MEM_WRITE32(me->mem, MEM_APU_BASE + chnNum * 64 + 4, (uint32_t)pos);
}

void Apu_FetchAudio(Apu* me, int16_t* stream, int nSamples)
{
	memset(stream, 0, nSamples * 4);
	for(int i = 0; i < me->nChannels; i++)
		Apu_HandleChannel(me, i, stream, nSamples);
}
