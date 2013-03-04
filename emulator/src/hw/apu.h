#ifndef APU_H
#define APU_H

#include <stdint.h>

#include "htypes.h"

#define APU_RATE 48000

Apu* Apu_Create(Mem* mem);

// stream is expected to be a native byte ordered, signed 16 bit, 48 KHz stream with stereo samples (4 bytes per sample, 2 per channel)
void Apu_FetchAudio(Apu* me, int16_t* stream, int nSamples);

#endif
