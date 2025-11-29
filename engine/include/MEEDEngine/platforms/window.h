#pragma once
#include "common.h"

/**
 * @file window.h
 * The utilities for managing the windowing system inside the `MEEDEngine`.
 *
 * @example
 * ```
 * struct MEEDWindowData* pWindowData = meedWindowCreate(800, 600, "MEED Application Window");
 *
 * // Main loop
 * while (!pWindowData->shouldClose)
 * {
 *     meedWindowPollEvents(pWindowData);
 *     // Rendering and other logic here...
 * }
 *
 * meedWindowDestroy(pWindowData);
 * ```
 */

/**
 * Enumeration of possible window event types.
 */
enum MEEDWindowEventType
{
	MEED_WINDOW_EVENT_TYPE_NONE,  ///< No event, but the polling always returns this when there are no events.
	MEED_WINDOW_EVENT_TYPE_CLOSE, ///< The window close event.
	MEED_WINDOW_EVENT_TYPE_COUNT,
};

/**
 * Represents a window event.
 * (Currently empty, can be expanded in the future for event handling.)
 */
struct MEEDWindowEvent
{
	enum MEEDWindowEventType type; ///< The type of the window event.
};

/**
 * Needed information for working with the windowing system.
 */
struct MEED_BINDING MEEDWindowData
{
	void* pInternal; ///< Used for storing custom windowing system data (e.g Windows, Linux, MacOS, SDL, GLFW, etc.)

	u32			width;		 ///< The width of the window.
	u32			height;		 ///< The height of the window.
	const char* title;		 ///< The title of the window.
	b8			shouldClose; ///< Flag indicating whether the window should close.
};

/**
 * Initialize the windowing system. Must be called before using any window-related functions.
 */
void meedWindowInitialize();

/**
 * Start a new window (display) for rendering.
 *
 * @param width The width of the window in pixels.
 * @param height The height of the window in pixels.
 * @param title The title of the window as a null-terminated string.
 *
 * @return A pointer to the created window data.
 */
struct MEEDWindowData* meedWindowCreate(u32 width, u32 height, const char* title);

/**
 * Poll for window events (e.g., input, close events).
 *
 * @param pWindowData A pointer to the window data to poll events from.
 * @return A `MEEDWindowEvent` structure containing the polled event information.
 */
struct MEEDWindowEvent meedWindowPollEvents(struct MEEDWindowData* pWindowData);

/**
 * Destroy a previously created window.
 *
 * @param pWindowData A pointer to the window data to destroy.
 */
void meedWindowDestroy(struct MEEDWindowData* pWindowData);

/**
 * Shutdown the windowing system. Must be called after all window-related functions are done.
 */
void meedWindowShutdown();
