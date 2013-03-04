#include "log.h"
#include "apu.h"
#include <math.h>

struct Apu {
	Mem* mem;
};

Apu* Apu_Create(Mem* mem)
{
	Apu* me = calloc(1, sizeof(Apu));
	LAssert(me, "could not allocate ram for APU");

	me->mem = mem;
	
	return me;
}

void Apu_FetchAudio(Apu* me, int16_t* stream, int nSamples)
{
	static float t;
	for(unsigned i = 0; i < nSamples; i++){
		t += .1;
		stream[i * 2] = sinf(t) * 8192.0f;
		stream[i * 2 + 1] = sinf(t) * 8192.0f;
	}
}
