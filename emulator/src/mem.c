#include "mem.h"
#include "log.h"
#include "cvector.h"

// Memory Map
//   0x00000000 - 0x3FFFFFFF ROM, mirrored
//   0x40000000 - 0x7FFFFFFF RAM, mirrored 
//   0x80000000 ...          Hardware registers

#define MEM_MASK 0x3FFFFFFF

typedef struct {
	Mem_RegisterCb onRead;
	Mem_RegisterCb onWrite;
	void* data;
} Mem_Register;

struct Mem {
	uint8_t* rom;
	uint32_t romMask;
	uint32_t romSize;
	
	uint8_t* ram;
	uint32_t ramMask;
	uint32_t ramSize;

	uint8_t* regs;
	uint32_t regMask;
	uint32_t regSize;

	Mem_Register* regCb;
};

Mem* Mem_Create()
{
	Mem* me = calloc(1, sizeof(Mem));
	LAssert(me, "could not allocate ram for mem");

	me->romSize = 1024 * 1024 * 16;
	me->rom = calloc(1, me->romSize);
	LAssert(me->rom, "could not allocate ram for vm rom");
	me->romMask = me->romSize - 1;

	me->ramSize = 1024 * 1024 * 2;
	me->ram = calloc(1, me->ramSize);
	LAssert(me->ram, "could not allocate ram for vm ram");
	me->ramMask = me->ramSize - 1;
	
	me->regSize = 1024;
	me->regs = calloc(1, me->regSize);
	LAssert(me->regs, "could not allocate ram for vm hw regs");
	me->regMask = me->regSize - 1;

	me->regCb = calloc(1, me->regSize * sizeof(Mem_Register));
	LAssert(me->regCb, "could not allocate ram for vm hw reg callbacks");

	return me;
}

void Mem_Write8(Mem* me, uint32_t addr, uint8_t val)
{
	uint32_t idx;
	switch(addr >> 30){
		case 1: me->rom[addr & me->romMask] = val; break; // ROM
		case 2: me->ram[(addr & MEM_MASK) & me->ramMask] = val; break; // RAM
		default:
			// hw register
			idx = (addr & MEM_MASK) & me->regMask;
			me->regs[idx] = val; break;
			if(me->regCb[idx].onWrite) 
				me->regCb[idx].onWrite(me, addr, me->regs + idx, true, me->regCb[idx].data);
	}
}

void Mem_Write16(Mem* me, uint32_t addr, uint16_t val)
{
	// Write LSB last to trigger any register callback on the base address
	// when the rest of the value has been written
	Mem_Write8(me, addr + 1, val >> 8);
	Mem_Write8(me, addr, val & 0xff);
}

void Mem_Write32(Mem* me, uint32_t addr, uint32_t val)
{
	// Order as Write16
	Mem_Write8(me, addr + 3, (val >> 24) & 0xff);
	Mem_Write8(me, addr + 2, (val >> 16) & 0xff);
	Mem_Write8(me, addr + 1, (val >> 8) & 0xff);
	Mem_Write8(me, addr, val & 0xff);
}

uint8_t Mem_Read8(Mem* me, uint32_t addr)
{
	uint32_t idx;
	switch(addr >> 30){
		case 1: return me->rom[addr & me->romMask]; // ROM
		case 2: return me->ram[(addr & MEM_MASK) & me->ramMask]; // RAM
		default:
			// hw register
			idx = (addr & MEM_MASK) & me->regMask;
			if(me->regCb[idx].onRead) 
				me->regCb[idx].onRead(me, addr, me->regs + idx, false, me->regCb[idx].data);
			return me->regs[idx];
	}
}

uint16_t Mem_Read16(Mem* me, uint32_t addr)
{
	return Mem_Read8(me, addr) | (Mem_Read8(me, addr) << 8);
}

uint32_t Mem_Read32(Mem* me, uint32_t addr)
{
	return Mem_Read8(me, addr) | (Mem_Read8(me, addr) << 8) | (Mem_Read8(me, addr) << 16) | (Mem_Read8(me, addr) << 24);
}
