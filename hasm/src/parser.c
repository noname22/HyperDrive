#include "hasmi.h"

static const char* dinsNames[] = DINSNAMES;
static const char* valNames[] = VALNAMES;

// Assembler directives
#define AD_NUM (AD_Dd + 1)
#define AD2INS(_n) (-2 - (_n))
#define INS2AD(_n) (-(_n) - 2)

typedef enum                           { AD_Org, AD_Const, AD_Reserve, AD_Fill, AD_IncBin, AD_Include, AD_Macro, AD_End, AD_Db, AD_Dw,  AD_Dd } AsmDir;
static const char* adNames[AD_NUM] =   { ".org", ".const", ".reserve", ".fill", ".incbin", ".include",  "macro", "end", ".db",  ".dw", ".dd"  };
int                adNumArgs[AD_NUM] = {    1,       2,        1,         2,        1,          1,         -1,     0,     -1,    -1,     -1   };

bool StrEmpty(const char* str)
{
	for(int i = 0; i < strlen(str); i++) if(str[i] > 32) return false;
	return true;
}

char* LStrip(char* str)
{
	while(*str <= 32 && *str != 0) str++;
	return str;
}

// XXX: make ParseLiteral complain about garbage after the literal
uint32_t ParseLiteral(Hasm* me, const char* str, bool* success, bool failOnError)
{
	unsigned lit = 0xaaaa;
	//char tmp[MAX_STR_SIZE];

	//if(sscanf(str, "0x%x%[^a-f0-9]", &lit, tmp) == 1 || sscanf(str, "%u%[^0-9]", &lit, tmp) == 1){
	if(sscanf(str, "0x%x", &lit) == 1 || sscanf(str, "%u", &lit) == 1){
		//LAssertError(lit < 0x10000, "Literal number must be in range 0 - 65535 (0xFFFF)");
		if(success) *success = true;
		return lit;
	}

	int sLit = 0;
	if(sscanf(str, "%d", &sLit) == 1){
		if(success) *success = true;
		return (uint32_t)sLit;
	}

	LAssertError(!failOnError, "could not parse literal: %s", str)
	if(success) *success = false;

	return lit;
}

void WriteDebugInfo(Hasm* me, uint16_t addr, int wrote, int labelsAdded)
{
	// Debug file
	if(me->debugFile){
		LogD("labels found: %d", labelsAdded);
		// Write [address] [length of output (instruction, etc)] [line number] [file]
		fprintf(me->debugFile, "%04x %04x %d %s", addr - wrote, wrote, me->lineNumber, me->currentFile);

		// Write all labels associated with this address			
		for(int i = 0; i < labelsAdded; i++) 
			fprintf(me->debugFile, " %s", me->labels->elems[me->labels->count - 1 - i].label);
		fprintf(me->debugFile, "\n");
	}
}

int LookUpReg(Hasm* me, char c, bool fail)
{
	char tab[] = "abcxyzij";
	for(int i = 0; i < 8; i++) if(tab[i] == c) return i;
	LAssertError(!fail, "No such register: %c", c);
	return -1;
}

