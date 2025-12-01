#pragma once
#if MEED_DEBUG
#include <assert.h>
#endif

#define MEED_PATH_SEPARATOR		'/'
#define MEED_PATH_SEPARATOR_STR "/"

#if PLATFORM_IS_WINDOWS
#define debugBreak() __debugbreak()
#else
#include <signal.h>
#define debugBreak() __builtin_trap()
#endif

#if PLATFORM_IS_WEB
#include <emscripten/emscripten.h>
#endif

#if MEED_DEBUG
#define MEED_ASSERT(expr)                                                                                              \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!(expr))                                                                                                   \
		{                                                                                                              \
			struct MEEDPlatformConsoleConfig config;                                                                   \
			config.color = MEED_CONSOLE_COLOR_RED;                                                                     \
			meedPlatformSetConsoleConfig(config);                                                                      \
			meedPlatformFPrint("Assertion failed: %s at %s:%d\n", #expr, __FILE__, __LINE__);                          \
			config.color = MEED_CONSOLE_COLOR_RESET;                                                                   \
			meedPlatformSetConsoleConfig(config);                                                                      \
			debugBreak();                                                                                              \
		}                                                                                                              \
	} while (0)

#define MEED_ASSERT_MSG(expr, msg, ...)                                                                                \
	do                                                                                                                 \
	{                                                                                                                  \
		if (!(expr))                                                                                                   \
		{                                                                                                              \
			struct MEEDPlatformConsoleConfig config;                                                                   \
			config.color = MEED_CONSOLE_COLOR_RED;                                                                     \
			meedPlatformSetConsoleConfig(config);                                                                      \
			char buffer[512];                                                                                          \
			meedPlatformBufferedPrint(buffer, sizeof(buffer), msg, ##__VA_ARGS__);                                     \
			meedPlatformFPrint("Assertion failed: %s at %s:%d\n", buffer, __FILE__, __LINE__);                         \
			config.color = MEED_CONSOLE_COLOR_RESET;                                                                   \
			meedPlatformSetConsoleConfig(config);                                                                      \
			debugBreak();                                                                                              \
		}                                                                                                              \
	} while (0)

// Macro to mark a variable as untouchable for debugging purposes.
#define MEED_UNTOUCHABLE() MEED_ASSERT_MSG(MEED_FALSE, "This code path should be unreachable!")

#else
#define MEED_ASSERT(expr)
#define MEED_ASSERT_MSG(expr, msg, ...)
#define MEED_UNTOUCHABLE()
#endif

/**
 * Macro to mark a variable as unused to avoid compiler warnings.
 */
#define MEED_UNUSED(x) (void)(x)

/**
 * This macro is used for marking that the symbol is intended for binding generation (python)
 */
#if defined(__clang__)
#define MEED_BINDING __attribute__((annotate("binding")))
#define MEED_HIDDEN	 __attribute__((annotate("hidden")))
#else
#define MEED_BINDING
#define MEED_HIDDEN
#endif

#if defined(__EMSCRIPTEN__)
#define PLATFORM_API __attribute__((visibility("default"))) __attribute__((used))
#else
#define PLATFORM_API
#endif

// =================== Types defined ===================

typedef MEED_BINDING unsigned char		u8;
typedef MEED_BINDING unsigned short		u16;
typedef MEED_BINDING unsigned int		u32;
typedef MEED_BINDING unsigned long long u64;

#if MEED_DEBUG
static_assert(sizeof(u8) == 1, "u8 size is not 1 byte");
static_assert(sizeof(u16) == 2, "u16 size is not 2 bytes");
static_assert(sizeof(u32) == 4, "u32 size is not 4 bytes");
static_assert(sizeof(u64) == 8, "u64 size is not 8 bytes");
#endif

typedef MEED_BINDING char	   i8;
typedef MEED_BINDING short	   i16;
typedef MEED_BINDING int	   i32;
typedef MEED_BINDING long long i64;

#if MEED_DEBUG
static_assert(sizeof(i8) == 1, "i8 size is not 1 byte");
static_assert(sizeof(i16) == 2, "i16 size is not 2 bytes");
static_assert(sizeof(i32) == 4, "i32 size is not 4 bytes");
static_assert(sizeof(i64) == 8, "i64 size is not 8 bytes");
#endif

typedef MEED_BINDING float	f32;
typedef MEED_BINDING double f64;

#if MEED_DEBUG
static_assert(sizeof(f32) == 4, "f32 size is not 4 bytes");
static_assert(sizeof(f64) == 8, "f64 size is not 8 bytes");
#endif

typedef MEED_BINDING unsigned char b8;
typedef MEED_BINDING unsigned int  b32;

#if MEED_DEBUG
static_assert(sizeof(b8) == 1, "b8 size is not 1 byte");
static_assert(sizeof(b32) == 4, "b32 size is not 4 bytes");
#endif

typedef MEED_BINDING size_t meedSize;

#define MEED_NULL ((void*)0)

#define MEED_TRUE  ((b8)1)
#define MEED_FALSE ((b8)0)

#define MEED_ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

/**
 * Convert predefined macro to c string.
 *
 * @example
 * ```c
 * STRINGIFY(PROJECT_BASE_DIR) -> "/path/to/project"
 * ```
 */
#define MEED_STRINGIFY(x)  _MEED_STRINGIFY(x)
#define _MEED_STRINGIFY(x) #x

#include "console.h"
#include "exceptions.h"
#include "memory.h"