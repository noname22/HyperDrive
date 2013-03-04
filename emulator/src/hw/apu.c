#include "log.h"
#include "apu.h"

struct Apu {
	int nBufferSamples;
	int16_t* buffer;
};

Apu* Apu_Create(Mem* mem, int nBufferSamples)
{
	Apu* me = calloc(1, sizeof(Apu));
	LAssert(me, "could not allocate ram for APU");

	me->nBufferSamples = nBufferSamples;
	me->buffer = calloc(1, sizeof(int16_t) * nBufferSamples);
	LAssert(me->buffer, "could not allocate ram for APU buffer");
	
	return me;
}

void Apu_ProcessAudio(Apu* me, int nSamples)
{
}

void Apu_FetchAudio(Apu* me, int16_t* stream, int samples)
{
}
