#if MD_USE_OPENGL

#include "MEEDEngine/platforms/common.h"
#include "MEEDEngine/platforms/window.h"

// clang-format off
#if !PLATFORM_IS_WEB
#include <glad/glad.h>
#endif
#include <GLFW/glfw3.h>
// clang-format on

/**
 * Internal data for OpenGL.
 */
struct OpenGLWindowData
{
	GLFWwindow* pWindow;
};

static b8 s_isInitialized = MD_FALSE; ///< Track if the windowing system is initialized

void mdWindowInitialize()
{
	MD_ASSERT(s_isInitialized == MD_FALSE);

	i32 glfwError = glfwInit();
	MD_ASSERT_MSG(glfwInit() == GLFW_TRUE, "Failed to initialize GLFW with error code: %d", glfwError);

	s_isInitialized = MD_TRUE;
}

struct MdWindowData* mdWindowCreate(u32 width, u32 height, const char* title)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);
	struct MdWindowData* pWindowData = MD_MALLOC(struct MdWindowData);
	MD_ASSERT(pWindowData != MD_NULL);
	mdMemorySet(pWindowData, 0, sizeof(struct MdWindowData));

	pWindowData->width	= width;
	pWindowData->height = height;
	pWindowData->title	= title;

	pWindowData->pInternal = MD_MALLOC(struct OpenGLWindowData);
	MD_ASSERT(pWindowData->pInternal != MD_NULL);
	mdMemorySet(pWindowData->pInternal, 0, sizeof(struct OpenGLWindowData));
	struct OpenGLWindowData* pOpenGLData = (struct OpenGLWindowData*)pWindowData->pInternal;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE); // already default but force it anyway

	pOpenGLData->pWindow = glfwCreateWindow((i32)width, (i32)height, title, NULL, NULL);
	MD_ASSERT(pOpenGLData->pWindow != MD_NULL);

	glfwSetWindowAttrib(pOpenGLData->pWindow, GLFW_AUTO_ICONIFY, GLFW_FALSE);

	glfwMakeContextCurrent(pOpenGLData->pWindow);

#if !PLATFORM_IS_WEB
	MD_ASSERT_MSG(gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 1, "Failed to initialize GLAD");
#endif

	struct MdConsoleConfig config = {};
	config.color				  = MD_CONSOLE_COLOR_GREEN;
	mdSetConsoleConfig(config);
	mdFormatPrint("OpenGL Version: %s\n", glGetString(GL_VERSION));
	mdFormatPrint("Renderer: %s\n", glGetString(GL_RENDERER));
	config.color = MD_CONSOLE_COLOR_RESET;
	mdSetConsoleConfig(config);

	glfwSwapInterval(1); // Enable V-Sync

	i32 framebufferWidth, framebufferHeight;
	glfwGetFramebufferSize(pOpenGLData->pWindow, &framebufferWidth, &framebufferHeight);
	glViewport(0, 0, framebufferWidth, framebufferHeight);

	return pWindowData;
}

struct MdWindowEvent mdWindowPollEvents(struct MdWindowData* pWindowData)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);
	MD_ASSERT(pWindowData != MD_NULL);
	MD_ASSERT(pWindowData->pInternal != MD_NULL);
	struct OpenGLWindowData* pOpenGLData = (struct OpenGLWindowData*)pWindowData->pInternal;

	glfwPollEvents();
	pWindowData->shouldClose = glfwWindowShouldClose(pOpenGLData->pWindow) ? MD_TRUE : MD_FALSE;

	return (struct MdWindowEvent){MD_WINDOW_EVENT_TYPE_NONE};
}

void mdWindowDestroy(struct MdWindowData* pWindowData)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);
	MD_ASSERT(pWindowData != MD_NULL);
	MD_ASSERT(pWindowData->pInternal != MD_NULL);
	struct OpenGLWindowData* pOpenGLData = (struct OpenGLWindowData*)pWindowData->pInternal;

	glfwDestroyWindow(pOpenGLData->pWindow);

	MD_FREE(pWindowData->pInternal, struct OpenGLWindowData);
	MD_FREE(pWindowData, struct MdWindowData);
}

void mdWindowShutdown()
{
	MD_ASSERT(s_isInitialized == MD_TRUE);

	glfwTerminate();

	s_isInitialized = MD_FALSE;
}

#endif // MD_USE_OPENGL