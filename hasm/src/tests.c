#include "hasmi.h"

#define TAssert(__v1, __v2) LAssert(!strcmp(__v1, __v2), "Test failed in function %s at %s:%d." \
	" Expected '%s' but got '%s'.", __func__, __FILE__, __LINE__, __v2, __v1)

static void Tokenizer(){
	char buffer[] = "tokenize this(you moth\ner )...F)E(*", token[MAX_STR_SIZE];
	char* line = buffer;
	char* expected[] = {"tokenize", "this", "(", "you", "moth", "er", ")", "...F", ")", "E", "(", "*", ""};

	for(int i = 0; i < sizeof(expected) / sizeof(char*); i++){
		line = GetToken(NULL, line, token); 
		TAssert(token, expected[i]);
	}
}

static void Strings(){
	char out[MAX_STR_SIZE] = {0};

	TAssert(StrStrip(out, "  \t  strip meh  \t\t  "), "strip meh");
	TAssert(StrStrip(out, "st "), "st");
	TAssert(StrStrip(out, " st"), "st");
	TAssert(StrStrip(out, " st "), "st");
	TAssert(StrStrip(out, "st"), "st");
	TAssert(StrStrip(out, "asd\t\t 123"), "asd\t\t 123");
}

int Tests(int argc, char** argv)
{
	extern int logLevel;
	logLevel = 0;

	Tokenizer();
	Strings();

	LogD("Ok!");
	return 0;
}
