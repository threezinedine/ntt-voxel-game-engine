#if MEED_USE_OPENGL

#include "MEEDEngine/platforms/common.h"
#include "MEEDEngine/platforms/window.h"
// clang-format off
#include <GL/glew.h>
#include <GLFW/glfw3.h>
// clang-format on

/**
 * Internal data for OpenGL.
 */
struct OpenGLWindowData
{
	GLFWwindow* pWindow;
};

static b8 s_isInitialized = MEED_FALSE; ///< Track if the windowing system is initialized

void meedWindowInitialize()
{
	MEED_ASSERT(s_isInitialized == MEED_FALSE);

	i32 glfwError = glfwInit();
	MEED_ASSERT_MSG(glfwError == GLFW_TRUE, "Failed to initialize GLFW with error code: %d", glfwError);

	s_isInitialized = MEED_TRUE;
}

struct MEEDWindowData* meedWindowCreate(u32 width, u32 height, const char* title)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);
	struct MEEDWindowData* pWindowData = MEED_MALLOC(struct MEEDWindowData);
	MEED_ASSERT(pWindowData != MEED_NULL);
	meedPlatformMemorySet(pWindowData, 0, sizeof(struct MEEDWindowData));

	pWindowData->width	= width;
	pWindowData->height = height;
	pWindowData->title	= title;

	pWindowData->pInternal = MEED_MALLOC(struct OpenGLWindowData);
	MEED_ASSERT(pWindowData->pInternal != MEED_NULL);
	meedPlatformMemorySet(pWindowData->pInternal, 0, sizeof(struct OpenGLWindowData));
	struct OpenGLWindowData* pOpenGLData = (struct OpenGLWindowData*)pWindowData->pInternal;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	pOpenGLData->pWindow = glfwCreateWindow((i32)width, (i32)height, title, NULL, NULL);
	MEED_ASSERT(pOpenGLData->pWindow != MEED_NULL);

	glfwMakeContextCurrent(pOpenGLData->pWindow);

	i32 glewError = glewInit();
	MEED_ASSERT_MSG(glewError == GLEW_OK, "Failed to initialize GLEW with error code: %d", glewError);

	struct MEEDPlatformConsoleConfig config = {};
	config.color							= MEED_CONSOLE_COLOR_GREEN;
	meedPlatformSetConsoleConfig(config);
	meedPlatformFPrint("OpenGL Version: %s\n", glGetString(GL_VERSION));
	config.color = MEED_CONSOLE_COLOR_RESET;
	meedPlatformSetConsoleConfig(config);

	glfwSwapInterval(1); // Enable V-Sync

	i32 framebufferWidth, framebufferHeight;
	glfwGetFramebufferSize(pOpenGLData->pWindow, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

	return pWindowData;
}

struct MEEDWindowEvent meedWindowPollEvents(struct MEEDWindowData* pWindowData)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);
	MEED_ASSERT(pWindowData != MEED_NULL);
	MEED_ASSERT(pWindowData->pInternal != MEED_NULL);
	struct OpenGLWindowData* pOpenGLData = (struct OpenGLWindowData*)pWindowData->pInternal;

	glfwPollEvents();
	pWindowData->shouldClose = glfwWindowShouldClose(pOpenGLData->pWindow) ? MEED_TRUE : MEED_FALSE;

	return (struct MEEDWindowEvent){MEED_WINDOW_EVENT_TYPE_NONE};
}

void meedWindowDestroy(struct MEEDWindowData* pWindowData)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);
	MEED_ASSERT(pWindowData != MEED_NULL);
	MEED_ASSERT(pWindowData->pInternal != MEED_NULL);
	struct OpenGLWindowData* pOpenGLData = (struct OpenGLWindowData*)pWindowData->pInternal;

	glfwDestroyWindow(pOpenGLData->pWindow);

	MEED_FREE(pWindowData->pInternal, struct OpenGLWindowData);
	MEED_FREE(pWindowData, struct MEEDWindowData);
}

void meedWindowShutdown()
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);

	glfwTerminate();

	s_isInitialized = MEED_FALSE;
}

#endif // MEED_USE_OPENGL