DVals ParseOperand(Hasm* me, const char* tok, unsigned int* nextWord, char** label)
{
	char c;
	char token[MAX_STR_SIZE];
	strcpy(token, tok);

	LogD("parsing operand: %s", tok);

	*label = NULL;

	// POP / [SP++], PEEK / [SP], PUSH / [--SP]
	if(!strcmp(token, "pop") || !strcmp(token, "[sp++]")) return DV_Pop;
	if(!strcmp(token, "peek") || !strcmp(token, "[sp]")) return DV_Peek;
	if(!strcmp(token, "push") || !strcmp(token, "[--sp]")) return DV_Push;
	
	// SP, PC
	if(!strcmp(token, "sp")) return DV_SP;
	if(!strcmp(token, "pc")) return DV_PC;
	if(!strcmp(token, "o")) return DV_O;

	char buffer[MAX_STR_SIZE];
	char buffer2[MAX_STR_SIZE];

	char* b1 = buffer, *b2 = buffer2;
	
	// [nextword + register] or [register + nextword]
	if(sscanf(token, "[%[^+]+%[^]]s]", buffer, buffer2) == 2){
		// if it's on the format [register + nextword], flip it
		bool isLiteral = false;
		*nextWord = ParseLiteral(me, b2, &isLiteral, false);
		if(isLiteral){
			b2 = buffer;
			b1 = buffer2;
		}

		LogD("b1 '%s' b2 '%s'", b1, b2);
		LogD("nw: %x", *nextWord);

		// 0x1 or 1...
		isLiteral = false;
		*nextWord = ParseLiteral(me, b1, &isLiteral, false);
		if(isLiteral) goto done_parsing;

		// label
		*nextWord = 0;
		*label = strdup(b1);

		done_parsing:
		b2 = LStrip(b2);
		LogD("looking for reg '%c' (%s)", b2[0], b2);
		return DV_RefRegNextWordBase + LookUpReg(me, b2[0], true);
	}

	if(sscanf(token, "[%[^]]]", buffer)){
		// [register]
		int reg = LookUpReg(me, *buffer, false);
		if(buffer[1] == 0 && reg != -1) return DV_RefBase + reg;

		// [nextword]
		bool isLiteral = false;
		*nextWord = ParseLiteral(me, buffer, &isLiteral, false);
		if(isLiteral) return DV_RefNextWord;
	
		// [label]
		*nextWord = 0;
		*label = strdup(buffer);
		return DV_RefNextWord;
	}

	// literal or nextword
	bool isLiteral = false;
	*nextWord = ParseLiteral(me, tok, &isLiteral, false);
	if(isLiteral){
		//if(*nextWord < 0x20) return DV_LiteralBase + *nextWord;
		return DV_NextWord;
	}

	// register
	if(strlen(token) == 1 && sscanf(token, "%c", &c) == 1){
		int reg = LookUpReg(me, c, false);
		if(reg != -1) return DV_A + reg;
	}

	// label
	*nextWord = 0;
	*label = strdup(tok);
	
	return DV_NextWord;
}

char* UnquoteStr(Hasm* me, char* target, const char* str)
{
	LAssertError(
		(STARTSWITH(str, '"') && ENDSWITH(str, '"')) ||
		(STARTSWITH(str, '\'') && ENDSWITH(str, '\'')), 
		"expected quoted string, got: %s (without quotes)."
		" Did you mean \"%s\"?", str, str);


	target[0] = 0;
	return strncat(target, str + 1, strlen(str) - 2);
}

