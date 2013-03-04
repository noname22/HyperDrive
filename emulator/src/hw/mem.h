#ifndef MEM_H
#define MEM_H

#include <stdint.h>
#include <stdbool.h>

#define MEM_SIZE (64 * 1024 * 1024)
#define MEM_ROM_MASK 0x1FFFFFF
#define MEM_RAM_MASK 0x2000000
#define MEM_MASK (MEM_SIZE - 1)

#define MEM_BOS      0x3000000

#define MEM_ROM_BASE 0
#define MEM_RAM_BASE 0x2000000 
#define MEM_REG_BASE 0x3000000

// Interrupt vector table
#define MEM_IVT_BASE MEM_REG_BASE
#define MEM_VDP_BASE (MEM_REG_BASE + 0x100)

#define MEM_INPUT_BASE 0x3004100

typedef struct Mem Mem;
struct Mem {
	uint8_t* mem;
};

Mem* Mem_Create();

#define MEM_READ_PTR(_mem, _addr) (((_mem)->mem) + ((_addr) & MEM_MASK))
#define MEM_WRITE_PTR(_mem, _addr) (((_mem)->mem) + (((_addr) | MEM_RAM_MASK) & MEM_MASK))

// XXX fix big endian systems
#define MEM_TOLE32(__v) __v
#define MEM_TOLE16(__v) __v
#define MEM_FROMLE32(__v) __v
#define MEM_FROMLE16(__v) __v

#define MEM_READ_UNSAFE32(_mem, _addr) (MEM_FROMLE32(*(uint32_t*)&(_mem)->mem[_addr]))
#define MEM_READ_UNSAFE16(_mem, _addr) (MEM_FROMLE16(*(uint16_t*)&(_mem)->mem[_addr]))
#define MEM_READ_UNSAFE8(_mem, _addr) ((_mem)->mem[_addr])

#define MEM_READ32(_mem, _addr) (MEM_FROMLE32(*(uint32_t*)MEM_READ_PTR((_mem), (_addr))))
#define MEM_READ16(_mem, _addr) (MEM_FROMLE16(*(uint16_t*)MEM_READ_PTR((_mem), (_addr))))
//#define MEM_READ8(_mem, _addr)  (*MEM_READ_PTR(_mem, (_addr)))
#define MEM_READ8(_mem, _addr)  ((_mem)->mem[(_addr) & MEM_MASK])

#define MEM_WRITE8(_mem, _addr, _v) (*(MEM_WRITE_PTR((_mem), (_addr))) = _v)
#define MEM_WRITE16(_mem, _addr, _v) (*((uint16_t*)MEM_WRITE_PTR((_mem), (_addr))) = MEM_TOLE16(_v))
#define MEM_WRITE32(_mem, _addr, _v) (*(uint32_t*)(MEM_WRITE_PTR((_mem), (_addr))) = MEM_TOLE32(_v))

bool Mem_SetROM(Mem* me, uint8_t* rom, uint32_t size);

#endif
