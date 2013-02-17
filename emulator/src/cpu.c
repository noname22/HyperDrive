#include "common.h"
#include "cpu.h"

typedef void (*InsPtr)(Cpu* me, uint32_t* v1, uint32_t* v2);
typedef void (*SysCallPtr)(Cpu* me, void* data);

typedef struct {
	SysCallPtr fun;
	int id;
	void* data;
} SysCall;

static const char* dinsNames[] = DINSNAMES;

typedef Vector(SysCall) SysCallVector;

struct Cpu {
	uint8_t* ram;
	uint32_t memSize;

	uint32_t regs[8];
	uint32_t sp, pc, o;

	bool performNextIns;
	bool exit;

	int cycles;

	SysCallVector sysCalls;
	void (*inspector)(Cpu* dcpu, void* data);
	void* inspectorData;

	InsPtr ins[DINS_NUM];
};

// Cast to uint16_t
#define U16C(__w) ((uint16_t)(__w))

void Cpu_SetExit(Cpu* me, bool e)
{
	me->exit = e;
}

bool Cpu_GetExit(Cpu* me)
{
	return me->exit;
}

uint32_t Cpu_Pop(Cpu* me) { 
	uint32_t ret = *((uint32_t*)(me->ram + me->sp));
	me->sp += 4;
	return ret;
}

void Cpu_Push(Cpu* me, uint32_t v){
	LogD("pushing: %u", v);
	me->sp -= 4;
	*((uint32_t*)(me->ram + me->sp)) = v;
}

void Cpu_SetInspector(Cpu* me, void (*ins)(Cpu* dcpu, void* data), void* data)
{
	me->inspector = ins;
	me->inspectorData = data;
}

// Extended instructions
void NonBasic(Cpu* me, uint32_t* v1, uint32_t* v2)
{
	if(*v1 == DI_ExtJsr - DINS_EXT_BASE){
		LogD("EXT_JSR");
		Cpu_Push(me, me->pc);
		me->pc = *v2;
		me->cycles += 2;
	}

	else if(*v1 == DI_ExtSys - DINS_EXT_BASE){
		me->cycles += 1;

		if(*v2 == 0){
			me->exit = true;
			return;
		}

		SysCall* s;
		bool found = false;
		Vector_ForEach(me->sysCalls, s){
			if(s->id == *v2){
				s->fun(me, s->data);
				found = true;
				break;
			}
		}
		if(!found) LogW("Invalid syscall: %d", *v2);
	}

	else
		me->cycles += 1;
}

// Basic instructions
void Set(Cpu* me, uint32_t* v1, uint32_t* v2){ *v1 = *v2; me->cycles++; }

void Add(Cpu* me, uint32_t* v1, uint32_t* v2){
	uint16_t tmp = *v1;
	*v1 += *v2;
	me->o = *v1 < tmp;
	me->cycles += 2;
}

void Sub(Cpu* me, uint32_t* v1, uint32_t* v2)
{
	uint16_t tmp = *v1;
	*v1 -= *v2;
	me->o = *v1 > tmp;
	me->cycles += 2;
}

void Mul(Cpu* me, uint32_t* v1, uint32_t* v2)
{
	me->o = ((uint64_t)*v1 * (uint64_t)*v2 >> 32) & 0xffffffff;
	*v1 *= *v2;
	me->cycles += 2;
}

void Div(Cpu* me, uint32_t* v1, uint32_t* v2)
{
	if(*v2 == 0){ me->o = *v1 = 0; return; }
	me->o = (((uint64_t)*v1 << 32) / ((uint64_t)*v2)) & 0xffffffff;

	// do this twice because v2 can be o
	if(*v2 == 0){ me->o = *v1 = 0; return; }

	*v1 /= *v2;
	me->cycles += 3;
}

void Mod(Cpu* me, uint32_t* v1, uint32_t* v2)
{
	if(*v2 == 0){ me->o = *v1 = 0; return; }
	*v1 %= *v2;
	me->cycles += 3;
}

