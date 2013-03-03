#ifndef APU_H
#define APU_H

#include <stdint.h>

typedef struct Apu Apu;
typedef struct Mem Mem;

Apu* Apu_Create(Mem* mem, int bufferSize);
void Apu_ProcessAudio(Apu* me, int nSamples);

// stream is expected to be a native byte ordered, signed 16 bit, 48 KHz stream with stereo samples (4 bytes per sample, 2 per channel)
void Apu_FetchAudio(Apu* me, int16_t* stream, int samples);

#endif
