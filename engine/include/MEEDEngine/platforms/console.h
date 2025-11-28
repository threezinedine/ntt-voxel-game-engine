#pragma once
#include "common.h"

/**
 * All possible color which the console can display, using
 * `meedPlatformSetColor` for modifying the color.
 */
enum MEED_BINDING MEEDConsoleColor : u8
{
	MEED_CONSOLE_COLOR_RESET,
	MEED_CONSOLE_COLOR_RED,
	MEED_CONSOLE_COLOR_GREEN,
	MEED_CONSOLE_COLOR_YELLOW,
	MEED_CONSOLE_COLOR_BLUE,
	MEED_CONSOLE_COLOR_MAGENTA,
	MEED_CONSOLE_COLOR_CYAN,
	MEED_CONSOLE_COLOR_WHITE,
	MEED_CONSOLE_COLOR_BOLD,
};

/**
 * Modify the following content's color which is used by `meedPlatformPrint`.
 * @param color The color to set the console to.
 */
void meedPlatformSetColor(enum MEEDConsoleColor color) MEED_BINDING;

/**
 * Print formatted content to the console.
 * @param format The format string.
 * @param ... The format arguments.
 */
void meedPlatformPrint(const char *format, ...);