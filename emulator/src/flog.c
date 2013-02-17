#include "flog.h"
#include <stdarg.h>

#define ABlack "0"
#define ARed "1"
#define AGreen "2"
#define AYellow "3"
#define ABlue "4"
#define AMagenta "5"
#define ACyan "6"
#define AWhite "7"

#define AnsiColorIn(__color) "\033[9" __color "m"
#define AnsiColorOut "\033[39m"
#define AnsiColorBGIn(__color) "\033[4" __color "m"
#define AnsiColorBGOut "\033[49m"

#define AnsiBoldIn "\033[1m"
#define AnsiBoldOut "\033[22m"
#define AnsiUnderlineIn "\033[4m" 
#define AnsiUnderlineOut "\033[24m"
#define AnsiInvertIn "\033[7m" 
#define AnsiInvertOut "\033[27m"

int Flog_SeverityToIndex(Flog_Severity severity)
{
	for(int i = 0; i < 8; i++){
		if(((int)severity >> i) & 1){
			return i;
		}
	}

	return 8;
}

const char* Flog_SeverityToString(Flog_Severity severity)
{
	static const char *severityString[] = { "D1", "D2", "D3", "VV", "II", "WW", "EE", "FF", "??" };
	return severityString[Flog_SeverityToIndex(severity)];
}

void Flog_Log(const char* file, uint32_t lineNumber, Flog_Severity severity, const char* format, ...)
{
	va_list fmtargs;
	char buffer[4096];

	buffer[sizeof(buffer) - 1] = '\0';

	va_start(fmtargs, format);
	vsnprintf(buffer, sizeof(buffer) - 1, format, fmtargs);
	va_end(fmtargs);
	
	printf("[%s] %s:%d %s\r\n", Flog_SeverityToString(severity), file, lineNumber, buffer);
	fflush(stdout);
}
