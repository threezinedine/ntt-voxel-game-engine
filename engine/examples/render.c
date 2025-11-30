#include "MEEDEngine/MEEDEngine.h"

int main(void)
{
	meedPlatformMemoryInitialize();
	meedWindowInitialize();
	struct MEEDWindowData* pWindow = meedWindowCreate(1600, 1400, "MEEDEngine Vulkan Renderer Example");

	meedRenderInitialize(pWindow);

	while (!pWindow->shouldClose)
	{
		meedWindowPollEvents(pWindow);
		// Rendering and update logic would go here
	}

	meedWindowDestroy(pWindow);

	meedRenderShutdown();
	meedWindowShutdown();
	meedPlatformMemoryShutdown();
	return 0;
}