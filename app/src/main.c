#include "MEEDEngine/MEEDEngine.h"

struct MEEDWindowData* pWindowData = MEED_NULL;
struct MEEDPipeline*   pPipeline   = MEED_NULL;

void mainLoop();

int main(void)
{
	meedPlatformMemoryInitialize();
	meedWindowInitialize();

	struct MEEDPlatformConsoleConfig config;
	config.color = MEED_CONSOLE_COLOR_GREEN;
	meedPlatformSetConsoleConfig(config);

	pWindowData = meedWindowCreate(800, 600, "MEED Application Window");
	meedRenderInitialize(pWindowData);

#if PLATFORM_IS_WEB
	pPipeline = meedPipelineCreate("shaders/triangle.vert", "shaders/triangle.frag");
#elif MEED_USE_VULKAN
	pPipeline = meedPipelineCreate(MEED_STRINGIFY(PROJECT_BASE_DIR) "/app/build/debug/shaders/triangle.vert.spv",
								   MEED_STRINGIFY(PROJECT_BASE_DIR) "/app/build/debug/shaders/triangle.frag.spv");
#elif MEED_USE_OPENGL
	pPipeline = meedPipelineCreate(MEED_STRINGIFY(PROJECT_BASE_DIR) "/engine/assets/shaders/opengl/triangle.vert",
								   MEED_STRINGIFY(PROJECT_BASE_DIR) "/engine/assets/shaders/opengl/triangle.frag");
#else
#error "No rendering backend selected."
#endif

#if PLATFORM_IS_WEB
	emscripten_set_main_loop(mainLoop, 0, MEED_TRUE);
#else
	while (pWindowData->shouldClose == MEED_FALSE)
	{
		mainLoop();
	}
#endif

	meedRenderWaitIdle();

	meedPipelineDestroy(pPipeline);
	meedRenderShutdown();
	meedWindowDestroy(pWindowData);

	meedWindowShutdown();
	meedPlatformMemoryShutdown();
	return 0;
}

void mainLoop()
{
	struct MEEDWindowEvent windowEvent = meedWindowPollEvents(pWindowData);

	if (windowEvent.type == MEED_WINDOW_EVENT_TYPE_CLOSE)
	{
		pWindowData->shouldClose = MEED_TRUE;
	}

	meedRenderClearScreen((struct MEEDColor){0.2f, 0.3f, 0.3f, 1.0f});

	meedRenderStartFrame();

	meedPipelineUse(pPipeline);
	meedRenderDraw(3, 1, 0, 0);

	meedRenderEndFrame();

	meedRenderPresent();
}