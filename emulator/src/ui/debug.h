#ifndef DEBUG_H
#define DEBUG_H

#include <stdint.h>
#include "cpu.h"

typedef struct Debug Debug;

Debug* Debug_Create(Cpu* cpu, Mem* mem);
void Debug_Destroy(Debug** me);
bool Debug_Inspector(Cpu* cpu, void* me);
bool Debug_LoadSymbols(Debug* me, const char* filename);

void Debug_AddBreakPointAddr(Debug* me, uint32_t addr);
bool Debug_AddBreakPointLine(Debug* me, const char* filename, int line);
bool Debug_AddBreakPointItem(Debug* me, const char* item);

bool Debug_RemoveBreakPoint(Debug* me, int index);
bool Debug_EnableBreakPoint(Debug* me, int index, bool enabled);

void Debug_Break(Debug* me);
void Debug_Continue(Debug* me);

void Debug_SetPrintfHandler(Debug* me, void (*fun)(Debug* me, void* data, const char* format, ...), void* data);
void Debug_HandleInput(Debug* me, const char* text);

bool Debug_GetRunning(Debug* me);

#endif
