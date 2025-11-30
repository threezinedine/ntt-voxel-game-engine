#include "MEEDEngine/MEEDEngine.h"

int main(void)
{
	meedPlatformMemoryInitialize();
	meedWindowInitialize();
	struct MEEDWindowData* pWindow = meedWindowCreate(1600, 1400, "MEEDEngine Vulkan Renderer Example");

	meedRenderInitialize(pWindow);

	struct MEEDPipeline* pPipeline =
		meedPipelineCreate(MEED_STRINGIFY(PROJECT_BASE_DIR) "/engine/build/debug/shaders/triangle.vert.spv",
						   MEED_STRINGIFY(PROJECT_BASE_DIR) "/engine/build/debug/shaders/triangle.frag.spv");

	while (!pWindow->shouldClose)
	{
		meedWindowPollEvents(pWindow);
		// Rendering and update logic would go here
	}

	meedPipelineDestroy(pPipeline);
	meedWindowDestroy(pWindow);

	meedRenderShutdown();
	meedWindowShutdown();
	meedPlatformMemoryShutdown();
	return 0;
}