void Shl(Cpu* me, uint32_t* v1, uint32_t* v2)
{
	me->o = (((uint64_t)*v1 << (uint64_t)*v2) >> 32) & 0xffffffff;
	*v1 = *v1 << *v2;
	me->cycles += 2;
}

void Shr(Cpu* me, uint32_t* v1, uint32_t* v2)
{
	me->o = (((uint64_t)*v1 << 32)>> (uint64_t)*v2) & 0xffffffff;
	me->cycles += 2;
}

void And(Cpu* me, uint32_t* v1, uint32_t* v2){ *v1 &= *v2; me->cycles++; }
void Bor(Cpu* me, uint32_t* v1, uint32_t* v2){ *v1 |= *v2; me->cycles++; }
void Xor(Cpu* me, uint32_t* v1, uint32_t* v2){ *v1 ^= *v2; me->cycles++; }

void Ife(Cpu* me, uint32_t* v1, uint32_t* v2){ me->performNextIns = *v1 == *v2; me->cycles += 2 + (uint32_t)me->performNextIns; }
void Ifn(Cpu* me, uint32_t* v1, uint32_t* v2){ me->performNextIns = *v1 != *v2; me->cycles += 2 + (uint32_t)me->performNextIns; }
void Ifg(Cpu* me, uint32_t* v1, uint32_t* v2){ me->performNextIns = *v1 > *v2; me->cycles += 2 + (uint32_t)me->performNextIns; }
void Ifb(Cpu* me, uint32_t* v1, uint32_t* v2){ me->performNextIns = (*v1 & *v2) != 0; me->cycles += 2 + (uint32_t)me->performNextIns; }

Cpu* Cpu_Create(uint8_t* ram, uint32_t ramSize)
{
	Cpu* me = calloc(1, sizeof(Cpu));
	//me->ram = calloc(1, sizeof(uint16_t) * 0x10000);
	me->ram = ram;
	me->sp = ramSize; // will be decreased before it's written to
	me->memSize = ramSize;

	me->performNextIns = true;

	Vector_Init(me->sysCalls, SysCall);

	me->ins[DI_NonBasic] = NonBasic;
	me->ins[DI_Set] = Set;
	me->ins[DI_Add] = Add;
	me->ins[DI_Sub] = Sub;
	me->ins[DI_Mul] = Mul;
	me->ins[DI_Div] = Div;
	me->ins[DI_Mod] = Mod;
	me->ins[DI_Shl] = Shl;
	me->ins[DI_Shr] = Shr;
	me->ins[DI_And] = And;
	me->ins[DI_Bor] = Bor;
	me->ins[DI_Xor] = Xor;
	me->ins[DI_Ife] = Ife;
	me->ins[DI_Ifn] = Ifn;
	me->ins[DI_Ifg] = Ifg;
	me->ins[DI_Ifb] = Ifb;

	return me;
}

void Cpu_Destroy(Cpu** me)
{
	Vector_Free((*me)->sysCalls);

	free((*me)->ram);
	free(*me);
	*me = NULL;
}

void Cpu_SetSysCall(Cpu* me, void (*sc)(Cpu* me, void* data), int id, void* data)
{
	SysCall s = {sc, id, data};
	Vector_Add(me->sysCalls, s);
}

uint8_t* Cpu_GetRam(Cpu* me)
{
	return me->ram;
}

uint32_t Cpu_GetRegister(Cpu* me, Cpu_Register reg)
{
	if(reg <= DR_J) return me->regs[reg];
	if(reg == DR_O) return me->o;
	if(reg == DR_SP) return me->sp;
	return me->pc;
}

void Cpu_SetRegister(Cpu* me, Cpu_Register reg, uint32_t val)
{
	if(reg <= DR_J) me->regs[reg] = val;
	else if(reg == DR_O) me->o = val;
	else if(reg == DR_SP) me->sp = val;
	else me->pc = val;
}

