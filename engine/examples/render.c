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

		meedRenderClearScreen((struct MEEDColor){0.1f, 0.1f, 0.1f, 1.0f});

		meedRenderStartFrame();

		// Rendering commands below...
		meedPipelineUse(pPipeline);
		meedRenderDraw(3, 1, 0, 0);

		// Rendering commands above...

		meedRenderEndFrame();

		meedRenderPresent();
	}

	meedWaitIdle();

	meedPipelineDestroy(pPipeline);
	meedWindowDestroy(pWindow);

	meedRenderShutdown();
	meedWindowShutdown();
	meedPlatformMemoryShutdown();
	return 0;
}