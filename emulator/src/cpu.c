//#define LDEBUG

#include "common.h"
#include "cpu.h"
#include "mem.h"

typedef void (*InsPtr)(Cpu* me, uint32_t* v1, uint32_t* v2);
typedef void (*SysCallPtr)(Cpu* me, void* data);

typedef struct {
	SysCallPtr fun;
	int id;
	void* data;
} SysCall;

#ifdef LDEBUG
static const char* dinsNames[] = DINSNAMES;
static const char* dvalNames[] = VALNAMES;
#endif

typedef Vector(SysCall) SysCallVector;

struct Cpu {
	Mem* mem;

	uint32_t regs[8];
	uint32_t sp, pc, o;

	bool performNextIns;
	bool exit;
	bool wait;

	int cycles;

	SysCallVector sysCalls;
	void (*inspector)(Cpu* dcpu, void* data);
	void* inspectorData;
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
	uint32_t ret = MEM_READ32(me->mem, me->sp);
	me->sp += 4;
	return ret;
}

void Cpu_Push(Cpu* me, uint32_t v){
	LogD("pushing: %u", v);
	me->sp -= 4;
	MEM_WRITE32(me->mem, me->sp, v);
}

void Cpu_SetInspector(Cpu* me, void (*ins)(Cpu* cpu, void* data), void* data)
{
	me->inspector = ins;
	me->inspectorData = data;
}

bool Cpu_GetPerformIns(Cpu* me)
{
	return me->performNextIns;
}

Cpu* Cpu_Create(Mem* mem)
{
	Cpu* me = calloc(1, sizeof(Cpu));
	me->mem = mem;
	me->sp = MEM_BOS;
	me->performNextIns = true;

	Vector_Init(me->sysCalls, SysCall);

	return me;
}

void Cpu_Destroy(Cpu** me)
{
	Vector_Free((*me)->sysCalls);
	free(*me);
	*me = NULL;
}

