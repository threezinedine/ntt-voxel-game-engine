#if PLATFORM_IS_WINDOWS
#include "MEEDEngine/platforms/console.h"
#include "windows.h"
#include <stdarg.h>
#include <stdio.h>

void mdSetConsoleConfig(struct MdConsoleConfig config)
{
	HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

	switch (config.color)
	{
	case MD_CONSOLE_COLOR_RESET:
		SetConsoleTextAttribute(console, 7);
		break;
	case MD_CONSOLE_COLOR_RED:
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_INTENSITY);
		break;
	case MD_CONSOLE_COLOR_GREEN:
		SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	case MD_CONSOLE_COLOR_YELLOW:
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		break;
	case MD_CONSOLE_COLOR_BLUE:
		SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		break;
	case MD_CONSOLE_COLOR_MAGENTA:
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		break;
	case MD_CONSOLE_COLOR_CYAN:
		SetConsoleTextAttribute(console, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		break;
	case MD_CONSOLE_COLOR_WHITE:
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		break;
	case MD_CONSOLE_COLOR_BOLD:
		SetConsoleTextAttribute(console, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | BACKGROUND_INTENSITY);
		break;
	default:
		MD_UNTOUCHABLE();
		break;
	}
}

void mdFormatString(char* buffer, mdSize length, const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, length, format, args);
	va_end(args);
}

void mdFormatPrint(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void mdPrintTrace(struct MdTraceInfo* pTraceInfo)
{
#if MD_DEBUG
	MD_UNUSED(pTraceInfo);
#else
	MD_UNUSED(pTraceInfo);
#endif
}

void mdPrint(const char* str) MD_BINDING
{
	printf("%s", str);
}

#endif // PLATFORM_IS_WINDOWS