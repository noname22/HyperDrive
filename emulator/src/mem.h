#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Mem Mem;

typedef void (*Mem_RegisterCb)(Mem* me, uint32_t addr, uint8_t* reg, bool write, void* data);

Mem* Mem_Create();

uint8_t* Mem_GetPtr(Mem* me, uint32_t addr, uint32_t* max);

void Mem_Write8(Mem* me, uint32_t addr, uint8_t val);
void Mem_Write16(Mem* me, uint32_t addr, uint16_t val);
void Mem_Write32(Mem* me, uint32_t addr, uint32_t val);

uint8_t Mem_Read8(Mem* me, uint32_t addr);
uint16_t Mem_Read16(Mem* me, uint32_t addr);
uint32_t Mem_Read32(Mem* me, uint32_t addr);

uint32_t Mem_GetBOS(Mem* me);
bool Mem_SetROM(Mem* me, uint8_t* rom, uint32_t size);

#define MEM_ROM_BASE 0
#define MEM_RAM_BASE 0x40000000 
#define MEM_REG_BASE 0x80000000

// Interrupt vector table
#define MEM_IVT_BASE MEM_REG_BASE
#define MEM_VDP_BASE (MEM_REG_BASE + 0x20)


#endif