void Cpu_DumpState(Cpu* me)
{
	LogD(" == Current State ==");
	LogD("Regs: A:0x%08x B:0x%08x C:0x%08x X:0x%08x Y:0x%08x Z:0x%8x I:0x%08x J:0x%08x", 
		me->regs[0], me->regs[1], me->regs[2], me->regs[3], me->regs[4], me->regs[5], me->regs[6], me->regs[7]);

	LogD("      SP:0x%08x PC:0x%08x O:0x%08x", me->sp, me->pc, me->o);
	
	LogD(" ");
	LogD("Stack: ");

	if(me->sp){
		for(int i = me->memSize - 1; i >= me->sp; i -= 4){
			LogD("  0x%08x: 0x%08x", i, *((uint32_t*)(me->ram + i)));
		}
	}else LogD("  (empty)");
	LogD(" ");
}

uint8_t Cpu_ReadRam8(Cpu* me)
{
	return me->ram[me->pc++];
}

uint16_t Cpu_ReadRam16(Cpu* me)
{
	uint16_t ret = me->ram[me->pc++];
	ret |= me->ram[me->pc++] << 8;
	return ret;
}

uint32_t Cpu_ReadRam32(Cpu* me)
{
	uint32_t ret = me->ram[me->pc++];
	ret |= me->ram[me->pc++] << 8;
	ret |= me->ram[me->pc++] << 16;
	ret |= me->ram[me->pc++] << 24;
	return ret;
}

int Cpu_Execute(Cpu* me, int execCycles)
{
	me->cycles = 0;

	while(me->cycles < execCycles){
		if(me->inspector) me->inspector(me, me->inspectorData);

		uint32_t addr = me->pc;
		bool hasNextWord[2];
		uint32_t val[2];
		uint16_t pIns = Cpu_ReadRam16(me);
		DVals v[2];

		uint32_t* pv[2];

		DIns ins = pIns & 0xf;

		v[0] = (pIns >> 4) & 0x3f;
		v[1] = (pIns >> 10) & 0x3f;

		for(int i = 0; i < 2; i++){
			DVals vv = v[i];
			
			// Extended instruction, first operand is the instruction number
			if(i == 0 && ins == DI_NonBasic){
				val[0] = v[0];
				pv[i] = val;
				continue;
			}

			if((hasNextWord[i] = OpHasNextWord(v[i]))) {
				val[i] = Cpu_ReadRam32(me);
				me->cycles++;
			}

			switch(vv){
				case DV_Pop:          pv[i] = (uint32_t*)(me->ram + me->sp); me->sp += 4; break;
				case DV_Peek:         pv[i] = (uint32_t*)me->ram + me->sp; break;
				case DV_Push:         me->sp -= 4; pv[i] = (uint32_t*)(me->ram + me->sp); break;
				case DV_SP:           pv[i] = &me->sp; break;
				case DV_PC:           pv[i] = &me->pc; break;
				case DV_O:            pv[i] = &me->o; break;
				case DV_RefNextWord:  pv[i] = (uint32_t*)me->ram + val[i]; break;
				case DV_NextWord:     pv[i] = val + i; break;
				default:
					// register
					if(vv >= DV_A && vv <= DV_J)
						pv[i] = me->regs + vv - DV_A;

					// register reference
					else if(vv >= DV_RefBase && vv <= DV_RefTop)
						pv[i] = (uint32_t*)me->ram + me->regs[vv - DV_RefBase];

					// nextword + register reference
					else if(vv >= DV_RefRegNextWordBase && vv <= DV_RefRegNextWordTop)
						pv[i] = (uint32_t*)me->ram + U16C(val[i] + me->regs[vv - DV_RefRegNextWordBase]);

					// literal
					else if(vv >= DV_LiteralBase){
						val[i] = vv - DV_LiteralBase;
						pv[i] = val + i;
					}

					break;
			}

		}
		
		if(me->performNextIns){ 
			LogD("%08x: %s", addr, dinsNames[ins]);
			me->ins[ins](me, pv[0], pv[1]);
		}

		else me->performNextIns = true;

		Cpu_DumpState(me);

		if(me->exit) return 0;
	}

	return 1;
}
