#include "MEEDEngine/MEEDEngine.h"

struct Vertex
{
	float position[2];
	float color[3];
};

static void WriteVertexData(u8*, const void*);

enum MdVertexBufferAttributeType vertexLayout[] = {
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT2, // position
	MD_VERTEX_BUFFER_ATTRIBUTE_TYPE_FLOAT3	// color
};

static struct Vertex vertices[] = {
	{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}}, // Bottom vertex (Red)
	{{0.5f, 0.5f},  {1.0f, 0.0f, 0.0f}}, // Right vertex (Green)
	{{-0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}}, // Left vertex (Blue)
};

int main(void)
{
	mdMemoryInitialize();
	mdWindowInitialize();

#if MD_USE_VULKAN
	struct MdWindowData* pWindow = mdWindowCreate(1600, 1400, "MEEDEngine Vulkan Renderer Example");
#else
	struct MdWindowData* pWindow = mdWindowCreate(1600, 1400, "MEEDEngine OpenGL Renderer Example");
#endif

	mdRenderInitialize(pWindow);

	struct MdVertexBuffer* pVertexBuffer =
		mdVertexBufferCreate(vertexLayout, MD_ARRAY_SIZE(vertexLayout), 3, WriteVertexData);

	u32 verticesCount = MD_ARRAY_SIZE(vertices);
	for (u32 vertexIndex = 0u; vertexIndex < verticesCount; ++vertexIndex)
	{
		mdVertexBufferWrite(pVertexBuffer, &vertices[vertexIndex]);
	}

#if MD_USE_VULKAN
	struct MdPipeline* pPipeline =
		mdPipelineCreate(MD_STRINGIFY(PROJECT_BASE_DIR) "/engine/build/debug/shaders/triangle.vert.spv",
						 MD_STRINGIFY(PROJECT_BASE_DIR) "/engine/build/debug/shaders/triangle.frag.spv",
						 pVertexBuffer);
#elif MD_USE_OPENGL
	struct MdPipeline* pPipeline =
		mdPipelineCreate(MD_STRINGIFY(PROJECT_BASE_DIR) "/engine/assets/shaders/opengl/triangle.vert",
						 MD_STRINGIFY(PROJECT_BASE_DIR) "/engine/assets/shaders/opengl/triangle.frag",
						 MD_NULL);
#else
#error "No rendering backend selected."
#endif

	while (!pWindow->shouldClose)
	{
		mdWindowPollEvents(pWindow);

		mdRenderClearScreen((struct MdColor){0.1f, 0.1f, 0.1f, 1.0f});

		mdRenderStartFrame();

		// Rendering commands below...
		mdPipelineUse(pPipeline);
		mdVertexBufferBind(pVertexBuffer);
		mdRenderDraw(3, 1, 0, 0);

		// Rendering commands above...

		mdRenderEndFrame();

		mdRenderPresent();
	}

	mdRenderWaitIdle();

	mdPipelineDestroy(pPipeline);
	mdVertexBufferDestroy(pVertexBuffer);
	mdWindowDestroy(pWindow);

	mdRenderShutdown();
	mdWindowShutdown();
	mdMemoryShutdown();
	return 0;
}

static void WriteVertexData(u8* pDest, const void* pData)
{
	struct Vertex* pVertexData = (struct Vertex*)pData;
#if MD_USE_OPENGL
	struct Vertex clone = {};
	mdMemoryCopy(&clone, pVertexData, sizeof(struct Vertex));
	clone.position[1] = -clone.position[1];
	mdMemoryCopy(pDest, &clone, sizeof(struct Vertex));
#else
	mdMemoryCopy(pDest, pVertexData, sizeof(struct Vertex));
#endif
}