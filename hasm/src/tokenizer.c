#include "hasmi.h"

/*static bool StrContainsOneOf(const char* haystack, const char* needles)
{
	for(int i = 0; i < strlen(needles); i++)
		if(strchr(haystack, needles[i]))
			return true;

	return false;
}*/

char* GetToken(Hasm* me, char* line, char* token)
{
	// skip spaces etc.
	while((*line <= 32 || *line == ',') && *line != 0) line++;

	// read into token until the next space or end of string
	int at = 0;
	int idx = 0;
	
	char expecting = 0;

	char start[] = "\"'[";
	char end[] = "\"']";
	char stop[] = "()";

	// already at the end of the line
	if(*line == 0){
		*token = 0;
		return line;
	}

	// stop if the first character is a stop char
	if(strchr(stop, *line)){
		*token = *line;
		token[1] = 0;
		return line + 1;
	}

	while((expecting || (*line > 32 && *line != ',')) && *line != 0){
		if(*line == '\\'){
			line++;
			LAssert(*line != 0, "can't end a line with escaping backslash");
		}else{
			if(expecting){
				if(*line == expecting) expecting = 0;
			}else{
				if((idx = StrIndexOfChr(start, *line)) != -1)
					expecting = end[idx];
				else if(strchr(stop, *line)){
					break;
				}
			}
		}

		token[at++] = *(line++);
	}

	LAssertError(!expecting, "unterminated quotation, expected: '%c'", expecting);	
	token[at] = '\0';

	// TODO this is broken, eg. [MY_DEFINE] doesn't work
	// Handle defines	
	/*Define* it;
	Vector_ForEach(*me->defines, it){
		if(!strcmp(token, it->searchReplace[0])){
			strcpy(token, it->searchReplace[1]);
		}
	}*/

	return line;
}
