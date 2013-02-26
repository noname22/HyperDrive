#ifndef DCPU_H
#define DCPU_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Cpu Cpu;
typedef struct Mem Mem;

Cpu* Cpu_Create(Mem* mem);
void Cpu_Destroy(Cpu** me);
int Cpu_Execute(Cpu* me, int cycles);

void Cpu_SetSysCall(Cpu* me, void (*sc)(Cpu* me, void* data), int id, void* data);

uint32_t Cpu_Pop(Cpu* me);
uint32_t Cpu_peek(Cpu* me);
void Cpu_Push(Cpu* me, uint32_t v);
void Cpu_DumpState(Cpu* me);

typedef enum { DR_A, DR_B, DR_C, DR_X, DR_Y, DR_Z, DR_I, DR_J, DR_SP, DR_PC, DR_O } Cpu_Register;
typedef enum { CI_Hblank, CI_Vblank } Cpu_Irq;

uint32_t Cpu_GetRegister(Cpu* me, Cpu_Register reg);
void Cpu_SetRegister(Cpu* me, Cpu_Register reg, uint32_t val);

void Cpu_SetExit(Cpu* me, bool e);
bool Cpu_GetExit(Cpu* me);
void Cpu_Interrupt(Cpu* me, int num);
bool Cpu_GetPerformIns(Cpu* me);

/* Something to be called before executing each instruction */
void Cpu_SetInspector(Cpu* me, void (*ins)(Cpu* cpu, void* data), void* data);

#endif
