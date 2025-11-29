#pragma once
#include "common.h"

#if __cplusplus
extern "C" {
#endif

/**
 * @file exceptions.h
 * The utilities for managing exceptions inside the `MEEDEngine`.
 */
enum MEEDExceptionType
{
	MEED_EXCEPTION_TYPE_SUCCESS = 0,
	MEED_EXCEPTION_TYPE_OUT_OF_INDEX, ///< Raised when an index is out of bounds of a container.
};

/**
 * Raise an exception of the specified type with a message, file, and line number.
 * @param type The type of exception to raise.
 * @param message The message associated with the exception.
 * @param file The file where the exception occurred.
 * @param line The line number where the exception occurred.
 */
void meedRaiseException(enum MEEDExceptionType type, const char* message, const char* file, u32 line);

/**
 * Macro to raise an exception with automatic file and line number information.
 * @param type The type of exception to raise.
 * @param message The message associated with the exception.
 */
#define MEED_THROW(type, message, ...)                                                                                 \
	do                                                                                                                 \
	{                                                                                                                  \
		char formattedMessage[512];                                                                                    \
		meedPlatformBufferedPrint(formattedMessage, sizeof(formattedMessage), message, ##__VA_ARGS__);                 \
		meedRaiseException(type, formattedMessage, __FILE__, __LINE__);                                                \
	} while (MEED_FALSE);

#if __cplusplus
}
#endif