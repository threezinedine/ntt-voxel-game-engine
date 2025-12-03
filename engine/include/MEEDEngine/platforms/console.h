#pragma once
#include "common.h"

#if __cplusplus
extern "C" {
#endif

#define MD_MAX_TRACE_FRAMES 16

/**
 * All possible color which the console can display, using
 * `mdPlatformSetColor` for modifying the color.
 */
enum MD_BINDING MdConsoleColor
{
	MD_CONSOLE_COLOR_RESET,
	MD_CONSOLE_COLOR_RED,
	MD_CONSOLE_COLOR_GREEN,
	MD_CONSOLE_COLOR_YELLOW,
	MD_CONSOLE_COLOR_BLUE,
	MD_CONSOLE_COLOR_MAGENTA,
	MD_CONSOLE_COLOR_CYAN,
	MD_CONSOLE_COLOR_WHITE,
	MD_CONSOLE_COLOR_BOLD,
};

/**
 * Configuration structure for the console platform.
 */
struct MD_BINDING MdConsoleConfig
{
	enum MdConsoleColor color; ///< The color to set the console to.
};

/**
 * Modify the following content's color which is used by `mdPrint`.
 * @param color The color to set the console to.
 */
void mdSetConsoleConfig(struct MdConsoleConfig config) MD_BINDING;

/**
 * Print formatted content to a buffer.
 * @param buffer The buffer to print to.
 * @param length The length of the buffer.
 * @param format The format string.
 * @param ... The format arguments.
 */
void mdFormatString(char* buffer, mdSize length, const char* format, ...);

/**
 * Print formatted content to the console.
 * @param format The format string.
 * @param ... The format arguments.
 */
void mdFormatPrint(const char* format, ...);

/**
 * Structure used for storing trace information.
 */
struct MdTraceInfo
{
	void*  frames[MD_MAX_TRACE_FRAMES];
	mdSize framesCount;
	mdPid  threadId;
};

/**
 * Used for printing the trace of the current call stack to the console.
 * @param pTraceInfo The trace information to print. if null, will capture the current trace.
 */
void mdPrintTrace(struct MdTraceInfo* pTraceInfo);

/**
 * Print a string to the console.
 * @param str The string to print.
 */
void mdPrint(const char* str) MD_BINDING;

#if __cplusplus
}
#endif