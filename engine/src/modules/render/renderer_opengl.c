#if MEED_USE_OPENGL

#include "MEEDEngine/modules/render/render.h"
#include "opengl_common.h"

struct OpenGLRenderData* s_pRenderData = MEED_NULL;

static u32 vbo, vao;

void meedRenderInitialize(struct MEEDWindowData* pWindowData)
{
	MEED_ASSERT(s_pRenderData == MEED_NULL);

	s_pRenderData			   = MEED_MALLOC(struct OpenGLRenderData);
	s_pRenderData->pWindowData = pWindowData;

	// clang-format off
	float vertices[] = {
		// positions        // colors
		0.0f,  -0.5f, 1.0f, 0.0f, 0.0f, // bottom
		0.5f,  0.5f,  0.0f, 1.0f, 0.0f, // top right
		-0.5f, 0.5f,  0.0f, 0.0f, 1.0f  // top left
	};
	// clang-format on

	GL_ASSERT(glGenVertexArrays(1, &vao));
	GL_ASSERT(glGenBuffers(1, &vbo));
	GL_ASSERT(glBindVertexArray(vao));
	GL_ASSERT(glBindBuffer(GL_ARRAY_BUFFER, vbo));
	GL_ASSERT(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

	GL_ASSERT(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0));
	GL_ASSERT(glEnableVertexAttribArray(0));
}

void meedRenderClearScreen(struct MEEDColor color)
{
	MEED_ASSERT(s_pRenderData != MEED_NULL);

	GL_ASSERT(glClearColor(color.r, color.g, color.b, color.a));
	GL_ASSERT(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void meedRenderStartFrame()
{
	MEED_ASSERT(s_pRenderData != MEED_NULL);

	GL_ASSERT(glBindVertexArray(vao));
}

void meedRenderEndFrame()
{
}

void meedRenderDraw(u32 vertexCount, u32 instanceCount, u32 firstVertex, u32 firstInstance)
{
	MEED_UNUSED(instanceCount);
	MEED_UNUSED(firstInstance);

	MEED_ASSERT(s_pRenderData != MEED_NULL);

	GL_ASSERT(glDrawArrays(GL_TRIANGLES, firstVertex, vertexCount));
}

void meedRenderPresent()
{
	MEED_ASSERT(s_pRenderData != MEED_NULL);

	GLFWwindow* pWindow = (GLFWwindow*)s_pRenderData->pWindowData->pInternal;
	GL_ASSERT(glfwSwapBuffers(pWindow));
}

void meedWaitIdle()
{
}

void meedRenderShutdown()
{
	MEED_ASSERT(s_pRenderData != MEED_NULL);

	MEED_FREE(s_pRenderData, struct OpenGLRenderData);
	s_pRenderData = MEED_NULL;
	s_pRenderData = MEED_NULL;
}

#endif // MEED_USE_OPENGL