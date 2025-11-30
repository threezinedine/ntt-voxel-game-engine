#if PLATFORM_IS_LINUX
#include "MEEDEngine/platforms/window.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>

#if MEED_USE_VULKAN
#include <vulkan/vulkan_xlib.h>
#endif

/**
 * Internal structure to hold Linux-specific window data.
 */
struct LinuxWindowData
{
	Display* pDisplay;
	i32		 screen;
	Window	 window;
};

static i32 ignore_x_errors(Display* display, XErrorEvent* event)
{
	return 0; // Ignore all X errors
}

static XEvent event; ///< used for event polling, declared static to avoid reallocation on each poll

static b8 s_isInitialized = MEED_FALSE; ///< Track if the windowing system is initialized

#define MEED_WINDOW_UTILS_ASSERTION()                                                                                  \
	do                                                                                                                 \
	{                                                                                                                  \
		MEED_ASSERT_MSG(                                                                                               \
			s_isInitialized == MEED_TRUE,                                                                              \
			"Windowing system is not initialized. Call meedWindowInitialize() before using window functions.");        \
		MEED_ASSERT_MSG(pWindowData != MEED_NULL, "Provided MEEDWindowData pointer is NULL.");                         \
		MEED_ASSERT_MSG(pWindowData->pInternal != MEED_NULL, "Internal window data pointer is NULL.");                 \
	} while (0)

void meedWindowInitialize()
{
	MEED_ASSERT(s_isInitialized == MEED_FALSE);

	// Set X11 error handler to ignore errors
	XSetErrorHandler(ignore_x_errors);

	// Initialization complete.
	s_isInitialized = MEED_TRUE;
}

struct MEEDWindowData* meedWindowCreate(u32 width, u32 height, const char* title)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);

	struct MEEDWindowData* pWindowData = MEED_MALLOC(struct MEEDWindowData);
	MEED_ASSERT(pWindowData != MEED_NULL);
	pWindowData->width	= width;
	pWindowData->height = height;
	pWindowData->title	= title;

	pWindowData->pInternal = MEED_MALLOC(struct LinuxWindowData);
	MEED_ASSERT(pWindowData->pInternal != MEED_NULL);

	struct LinuxWindowData* pLinuxData = (struct LinuxWindowData*)pWindowData->pInternal;
	pLinuxData->pDisplay			   = XOpenDisplay(NULL);
	MEED_ASSERT(pLinuxData->pDisplay != MEED_NULL);
	pLinuxData->screen = DefaultScreen(pLinuxData->pDisplay);

	pLinuxData->window = XCreateSimpleWindow(pLinuxData->pDisplay,
											 RootWindow(pLinuxData->pDisplay, pLinuxData->screen),
											 0,
											 0,
											 width,
											 height,
											 1,
											 BlackPixel(pLinuxData->pDisplay, pLinuxData->screen),
											 WhitePixel(pLinuxData->pDisplay, pLinuxData->screen));

	XSelectInput(pLinuxData->pDisplay, pLinuxData->window, ExposureMask | KeyPressMask);
	XMapWindow(pLinuxData->pDisplay, pLinuxData->window);
	XStoreName(pLinuxData->pDisplay, pLinuxData->window, title);

	XSizeHints sizeHints;
	meedPlatformMemorySet(&sizeHints, 0, sizeof(XSizeHints));
	sizeHints.flags		 = PMinSize | PMaxSize;
	sizeHints.min_width	 = width;
	sizeHints.min_height = height;
	sizeHints.max_width	 = width;
	sizeHints.max_height = height;
	XSetWMNormalHints(pLinuxData->pDisplay, pLinuxData->window, &sizeHints);

	Atom wmDeleteMessage = XInternAtom(pLinuxData->pDisplay, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(pLinuxData->pDisplay, pLinuxData->window, &wmDeleteMessage, 1);

	return pWindowData;
}

struct MEEDWindowEvent meedWindowPollEvents(struct MEEDWindowData* pWindowData)
{
	MEED_WINDOW_UTILS_ASSERTION();

	struct LinuxWindowData* pLinuxData = (struct LinuxWindowData*)pWindowData->pInternal;

	XNextEvent(pLinuxData->pDisplay, &event);
	struct MEEDWindowEvent windowEvent;
	meedPlatformMemorySet(&windowEvent, 0, sizeof(struct MEEDWindowEvent));
	windowEvent.type = MEED_WINDOW_EVENT_TYPE_NONE;

	if (event.type == ClientMessage)
	{
		if (event.xclient.data.l[0] == XInternAtom(pLinuxData->pDisplay, "WM_DELETE_WINDOW", False))
		{
			pWindowData->shouldClose = MEED_TRUE;
			windowEvent.type		 = MEED_WINDOW_EVENT_TYPE_CLOSE;
		}
	}

	return windowEvent;
}

#if MEED_USE_VULKAN
VkResult meedWindowCreateVulkanSurface(struct MEEDWindowData* pWindowData, VkInstance instance, VkSurfaceKHR* pSurface)
{
	MEED_WINDOW_UTILS_ASSERTION();

	struct LinuxWindowData* pLinuxData = (struct LinuxWindowData*)pWindowData->pInternal;

	VkSurfaceKHR			   surface;
	VkXlibSurfaceCreateInfoKHR createInfo = {};
	createInfo.sType					  = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
	createInfo.dpy						  = pLinuxData->pDisplay;
	createInfo.window					  = pLinuxData->window;

	return vkCreateXlibSurfaceKHR(instance, &createInfo, MEED_NULL, pSurface);
}

void meedWindowDestroyVulkanSurface(struct MEEDWindowData* pWindowData, VkInstance instance, VkSurfaceKHR surface)
{
	MEED_WINDOW_UTILS_ASSERTION();

	vkDestroySurfaceKHR(instance, surface, MEED_NULL);
}

#endif

void meedWindowDestroy(struct MEEDWindowData* pWindowData)
{
	MEED_WINDOW_UTILS_ASSERTION();

	struct LinuxWindowData* pLinuxData = (struct LinuxWindowData*)pWindowData->pInternal;

	XDestroyWindow(pLinuxData->pDisplay, pLinuxData->window);
	XFlush(pLinuxData->pDisplay);
	XCloseDisplay(pLinuxData->pDisplay);

	MEED_FREE(pWindowData->pInternal, struct LinuxWindowData);
	MEED_FREE(pWindowData, struct MEEDWindowData);
}

void meedWindowShutdown()
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);

	// Shutdown complete.
	s_isInitialized = MEED_FALSE;
}

#endif // PLATFORM_IS_LINUX