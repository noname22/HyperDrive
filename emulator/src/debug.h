#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include "cpu.h"

typedef struct Debug Debug;

Debug* Debug_Create(Cpu* cpu, Mem* mem);
void Debug_Destroy(Debug** debug);
void Debug_Inspector(Cpu* cpu, void* debug);
bool Debug_LoadSymbols(Debug* debug, const char* filename);

void Debug_AddBreakPointAddr(Debug* debug, uint32_t addr);
bool Debug_AddBreakPointLine(Debug* debug, const char* filename, int line);
bool Debug_AddBreakPointItem(Debug* debug, const char* item);

bool Debug_RemoveBreakPoint(Debug* debug, int index);
bool Debug_EnableBreakPoint(Debug* debug, int index, bool enabled);

#endif
