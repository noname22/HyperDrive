#ifndef CVECTOR_H
#define CVECTOR_H

#include <string.h>
#include <stdlib.h>

#define VMIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#define VMAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))

#define Vector(type) \
	struct type ## _vec_s\
	{\
		int count;\
		type* elems;\
		int pAllocCount;\
		int pElemSize;\
	}

#define Vector_Init(vec, type) Vector_InitEx(vec, type, 16);
#define Vector_InitEx(vec, type, _count) \
	do{\
		(vec).count = 0;\
		(vec).pElemSize = sizeof(type);\
		(vec).elems = malloc((vec).pElemSize * (_count));\
		(vec).pAllocCount = (_count);\
	}while(0);

#define Vector_Add(vec, elem) \
	do{\
		if((vec).pAllocCount <= (vec).count + 1){ \
			void* __tmpbuffer = malloc((vec).pElemSize * (vec).pAllocCount * 2); \
			memcpy(__tmpbuffer, (vec).elems, (vec).pAllocCount * (vec).pElemSize);\
			(vec).pAllocCount *= 2;\
			free((vec).elems);\
			(vec).elems = __tmpbuffer;\
		}\
		(vec).elems[(vec).count++] = (elem);\
	}while(0);

#define Vector_Remove(vec, _at, _n) \
	do{\
		if((_at) < 0 || (_n) < 0) break;\
		if((_at) >= (vec).count) break;\
		if((_n) <= (vec).count - (_at) - 1){\
			for(int __i = (_at); __i < _at + (vec).count - ((_at) + (_n)); __i++){ \
				(vec).elems[__i] = (vec).elems[__i + (_n)];\
			}\
			(vec).count -= (_n);\
		}else{\
			(vec).count -= (vec).count - (_at);\
		}\
	}while(0);

#define Vector_Concat(vec, src)\
	for(int __i = 0; __i < (src).count; __i++){\
		Vector_Add((vec), (src).elems[i])\
	}

#define Vector_ForEach(vec, _iterator) for((_iterator) = (vec).elems; (_iterator) < (vec).elems + (vec).count; (_iterator)++)
#define Vector_ReverseForEach(vec, _iterator) for((_iterator) = (vec).elems + ((vec).count - 1); (_iterator) >= (vec).elems; (_iterator)--)

#define Vector_Free(vec) free((vec).elems)

#endif
