#include "hasmi.h"

#define TAssertStr(__v1, __v2) {const char* _v1 = __v1; const char* _v2 = __v2; LAssert(!strcmp(_v1, _v2), "Test failed in function %s at %s:%d." \
	" Expected '%s' but got '%s'.", __func__, __FILE__, __LINE__, _v2, _v1)}

#define TAssertEq(__v1, __v2) {int _v1 = __v1; int _v2 = __v2; LAssert((_v1) == (_v2), "Test failed in function %s at %s:%d." \
	" Expected '%d' but got '%d'.", __func__, __FILE__, __LINE__, _v2, _v1);}

static void DefinesTest()
{
	Defines* d = Defines_Create();
	TAssertEq(Defines_Push(d, "test", "tset"), true);
	TAssertEq(Defines_Push(d, "test", "tset"), false);
	
	TAssertEq(Defines_Push(d, "replace_me", "replaced"), true);
	TAssertEq(Defines_Push(d, "my", "stuff"), true);
	TAssertEq(Defines_Push(d, "a1", "1"), true);

	const char* test[][2] = {
		{"test.test", "test.test"},
		{"a", "a"},
		{"(", "("},
		{"mymymy", "mymymy"},
		{"my mymy", "stuff mymy"},
		{"my#my#my", "stuffstuffstuff"},
		{
			"test1 test[ dont_replace_me replace_me]test5",
			"test1 tset[ dont_replace_me replaced]test5"
		},

		{
			"test#test1 [ dont_replace_me replace_me]test5",
			"tsettest1 [ dont_replace_me replaced]test5"
		},
	};

	for(int i = 0; i < sizeof(test) / (2 * sizeof(char**)); i++){
		char buffer[MAX_STR_SIZE] = {0};
		strcpy(buffer, test[i][0]);
		TAssertStr(Defines_Replace(d, buffer), test[i][1]);
	}

	/*TAssertStr(Defines_Replace(d, "test1 test2[test3 test4]test5"), "tset");
	TAssertStr(Defines_Replace(d, "[test]"), "[tset]");
	TAssertStr(Defines_Replace(d, "[test+a]"), "[tset+a]");
	TAssertStr(Defines_Replace(d, "[test+ttestt]"), "[tset+ttestt]");
	TAssertStr(Defines_Replace(d, "ttestt"), "ttestt");*/
}

static void Tokenizer(){
	char buffer[] = "tokenize#this(you moth\ner )...F)E(*", token[MAX_STR_SIZE];
	char* line = buffer;
	char* expected[] = {"tokenize#this", "(", "you", "moth", "er", ")", "...F", ")", "E", "(", "*", ""};

	for(int i = 0; i < sizeof(expected) / sizeof(char*); i++){
		line = GetToken(NULL, line, token); 
		TAssertStr(token, expected[i]);
	}
}

static void Math()
{
	// HMAX
	TAssertEq(HMAX(0, 1), 1);
	TAssertEq(HMAX(10, -100), 10);
	TAssertEq(HMAX(10, 5), 10);

	// HMIN
	TAssertEq(HMIN(0, 1), 0);
	TAssertEq(HMIN(10, -100), -100);
	TAssertEq(HMIN(10, 5), 5);

	// HCLAMP
	TAssertEq(HCLAMP(100, 25, 50), 50);
	TAssertEq(HCLAMP(0, 25, 50), 25);
	TAssertEq(HCLAMP(-100, 25, 50), 25);
	TAssertEq(HCLAMP(-1, 0, 10), 0);
}

static void Strings(){
	char out[MAX_STR_SIZE] = {0};

	// Strip
	TAssertStr(StrStrip(out, "  \t  strip meh  \t\t  "), "strip meh");
	TAssertStr(StrStrip(out, "st "), "st");
	TAssertStr(StrStrip(out, " st"), "st");
	TAssertStr(StrStrip(out, " st "), "st");
	TAssertStr(StrStrip(out, "st"), "st");
	TAssertStr(StrStrip(out, "asd\t\t 123"), "asd\t\t 123");

	// Substring
	TAssertStr(StrSubStr(out, "0123456789", 2, 4), "2345");
	TAssertStr(StrSubStr(out, "0123456789", 0, 4), "0123");
	TAssertStr(StrSubStr(out, "0123456789", -1, 4), "0123");
	TAssertStr(StrSubStr(out, "0123456789", 0, -1), "");
	TAssertStr(StrSubStr(out, "0123456789", 1, 1000), "123456789");
	TAssertStr(StrSubStr(out, "0123456789", 1000, 0), "");
	TAssertStr(StrSubStr(out, "0123456789", 0, 0), "");
	TAssertStr(StrSubStr(out, "0123456789", 9, 1), "9");
	TAssertStr(StrSubStr(out, "", 9, 1), "");

	// Replace range
	TAssertStr(StrReplaceRange(out, "0123456789", 0, 3, "0321"), "03213456789");
	TAssertStr(StrReplaceRange(out, "0123456789", 0, 3, "0"), "03456789");
	TAssertStr(StrReplaceRange(out, "0123456789", 3, 3, "0"), "01206789");
	TAssertStr(StrReplaceRange(out, "0123456789", -1, 100, "999"), "999");
	TAssertStr(StrReplaceRange(out, "0123456789", -1, 0, "999"), "0123456789");
	TAssertStr(StrReplaceRange(out, "0123456789", -1, -1, "999"), "0123456789");
	TAssertStr(StrReplaceRange(out, "0123456789", 9, 12, "999"), "012345678999");
}



int Tests(int argc, char** argv)
{
	Math();
	Strings();
	Tokenizer();
	DefinesTest();

	LogI("Ok!");
	return 0;
}
