#pragma once
#include "common.h"

#if __cplusplus
extern "C" {
#endif

/**
 * @file memory.h
 * The utilities for managing the memory operations inside the `MEEDEngine`.
 */

/**
 * Must call this function before using any memory-related functions.
 * This function starts the memory management system.
 */
void meedPlatformMemoryInitialize();

/**
 * Allocates a block of memory of the specified size.
 *
 * @param size The size of memory to allocate in bytes.
 * @return A pointer to the allocated memory block, or nullptr if allocation fails.
 */
void* meedPlatformMalloc(meedSize size);

/**
 * Frees a previously allocated block of memory.
 *
 * @param ptr A pointer to the memory block to free.
 * @param size The size of memory to free in bytes (used for tracking purposes).
 */
void meedPlatformFree(void* ptr, meedSize size);

/**
 * Copies a block of memory from a source to a destination.
 *
 * @param pDest A pointer to the destination memory block.
 * @param pSrc A pointer to the source memory block.
 * @param size The number of bytes to copy.
 * @return A pointer to the destination memory block.
 */
void* meedPlatformMemoryCopy(void* pDest, const void* pSrc, meedSize size);

/**
 * Sets a block of memory to a specified value.
 *
 * @param pDest A pointer to the memory block to set.
 * @param value The value to set each byte of the memory block to.
 * @param size The number of bytes to set.
 * @return A pointer to the memory block.
 */
void* meedPlatformMemorySet(void* pDest, u8 value, meedSize size);

/**
 * Gets the length of a null-terminated string.
 * @param str The null-terminated string.
 * @return The length of the string, excluding the null terminator.
 */
u32 meedPlatformGetStringLength(const char* str);

/**
 * Cleans up the memory management system.
 * This function should be called when memory operations are no longer needed.
 * In `DEBUG` mode, it may also check for memory leaks.
 */
void meedPlatformMemoryShutdown();

/**
 * Helper macro to allocate memory for a specific type.
 * @param type The type of the object to allocate memory for.
 * @return A pointer to the allocated memory cast to the specified type.
 */
#define MEED_MALLOC(type) (type*)meedPlatformMalloc(sizeof(type))

/**
 * Helper macro to allocate memory for an array of a specific type.
 * @param type The type of the objects in the array.
 * @param count The number of objects to allocate memory for.
 * @return A pointer to the allocated memory cast to the specified type.
 */
#define MEED_MALLOC_ARRAY(type, count) (type*)meedPlatformMalloc(sizeof(type) * (count))

/**
 * Helper macro to free memory allocated for a specific type.
 * @param ptr A pointer to the memory block to free.
 */
#define MEED_FREE(ptr, type) meedPlatformFree((void*)(ptr), sizeof(type))

/**
 * Helper macro to free memory allocated for an array of a specific type.
 * @param ptr A pointer to the memory block to free.
 */
#define MEED_FREE_ARRAY(ptr, type, count) meedPlatformFree((void*)(ptr), sizeof(type) * (count))

#if __cplusplus
}
#endif