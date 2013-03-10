#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "hcpuins.h"
#include "log.h"
#include "cvector.h"

#define HMIN(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#define HMAX(_a, _b) ((_a) > (_b) ? (_a) : (_b))

#define HCLAMP(_v, _min, _max) HMAX( HMIN((_v), (_max)), (_min) )

extern int logLevel;
typedef enum {DBO_LittleEndian, DBO_BigEndian } DByteOrder;

uint32_t GetUsedRam(uint8_t* rom);
void DumpRam(uint8_t* ram, uint32_t end);

bool WriteFile(uint8_t* data, uint32_t size, const char* filename);
int ReadFile(uint8_t* data, uint32_t size, const char* filename);
int ReadFileAlloc(uint8_t** data, const char* filename);

long FileSize(FILE* f);

char* StrReplace(char* target, const char* str, const char* what, const char* with);
char* StrStrip(char* target, const char* str);
int StrIndexOfChr(const char* s, int c);
char* StrSubStr(char* target, const char* str, int from, int to);
char* StrReplaceRange(char* target, const char *str, int from, int len, const char* with);

int LoadRamMax(uint8_t* rom, const char* filename, uint16_t lastAddr);

bool OpHasNextWord(uint16_t v);
int InsNumOps(DIns ins);

#endif
