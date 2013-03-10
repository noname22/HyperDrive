#include "hasmi.h"

struct Reader {
	int (*Get)(Reader* me);
	int len;
	int lineNumber;
	int at;

	char* filename;

	FILE* f;
	char* buffer;
};

int Reader_GetF(Reader* me)
{
	return fgetc(me->f);
}

int Reader_GetB(Reader* me)
{
	if(me->at >= me->len)
		return EOF;
	return me->buffer[me->at++];
}

Reader* Reader_CreateFromFile(const char* filename)
{
	Reader* me = calloc(1, sizeof(Reader));
	LAssert(me, "could not allocate ram for reader");

	me->Get = Reader_GetF;

	FILE* in = fopen(filename, "r");

	if(!in){
		free(me);
		return NULL;
	}

	me->filename = strdup(filename);

	me->f = in;

	return me;
}

Reader* Reader_CreateFromBuffer(char* buffer, int len, const char* name, int startLineNum)
{
	Reader* me = calloc(1, sizeof(Reader));
	LAssert(me, "could not allocate ram for reader");

	me->Get = Reader_GetB;

	me->filename = strdup(name);
	me->lineNumber = startLineNum;

	me->buffer = buffer;
	me->len = len;

	return me;
}

int Reader_GetLineNumber(Reader* me)
{
	return me->lineNumber; 
}

const char* Reader_GetFilename(Reader* me)
{
	return me->filename;
}

bool Reader_GetLine(Reader* me, char* buffer)
{
	me->lineNumber++;
	int at = 0;

	for(;;) {
		int c = me->Get(me);
		if(c == '\r' || c == '\n' || c == EOF || c == ';'){
			bool ret = c != EOF;
			while(c != '\n' && c != EOF){ c = me->Get(me); }
			buffer[at] = '\0';
			return ret;
		}

		buffer[at++] = c;
	}
}

void Reader_Destroy(Reader** me)
{
	if((*me)->f)
		fclose((*me)->f);

	free((*me)->filename);
	free(*me);

	*me = NULL;
}
