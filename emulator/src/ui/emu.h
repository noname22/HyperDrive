#ifndef EMU_H
#define EMU_H

typedef struct {
	int freq;
	const char* file;
	const char* debugFile;
	const char* videoFile;
	int numRecFrames;
} Settings;

typedef struct Emu Emu;

Emu* Emu_Create(Settings* settings);
int Emu_Run(Emu* me);

#endif
