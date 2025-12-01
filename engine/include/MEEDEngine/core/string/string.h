#pragma once

#if __cppplus
extern "C" {
#endif
#include "MEEDEngine/platforms/platforms.h"

/**
 * Gets the length of a null-terminated string.
 * @param str The null-terminated string.
 * @return The length of the string, excluding the null terminator.
 */
u32 mdGetStringLength(const char* str);

/**
 * Compares two null-terminated strings for equality.
 * @param str1 The first null-terminated string.
 * @param str2 The second null-terminated string.
 * @return < 0 if str1 < str2, 0 if str1 == str2, > 0 if str1 > str2.
 */
i32 mdStringCompare(const char* str1, const char* str2);

#if __cppplus
}
#endif