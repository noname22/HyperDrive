#define LDEBUG
#include "common.h"

bool OpHasNextWord(uint16_t v){ 
	return (v >= DV_RefRegNextWordBase && v <= DV_RefRegNextWordTop) 
		|| v == DV_RefNextWord || v == DV_NextWord; 
}

int InsNumOps(DIns ins){
	if(ins >= DINS_2OP_BASE) return 2;
	if(ins >= DINS_1OP_BASE) return 1;
	return 0;
}

bool WriteFile(uint8_t* data, uint32_t size, const char* filename)
{
	FILE* f = fopen(filename, "w");
	LAssertWarn(f, "could not open file for writing: %s", filename);
	if(!f) return false;
	
	fwrite(data, size, 1, f);
	fclose(f);

	return true;
}

int ReadFile(uint8_t* data, uint32_t size, const char* filename)
{
	FILE* f = fopen(filename, "r");
	LAssertWarn(f, "could not open file: %s", filename);

	if(!f) return -1;

	long fSize = FileSize(f);
	if(fSize > (long)size){
		LogW("file doesn't fit in buffer");
		return -2;
	}

	LogD("reading file of size: %ld", fSize);
	memset(data, 0, size);
	LAssertWarn(fread(data, fSize, 1, f), "could not read file: %s", filename);
	fclose(f);

	return (int)fSize;
}

int ReadFileAlloc(uint8_t** data, const char* filename)
{
	FILE* f = fopen(filename, "r");
	LAssertWarn(f, "could not open file: %s", filename);
	if(!f) return -1;
	long fSize = FileSize(f);
	*data = malloc(fSize);
	LAssertWarn(fread(*data, fSize, 1, f), "could not read file: %s", filename);
	fclose(f);
	return (int)fSize;
}

void DumpRam(uint8_t* ram, uint16_t end)
{
	for(int i = 0; i < end + 1; i++){
		if(i % 8 == 0) printf("\n%02x: ", i);
		printf("%02x ", ram[i]);
	}

	printf("\n\n");
}

long FileSize(FILE* f)
{
	fseek(f, 0, SEEK_END);
	long ret = ftell(f);
	fseek(f, 0, SEEK_SET);
	return ret;
}

char* StrReplace(char* target, const char* str, const char* what, const char* with)
{
	const char* ss = strstr(str, what);

	if(!ss) strcpy(target, str);
	else{
		int at = (intptr_t)ss - (intptr_t)str;
			
		strncpy(target, str, at);
		strcpy(target + at, with);
		strcpy(target + at + strlen(with), str + at + strlen(what));
	}

	return target;
}

