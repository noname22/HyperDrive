#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stdbool.h>

typedef struct Mem Mem;

typedef void (*Mem_RegisterCb)(Mem* me, uint32_t addr, uint8_t* reg, bool write, void* data);

Mem* Mem_Create();

void Mem_Write8(Mem* me, uint32_t addr, uint8_t val);
void Mem_Write16(Mem* me, uint32_t addr, uint16_t val);
void Mem_Write32(Mem* me, uint32_t addr, uint32_t val);

uint8_t Mem_Read8(Mem* me, uint32_t addr);
uint16_t Mem_Read16(Mem* me, uint32_t addr);
uint32_t Mem_Read32(Mem* me, uint32_t addr);

#endif
