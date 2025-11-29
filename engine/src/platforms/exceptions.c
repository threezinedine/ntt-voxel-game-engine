#include "MEEDEngine/platforms/exceptions.h"
#include <stdlib.h>

void meedRaiseException(enum MEEDExceptionType type, const char* message, const char* file, u32 line)
{
	char errorBuffer[1024];

	switch (type)
	{
	case MEED_EXCEPTION_TYPE_SUCCESS:
		return; // No exception to raise
	case MEED_EXCEPTION_TYPE_OUT_OF_INDEX:
		meedPlatformBufferedPrint(errorBuffer, sizeof(errorBuffer), "Out of Index Exception: %s", message);
		break;
	case MEED_EXCEPTION_TYPE_EMPTY_CONTAINER:
		meedPlatformBufferedPrint(errorBuffer, sizeof(errorBuffer), "Empty Container Exception: %s", message);
		break;
	default:
		MEED_UNTOUCHABLE();
	}

	struct MEEDPlatformConsoleConfig config;
	config.color = MEED_CONSOLE_COLOR_RED;
	meedPlatformSetConsoleConfig(config);
	meedPlatformFPrint("Exception Raised: %s:%u: %s\n", file, line, errorBuffer);

	exit(type);
}