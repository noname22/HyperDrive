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

void DumpRam(uint8_t* ram, uint32_t end)
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

char* StrStrip(char* target, const char* str)
{
	int start = 0, end = strlen(str);

	for(int i = 0; i < strlen(str); i++){
		if(str[i] == '\t' || str[i] == ' ')
			start++;
		else
			break;
	}
	
	for(int i = strlen(str) - 1; i >= 0; i--){
		if(str[i] == '\t' || str[i] == ' ')
			end--;
		else
			break;
	}

	strncpy(target, str + start, end - start);
	target[end - start] = 0;

	return target;
}

char* StrSubStr(char* target, const char* str, int from, int len)
{
	target[0] = 0;
	int i;

	from = HMAX(from, 0);
	len = HCLAMP(len, 0, (int)strlen(str) - from);

	if(from > strlen(str))
		return target;

	for(i = 0; i < len; i++)
		target[i] = str[i + from];

	target[i] = 0;
	return target;
}

char* StrReplaceRange(char* target, const char *str, int from, int len, const char* with)
{
	target[0] = 0;
	char* w = target;
	const char* r = str, *r2 = with;

	from = HMAX(from, 0);
	len = HCLAMP(len, 0, (int)strlen(str) - from);
	
	if(len == 0){
		strcpy(target, str);
		return target;
	}

	for(int i = 0; i < from; i++)
		*(w++) = *(r++);

	while(*r2 != 0)
		*(w++) = *(r2++);

	for(int i = 0; i < len; i++)
		r++;

	while(*r != 0)
		*(w++) = *(r++);

	*w = 0;
	return target;
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

int StrIndexOfChr(const char* s, int c)
{
	char* ptr = 0;
	if((ptr = strchr(s, c)) != NULL)
		return (intptr_t)ptr - (intptr_t)s;

	return -1;
}
