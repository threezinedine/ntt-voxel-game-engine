#include "MEEDEngine/MEEDEngine.h"

int main(void)
{
	meedPlatformMemoryInitialize();
	meedWindowInitialize();

	struct MEEDPlatformConsoleConfig config;
	config.color = MEED_CONSOLE_COLOR_GREEN;
	meedPlatformSetConsoleConfig(config);

#if PLATFORM_IS_WEB
	meedPlatformPrint("Web build detected. Window creation is not supported in this environment.\n");
#else
	meedPlatformPrint("Starting MEED Application...\n");
#endif

#if !PLATFORM_IS_WEB
	struct MEEDWindowData* pWindowData = meedWindowCreate(800, 600, "MEED Application Window");

	while (pWindowData->shouldClose == MEED_FALSE)
	{
		struct MEEDWindowEvent windowEvent = meedWindowPollEvents(pWindowData);

		if (windowEvent.type == MEED_WINDOW_EVENT_TYPE_CLOSE)
		{
			pWindowData->shouldClose = MEED_TRUE;
		}
	}

	meedWindowDestroy(pWindowData);
#endif

	meedWindowShutdown();
	meedPlatformMemoryShutdown();
	return 0;
}