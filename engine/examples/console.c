#include "MEEDEngine/MEEDEngine.h"

int main(void)
{
	struct MEEDPlatformConsoleConfig config;
	config.color = MEED_CONSOLE_COLOR_BLUE;

	meedPlatformSetConsoleConfig(config);
	meedPlatformPrint("Hello, MEED Engine Console Green!\n");

	config.color = MEED_CONSOLE_COLOR_RED;
	meedPlatformSetConsoleConfig(config);
	meedPlatformPrint("Hello, MEED Engine Console Red!\n");

	config.color = MEED_CONSOLE_COLOR_RESET;
	meedPlatformSetConsoleConfig(config);
	meedPlatformPrint("Hello, MEED Engine Console Reset!\n");
	return 0;
}