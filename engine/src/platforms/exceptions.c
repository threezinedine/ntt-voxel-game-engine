#include "MEEDEngine/platforms/exceptions.h"
#include <stdlib.h>

void mdRaiseException(enum MdExceptionType type, const char* message, const char* file, u32 line)
{
	char errorBuffer[1024];

	switch (type)
	{
	case MD_EXCEPTION_TYPE_SUCCESS:
		return; // No exception to raise
	case MD_EXCEPTION_TYPE_OUT_OF_INDEX:
		mdFormatString(errorBuffer, sizeof(errorBuffer), "Out of Index Exception: %s", message);
		break;
	case MD_EXCEPTION_TYPE_EMPTY_CONTAINER:
		mdFormatString(errorBuffer, sizeof(errorBuffer), "Empty Container Exception: %s", message);
		break;
	case MD_EXCEPTION_TYPE_INVALID_OPERATION:
		mdFormatString(errorBuffer, sizeof(errorBuffer), "Invalid Operation Exception: %s", message);
		break;
	default:
		MD_UNTOUCHABLE();
	}

	struct MdConsoleConfig config;
	config.color = MD_CONSOLE_COLOR_RED;
	mdSetConsoleConfig(config);
	mdFormatPrint("Exception Raised: %s:%u: %s\n", file, line, errorBuffer);
	config.color = MD_CONSOLE_COLOR_RESET;
	mdSetConsoleConfig(config);

	exit(type);
}