uint32_t Assemble(Hasm* me, const char* ifilename, int addr, int depth)
{
	LogV("Assembling: %s", ifilename);
	LogD("at address: 0x%x", addr);

	FILE* in = fopen(ifilename, "r");
	LAssertError(in, "could not open file: %s", ifilename);

	const char* saveFile = me->currentFile;
	int saveLineNumber = me->lineNumber;

	me->currentFile = ifilename;
	me->lineNumber = 0;

	char buffer[MAX_STR_SIZE];
	char token[MAX_STR_SIZE];

	bool done = false;
	int labelsAdded = 0;

	do{
		int wrote = 0;
		#define Write8(__val) \
			do{\
				LAssertError(addr <= me->endAddr, "Out of space in binary, at last address %x", me->endAddr);\
				me->ram[addr++] = (uint8_t)(__val);\
				wrote++;\
			}while(0);
		
		#define Write16(__val) \
			do{\
				/* Little Endian */ \
				Write8((((uint32_t)(__val)) >> 0) & 0xff);\
				Write8((((uint32_t)(__val)) >> 8) & 0xff);\
			}while(0);

		#define Write32(__val) \
			do{\
				/* Little Endian */ \
				Write8((((uint32_t)(__val)) >> 0) & 0xff);\
				Write8((((uint32_t)(__val)) >> 8) & 0xff);\
				Write8((((uint32_t)(__val)) >> 16) & 0xff);\
				Write8((((uint32_t)(__val)) >> 24) & 0xff);\
			}while(0);

		char* line = buffer;

		done = GetLine(me, in, line);

		int insnum = -1;

		int numOperands = 0;
		uint8_t operands[2] = {0, 0};
		uint32_t nextWord[2] = {0, 0};
		char* opLabels[2] = {NULL, NULL};
		char* constName;

		uint32_t tmp = 0;
		
		if(StrEmpty(line)) continue;

		int toknum = 0;

		for(;;){
			// note: GetToken "consumes" the line, that's why it's line = GetToken
			line = GetToken(me, line, token);
			
			if(StrEmpty(token)) break;

			//LogD("token: '%s'", token);

			// A label, add it and continue	
			if(toknum == 0 && ENDSWITH(token, ':')) {
				token[strlen(token) - 1] = 0;
				Labels_Define(me->labels, me, token, addr, me->currentFile, me->lineNumber);
				labelsAdded++;
				continue;
			}

			// Handle arguments to directives
			if(toknum > 0 && insnum < -1){
				AsmDir ad = INS2AD(insnum);

				LAssertError(adNumArgs[ad] == -1 || 
					(toknum <= adNumArgs[ad] && toknum + 1 >= adNumArgs[ad]),
					"%s expects %d arguments", adNames[ad], adNumArgs[ad]);

				// .dw, .dl, .dd
			 	if(ad == AD_Db || ad == AD_Dw || ad == AD_Dd){
					#define WriteN(__val)\
						if(ad == AD_Db) Write8(__val) \
						else if(ad == AD_Dw) Write16(__val) \
						else if(ad == AD_Dd) Write32(__val) 

					LogD(".db/.dw/.dd data");
					// List of characters on the "string" format
					if(token[0] == '"'){
						LAssert(ENDSWITH(token, '"'), "expected \"");
						int len = strlen(token) - 2;
						for(int i = 0; i < len; i++){
							WriteN(token[i + 1]);
							LogD("%c", token[i + 1]);
						}
					}

					// Literal number (hex or dec)
					else{ 
						bool isLiteral = false;
						uint32_t lit = ParseLiteral(me, token, &isLiteral, false);
						if(isLiteral){
							WriteN(lit);
						}else{
							// A label
							LAssertError(ad == AD_Dd, "labels can only be written in a double word data block (.dd)");
							Labels_Get(me->labels, token, addr, addr - wrote, me->currentFile, me->lineNumber);
							// Write a placeholder address
							Write32(0);
						}
					}
				}

				// .ORG
				else if(ad == AD_Org){
					addr = ParseLiteral(me, token, NULL, true);
					LAssertError(addr <= me->endAddr, "ORG outside ROM size (0x%x > 0x%x)", addr, me->endAddr + 1);
				}
		
				// .DEFINE
				else if(ad == AD_Const){	
					//def.searchReplace[toknum - 1] = strdup(token);
					//if(toknum == 2) Vector_Add(*me->defines, def); 
					if(toknum == 1)
						constName = strdup(token);

					if(toknum == 2){
						Labels_Define(me->labels, me, constName, ParseLiteral(me, token, NULL, true), me->currentFile, me->lineNumber);
						free(constName);
					}
				}	

				// .FILL
				// XXX: FILLB FILLW FILLD?
				else if(ad == AD_Fill){
					if(toknum == 1) tmp = ParseLiteral(me, token, NULL, true);
					else{
						uint32_t c = ParseLiteral(me, token, NULL, true);
						for(int i = 0; i < tmp; i++) Write8(c);
					}
				}

				// .RESERVE
				else if(ad == AD_Reserve) addr += ParseLiteral(me, token, NULL, true);

				// .INCBIN
				else if(ad == AD_IncBin){
					char ibFile[MAX_STR_SIZE];
					UnquoteStr(me, ibFile, token);
					char buffer[MAX_STR_SIZE];
					sprintf(buffer, "%s%s", me->baseDir, ibFile);
					int fSize = ReadFile(me->ram + addr, me->endAddr - addr, buffer);
					LAssertError(fSize >= 0, "binary file include failed: %s", buffer);
					LAssertWarn(fSize > 0, "included empty binary file: %s", buffer);
					LogD("incbinned file of size: %d", fSize);
					addr += fSize;
				}

				// .INCLUDE
				else if(ad == AD_Include){
					char ibFile[MAX_STR_SIZE];
					char buffer[MAX_STR_SIZE];
					sprintf(buffer, "%s%s", me->baseDir, UnquoteStr(me, ibFile, token));
					addr = Assemble(me, buffer, addr, depth + 1);
				}
			}

			// An instruction or assembly directive
			else if(toknum == 0){
				insnum = -1;

				// Assembly directives
				for(int i = 0; i < AD_NUM; i++){
					if(!strcmp(adNames[i], token)){
						LogD("Directive: %s", token);
						insnum = AD2INS(i);
						goto done_searching;
					}
				}

				// Actual instructions
				for(int i = 0; i < DINS_NUM; i++){
					if(!strcmp(dinsNames[i], token)) {
						insnum = i;
						goto done_searching;
					}
				}

				LAssertError(insnum != -1, "no such instruction or directive: %s", token);

				done_searching: 
				while(0){} // "NOOP", must have something after a label
			}

			else if( toknum == 1 || toknum == 2 ){
				operands[toknum - 1] = ParseOperand(me, token, nextWord + toknum - 1, opLabels + toknum - 1);
				numOperands++;
			}
				
			toknum++;
		}

		// Assembler directive handled (-2) or no instruction found 
		// (probably a label but no instruction), continue with next line
		if(insnum <= -1){
			if(wrote){
				// An assembler direvtive wrote data, associate any labels with it
				WriteDebugInfo(me, addr, wrote, labelsAdded);
				labelsAdded = 0;
			}
			continue;
		}
		
		LAssertError(toknum - 1 == InsNumOps(insnum), "%s expects %d operands (not %d)", dinsNames[insnum], InsNumOps(insnum), toknum - 1);
		LAssertError(operands[0] != DV_NextWord || !DINS_MODS_TARGET(insnum), "Can't assign literal (did you mean [literal]?)");

		// Write to output

		// Instruction
		Write8(insnum);

		LogD("Line: '%s'", buffer);
		LogD("  Instruction: %s (0x%02x)", dinsNames[insnum], insnum);

		// Operands
		for(int i = 0; i < numOperands; i++){
			LogD("  Operand %d: %s (0x%02x)", i + 1, valNames[operands[i]], operands[i]);
			Write8(operands[i]);
		
			if(OpHasNextWord(operands[i])){
				// This refers to a label
				if(opLabels[i]){
					Labels_Get(me->labels, opLabels[i], addr, addr - wrote, me->currentFile, me->lineNumber);
					free(opLabels[i]);
				}

				Write32(nextWord[i]);
				LogD("  NextWord: 0x%08x", nextWord[i]);
			}
		}

		if(logLevel == 0){
			char dump[128];
			memset(dump, 0, 128);
			char* d = dump;

			for(int i = 0; i < wrote; i++) d += sprintf(d, "%02x ", me->ram[addr - wrote + i]);
			LogD("  Output: %s", dump);
		}
	
		WriteDebugInfo(me, addr, wrote, labelsAdded);
		
		labelsAdded = 0;

	} while (done);

	fclose(in);
	
	me->currentFile = saveFile;
	me->lineNumber = saveLineNumber;

	return addr;
}