void Cpu_SetSysCall(Cpu* me, void (*sc)(Cpu* me, void* data), int id, void* data)
{
	SysCall s = {sc, id, data};
	Vector_Add(me->sysCalls, s);
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

void Cpu_Interrupt(Cpu* me, int num)
{
	uint32_t vec = MEM_READ32(me->mem, MEM_IVT_BASE + num * 4);
	if(vec == 0) return;

	Cpu_Push(me, me->pc);
	me->pc = vec;
	me->wait = false;
}

void Cpu_DumpState(Cpu* me)
{
	LogD(" == Current State ==");
	LogD("Regs: A:0x%08x B:0x%08x C:0x%08x X:0x%08x Y:0x%08x Z:0x%8x I:0x%08x J:0x%08x", 
		me->regs[0], me->regs[1], me->regs[2], me->regs[3], me->regs[4], me->regs[5], me->regs[6], me->regs[7]);

	LogD("      SP:0x%08x PC:0x%08x O:0x%08x", me->sp, me->pc, me->o);
	
	LogD(" ");
	LogD("Stack: ");

	if(me->sp == MEM_BOS){
		for(int i = MEM_BOS - 1; i >= me->sp; i -= 4){
			LogD("  0x%08x: 0x%08x", i, MEM_READ32(me->mem, i));
		}
	}else LogD("  (empty)");
	LogD(" ");
}


int Cpu_Execute(Cpu* me, int execCycles)
{
	me->cycles = 0;
	if(me->wait) return 0;

	#define READ8 MEM_READ8(me->mem, me->pc); me->pc ++;
	#define READ16 MEM_READ16(me->mem, me->pc); me->pc += 2;
	#define READ32 MEM_READ32(me->mem, me->pc); me->pc += 4;

	while(me->cycles < execCycles && !me->wait){
		if(me->inspector) me->inspector(me, me->inspectorData);

		#ifdef LDEBUG
		uint32_t addr = me->pc;
		#endif

		uint32_t val[2];
		DVals v[2];

		uint32_t pv[2];
		uint32_t* reg[2] = {NULL, NULL};

		DIns ins = READ8;
		
		if(ins >= DINS_NUM){
			LogW("illegal instruction: 0x%02x", ins);
			continue;
		}

		LogD("%08x: %s", addr, dinsNames[ins]);

		int numOps = InsNumOps(ins);

		for(int i = 0; i < numOps; i++){
			v[i] = READ8;
			if(v[i] >= DVALS_NUM){
				LogW("illegal operand: 0x%02x", v[i]);
				continue;
			}

			LogD("  op %d: %s", i + 1, dvalNames[v[i]]);
			
			if((OpHasNextWord(v[i]))){
				val[i] = READ32;
				LogD("  NW: %08x", val[i]);
			}

			switch(v[i]){
				case DV_Pop:          pv[i] = me->sp; me->sp += 4; break;
				case DV_Peek:         pv[i] = me->sp; break;
				case DV_Push:         me->sp -= 4; pv[i] = me->sp; break;
				case DV_SP:           reg[i] = &me->sp; break;
				case DV_PC:           reg[i] = &me->pc; break;
				case DV_O:            reg[i] = &me->o; break;
				case DV_RefNextWord:  pv[i] = val[i]; break;
				case DV_NextWord:     pv[i] = me->pc - 4; break;
				default:
					// register
					if(v[i] >= DV_A && v[i] <= DV_J)
						reg[i] = me->regs + v[i] - DV_A;

					// register reference
					else if(v[i] >= DV_RefBase && v[i] <= DV_RefTop)
						pv[i] = me->regs[v[i] - DV_RefBase];

					// nextword + register reference
					else if(v[i] >= DV_RefRegNextWordBase && v[i] <= DV_RefRegNextWordTop)
						pv[i] = (uint32_t)(val[i] + me->regs[v[i] - DV_RefRegNextWordBase]);

					break;
			}
		}

		// Sets or gets the data the operand is referring to. Either a register or a memory address.
		// _o is the zero indexed operand number

		#define OP_GET(_o) (reg[_o] ? *reg[_o] : MEM_READ32(me->mem, pv[_o]))
		#define OP_SET(_o, _v) (reg[_o] ? *reg[_o] = (_v): MEM_WRITE32(me->mem, pv[_o], (_v)))
		
		if(me->performNextIns){ 
			uint32_t tmp, op[2] = {0};
			for(int i = 0; i < numOps; i++){
				op[i] = OP_GET(i);
			}

			switch(ins){
				case DI_Set: 
					OP_SET(0, op[1]);
					me->cycles++; 
					break;

				case DI_Add: 
					tmp = op[0];
					op[0] += op[1];
					OP_SET(0, op[0]);
					
					me->o = op[0] < tmp;
					me->cycles += 2;
					break;

				case DI_Sub:
					tmp = op[0] = OP_GET(0);
					OP_SET(0, op[0] -= OP_GET(1));
					me->o = op[0] > tmp;
					me->cycles += 2;
					break;

				case DI_Mul:
					me->o = ((uint64_t)op[0] * (uint64_t)op[1] >> 32) & 0xffffffff;
					op[0] *= op[0];
					OP_SET(0, op[0]);
					me->cycles += 2;
					break;

				case DI_Div:
					if(op[1] == 0){
						me->o = 0;
						OP_SET(0, 0);
						break;
					}

					me->o = (((uint64_t)op[0] << 32) / ((uint64_t)op[1])) & 0xffffffff;

					// do this twice because op[1] can be 0
					if(op[1] == 0){
						me->o = 0;
						OP_SET(0, 0);
						break;
					}

					op[0] /= op[1];
					OP_SET(0, op[0]);
					
					me->cycles += 3;
					break;

				case DI_Mod:
					if(op[1] == 0){
						me->o = 0;
						OP_SET(0, 0);
						break;
					}

					op[0] %= op[1];
					OP_SET(0, op[0]);
					me->cycles += 3;
					break;

				case DI_Shl:
					me->o = (((uint64_t)op[0] << (uint64_t)op[1]) >> 32) & 0xffffffff;
					OP_SET(0, op[0] << op[1]);
					me->cycles += 2;
					break;
				
				case DI_Shr:
					me->o = (((uint64_t)op[0] << 32)>> (uint64_t)op[1]) & 0xffffffff;
					OP_SET(0, op[0] >> op[1]);
					me->cycles += 2;
					break;

				case DI_And:
					OP_SET(0, op[0] & op[1]);
					me->cycles++;
					break;

				case DI_Bor:
					OP_SET(0, op[0] | op[1]);
					me->cycles++;
					break;

				case DI_Xor:
					OP_SET(0, op[0] ^ op[1]);
					me->cycles++;
					break;
				
				case DI_Ife:
					me->performNextIns = op[0] == op[1];
					me->cycles += 2 + (uint32_t)me->performNextIns;
					break;

				case DI_Ifn:
					me->performNextIns = op[0] != op[1];
					me->cycles += 2 + (uint32_t)me->performNextIns;
					break;

				case DI_Ifg:
					me->performNextIns = op[0] > op[1];
					me->cycles += 2 + (uint32_t)me->performNextIns;
					//LogD("IFG %x %x -> perform: %d", op[0], op[1], me->performNextIns);
					break;

				case DI_Ifb:
					me->performNextIns = (op[0] & op[1]) != 0;
					me->cycles += 2 + (uint32_t)me->performNextIns;
					break;

				case DI_Sys:
					do{
						LogD("SYSCALL");
						me->cycles += 1;

						if(op[0] == 0){
							LogD("wait");
							me->wait = true;
							break;
						}

						SysCall* s;
						bool found = false;
						Vector_ForEach(me->sysCalls, s){
							if(s->id == op[0]){
								s->fun(me, s->data);
								found = true;
								break;
							}
						}
						if(!found) LogW("Invalid syscall: %d", op[0]);

					}while(0);
					break;

				case DI_Jsr:
					Cpu_Push(me, me->pc);
					me->pc = op[0];
					me->cycles += 2;
					break;
			}
		}

		else me->performNextIns = true;

		/*char bb[1024];
		gets(bb);
		Cpu_DumpState(me);*/

		if(me->exit) return 0;
	}

	return 1;
}
