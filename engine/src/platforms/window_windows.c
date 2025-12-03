#if PLATFORM_IS_WINDOWS && !MD_USE_OPENGL
#include "MEEDEngine/platforms/window.h"
#include "windows.h"

struct WindowsWindowData
{
	HWND hwnd;
};

static b8 s_isInitialized = MD_FALSE;

void mdWindowInitialize()
{
	MD_ASSERT(s_isInitialized == MD_FALSE);

	// Initialize code below this line if needed in the future.

	// Initialization complete.

	s_isInitialized = MD_TRUE;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct MdWindowData* mdWindowCreate(u32 width, u32 height, const char* title)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);

	struct MdWindowData* pWindowData = MD_MALLOC(struct MdWindowData);
	MD_ASSERT(pWindowData != MD_NULL);
	mdMemorySet(pWindowData, 0, sizeof(struct MdWindowData));

	pWindowData->width		 = width;
	pWindowData->height		 = height;
	pWindowData->title		 = title;
	pWindowData->shouldClose = MD_FALSE;

	struct WindowsWindowData* pWindowsData = MD_MALLOC(struct WindowsWindowData);
	MD_ASSERT(pWindowsData != MD_NULL);
	pWindowData->pInternal = pWindowsData;

	pWindowsData->hwnd = CreateWindowA("STATIC",
									   title,
									   WS_OVERLAPPEDWINDOW,
									   CW_USEDEFAULT,
									   CW_USEDEFAULT,
									   (int)width,
									   (int)height,
									   NULL,
									   NULL,
									   GetModuleHandleA(NULL),
									   NULL);

	ShowWindow(pWindowsData->hwnd, SW_SHOW);

	return pWindowData;
}

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}

struct MdWindowEvent mdWindowPollEvents(struct MdWindowData* pWindowData)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);
	MD_ASSERT(pWindowData != MD_NULL);

	struct MdWindowEvent event;
	event.type = MD_WINDOW_EVENT_TYPE_NONE;

	struct WindowsWindowData* pWindowsData = (struct WindowsWindowData*)pWindowData->pInternal;

	MSG msg;
	while (PeekMessageA(&msg, pWindowsData->hwnd, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessageA(&msg);

		if (msg.message == WM_QUIT)
		{
			pWindowData->shouldClose = MD_TRUE;
			event.type				 = MD_WINDOW_EVENT_TYPE_CLOSE;
		}
	}

	return event;
}

void mdWindowDestroy(struct MdWindowData* pWindowData)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);
	MD_ASSERT(pWindowData != MD_NULL);

	struct WindowsWindowData* pWindowsData = (struct WindowsWindowData*)pWindowData->pInternal;

	DestroyWindow(pWindowsData->hwnd);
	MD_FREE(pWindowsData, struct WindowsWindowData);
	MD_FREE(pWindowData, struct MdWindowData);
}

void mdWindowShutdown()
{
	MD_ASSERT(s_isInitialized == MD_TRUE);

	// Shutdown code below this line if needed in the future.

	// Shutdown complete.

	s_isInitialized = MD_FALSE;
}

#endif // PLATFORM_IS_WINDOWS && !MD_USE_OPENGL