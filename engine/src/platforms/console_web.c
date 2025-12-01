#if PLATFORM_IS_WEB
#include <emscripten/emscripten.h>

#include <stdarg.h>
#include <stdio.h>

#include "MEEDEngine/platforms/console.h"

void meedPlatformSetConsoleConfig(struct MEEDPlatformConsoleConfig config)
{
}

void meedPlatformFPrint(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void meedPlatformBufferedPrint(char* buffer, meedSize length, char* format, ...)
{
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, length, format, args);
	va_end(args);
}

void meedPlatformPrint(const char* str)
{
	printf("%s", str);
}

#endif // PLATFORM_IS_WEB
