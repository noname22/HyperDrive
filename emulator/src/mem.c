#include "mem.h"
#include "log.h"
#include "cvector.h"

// XXX: THIS IS WRONG
// Memory Map
//   0x00000000 - 0x3FFFFFFF ROM, mirrored
//   0x40000000 - 0x7FFFFFFF RAM, mirrored 
//   0x80000000 ...          Hardware registers
//        +0x00 - 0x1F       Interrupt vectors
//    ---------------- VDP ------------------
//        +0x20 - 0x40       8 tile layers
// XXX: THIS IS WRONG

Mem* Mem_Create()
{
	Mem* me = calloc(1, sizeof(Mem));
	LAssert(me, "could not allocate ram for mem");

	me->mem = calloc(1, MEM_SIZE + 3);
	// + 3 because if a 32 bit word is written to the last address it should be a valid write
	LAssert(me->mem, "could not allocate ram for vm memory");

	return me;
}

bool Mem_SetROM(Mem* me, uint8_t* rom, uint32_t size)
{
	if(size > MEM_RAM_BASE)
		return false;

	memset(me->mem, 0, MEM_SIZE + 3);
	memcpy(me->mem, rom, size);
	return true;
}
