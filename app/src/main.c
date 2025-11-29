#include "MEEDEngine/MEEDEngine.h"

int main(void)
{
	meedPlatformMemoryInit();
	meedWindowInitialize();

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

	meedWindowShutdown();
	meedPlatformMemoryShutdown();
	return 0;
}