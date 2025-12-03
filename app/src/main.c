#include "MEEDEngine/MEEDEngine.h"

struct MdWindowData*   pWindowData	 = MD_NULL;
struct MdPipeline*	   pPipeline	 = MD_NULL;
struct MdVertexBuffer* pVertexBuffer = MD_NULL;

struct Vertex
{
	float position[2];
	float color[3];
};

static void WriteVertexData(u8*, const void*);

static enum MdVertexBufferAttributeType vertexLayout[] = {
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT2, // position
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT3, // color
};

static struct Vertex vertices[] = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
	{{0.5f, 0.5f},  {0.0f, 1.0f, 0.0f}},
	{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
};

void mainLoop();

int main(void)
{
	mdMemoryInitialize();
	mdWindowInitialize();

	struct MdConsoleConfig config;
	config.color = MD_CONSOLE_COLOR_GREEN;
	mdSetConsoleConfig(config);

	pWindowData = mdWindowCreate(800, 600, "MEED Application Window");
	mdRenderInitialize(pWindowData);

	pVertexBuffer =
		mdVertexBufferCreate(vertexLayout, MD_ARRAY_SIZE(vertexLayout), MD_ARRAY_SIZE(vertices), WriteVertexData);

	for (u32 i = 0; i < MD_ARRAY_SIZE(vertices); ++i)
	{
		mdVertexBufferWrite(pVertexBuffer, &vertices[i]);
	}

#if PLATFORM_IS_WEB
	pPipeline = mdPipelineCreate("shaders/triangle.vert", "shaders/triangle.frag", pVertexBuffer);
#elif MD_USE_VULKAN
	pPipeline = mdPipelineCreate(MD_STRINGIFY(PROJECT_BASE_DIR) "/app/build/debug/shaders/triangle.vert.spv",
								 MD_STRINGIFY(PROJECT_BASE_DIR) "/app/build/debug/shaders/triangle.frag.spv",
								 pVertexBuffer);
#elif MD_USE_OPENGL
	pPipeline = mdPipelineCreate(MD_STRINGIFY(PROJECT_BASE_DIR) "/engine/assets/shaders/opengl/triangle.vert",
								 MD_STRINGIFY(PROJECT_BASE_DIR) "/engine/assets/shaders/opengl/triangle.frag",
								 pVertexBuffer);
#else
#error "No rendering backend selected."
#endif

#if PLATFORM_IS_WEB
	emscripten_set_main_loop(mainLoop, 0, MD_TRUE);
#else
	while (pWindowData->shouldClose == MD_FALSE)
	{
		mainLoop();
	}
#endif

	mdRenderWaitIdle();

	mdPipelineDestroy(pPipeline);
	mdVertexBufferDestroy(pVertexBuffer);
	mdRenderShutdown();
	mdWindowDestroy(pWindowData);

	mdWindowShutdown();
	mdMemoryShutdown();
	return 0;
}

void mainLoop()
{
	struct MdWindowEvent windowEvent = mdWindowPollEvents(pWindowData);

	if (windowEvent.type == MD_WINDOW_EVENT_TYPE_CLOSE)
	{
		pWindowData->shouldClose = MD_TRUE;
	}

	mdRenderClearScreen((struct MdColor){0.2f, 0.3f, 0.3f, 1.0f});

	mdRenderStartFrame();

	mdPipelineUse(pPipeline);
	mdVertexBufferBind(pVertexBuffer);
	mdRenderDraw(3, 1, 0, 0);

	mdRenderEndFrame();

	mdRenderPresent();
}

static void WriteVertexData(u8* pDest, const void* pData)
{
#if MD_USE_VULKAN
	mdMemoryCopy(pDest, pData, sizeof(struct Vertex));
#else
	struct Vertex* pVertexData = (struct Vertex*)pData;
	struct Vertex  clone	   = {};
	mdMemoryCopy(&clone, pVertexData, sizeof(struct Vertex));
	clone.position[1] = -clone.position[1];
	mdMemoryCopy(pDest, &clone, sizeof(struct Vertex));
#endif
}