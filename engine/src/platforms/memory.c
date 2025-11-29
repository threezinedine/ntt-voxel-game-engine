#include "MEEDEngine/platforms/memory.h"
#include <stdint.h>
#include <stdlib.h>

#if PLATFORM_IS_LINUX
#include <execinfo.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#else
#error "Backtrace capturing is not implemented for this platform."
#endif

#define MEMORY_MAX_TRACKED_FRAMES 16

/**
 * The specific linked list node which is used only for tracking the allocated memory blocks.
 * The `linked list` itself is implemented in other files, this linked list is used for this file only.
 */
struct MemoryNode
{
	void*	 ptr;								///< Store the pointer address for later checking the freed memory.
	meedSize size;								///< Store the size of the allocated memory block for later checking.
	void*	 frames[MEMORY_MAX_TRACKED_FRAMES]; ///< The backtrace frames when the memory was allocated.
	meedSize framesCount;						///< The number of frames stored in the `frames` array.
	pid_t	 threadId;							///< The ID of the thread which allocated this memory block.

	struct MemoryNode* pNext;
	struct MemoryNode* pPrev;
};
/**
 * Must be tracked to avoid multiple initializations and non-initializations.
 */
static b8 s_isInitialized = MEED_FALSE;

/**
 * The needed information for the linked list head and tail pointers.
 */
static struct MemoryNode* s_pMemoryHead = MEED_NULL;
static struct MemoryNode* s_pMemoryTail = MEED_NULL;

/**
 * Be modified by the `_malloc` and `_free` functions, just be used for
 * debugging purposes to track the total allocated memory size.
 */
static meedSize s_totalAllocatedMemory = 0;

/**
 * Internal function for allocating memory without tracking but adding the allocation size to the total allocated
 * memory.
 * @param size The size of memory to allocate in bytes.
 * @return A pointer to the allocated memory block.
 */
static void* _malloc(meedSize size);

/**
 * Internal function for freeing memory without tracking but subtracting the allocation size from the total allocated
 * memory.
 * @param ptr A pointer to the memory block to free.
 * @param size The size of memory to free in bytes.
 */
static void _free(void* ptr, meedSize size);

void meedPlatformMemoryInitialize()
{
	MEED_ASSERT(s_isInitialized == MEED_FALSE);
	MEED_ASSERT(s_totalAllocatedMemory == 0);
	MEED_ASSERT(s_pMemoryHead == MEED_NULL);
	MEED_ASSERT(s_pMemoryTail == MEED_NULL);

	// Initialize code below this line if needed in the future.

	// Initialization complete.

	s_isInitialized = MEED_TRUE;
}

void* meedPlatformMalloc(meedSize size)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);

	struct MemoryNode* pNode = (struct MemoryNode*)_malloc(sizeof(struct MemoryNode));
	meedPlatformMemorySet(pNode, 0, sizeof(struct MemoryNode));

	pNode->ptr	= _malloc(size);
	pNode->size = size;

#if PLATFORM_IS_LINUX
	meedPlatformMemorySet(pNode->frames, 0, sizeof(void*) * MEMORY_MAX_TRACKED_FRAMES);
	pNode->framesCount = backtrace(pNode->frames, MEMORY_MAX_TRACKED_FRAMES);
	pNode->threadId	   = getpid();
#endif

	if (s_pMemoryHead == MEED_NULL)
	{
		s_pMemoryHead = pNode;
		pNode->pNext  = MEED_NULL;
	}
	else
	{
		s_pMemoryTail->pNext = pNode;
	}

	pNode->pPrev  = s_pMemoryTail;
	s_pMemoryTail = pNode;

	MEED_ASSERT(pNode->ptr != MEED_NULL);
	return pNode->ptr;
}

static struct MemoryNode* _findNodeByPtr(void* ptr);

void meedPlatformFree(void* ptr, meedSize size)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);

	struct MemoryNode* pNode = _findNodeByPtr(ptr);
	MEED_ASSERT_MSG(pNode != MEED_NULL, "Attempting to free untracked or already freed memory at address %p.", ptr);

	MEED_ASSERT_MSG(pNode->size == size,
					"Freeing memory size mismatch at address %p: expected %zu bytes, got %zu bytes.",
					ptr,
					pNode->size,
					size);

	_free(ptr, pNode->size); // Note: size should be tracked and passed here for accurate memory tracking.

	// Remove the node from the linked list.
	if (pNode->pPrev != MEED_NULL)
	{
		pNode->pPrev->pNext = pNode->pNext;
	}
	else
	{
		s_pMemoryHead = pNode->pNext;
	}

	if (pNode->pNext != MEED_NULL)
	{
		pNode->pNext->pPrev = pNode->pPrev;
	}
	else
	{
		s_pMemoryTail = pNode->pPrev;
	}

	_free(pNode, sizeof(struct MemoryNode));
}

void* meedPlatformMemoryCopy(void* pDest, const void* pSrc, meedSize size)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);
	return memcpy(pDest, pSrc, size);
}

void* meedPlatformMemorySet(void* pDest, u8 value, meedSize size)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);
	return memset(pDest, value, size);
}

u32 meedPlatformGetStringLength(const char* str)
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);
	MEED_ASSERT(str != MEED_NULL);

	u32 length = 0;
	while (str[length] != '\0')
	{
		length++;
	}

	return length;
}

void meedPlatformMemoryShutdown()
{
	MEED_ASSERT(s_isInitialized == MEED_TRUE);

	// Shutdown code below this line if needed in the future.
	if (s_pMemoryHead != MEED_NULL)
	{
#if PLATFORM_IS_LINUX
		struct MEEDPlatformConsoleConfig config;
		config.color = MEED_CONSOLE_COLOR_RED;
		meedPlatformSetConsoleConfig(config);

		meedPlatformFPrint("=== Memory Leak Stack Trace: ===\n");
		char cmd[512];
		snprintf(cmd, sizeof(cmd), "addr2line -e /proc/%d/exe -pif", s_pMemoryHead->threadId);
		for (meedSize i = 0; i < s_pMemoryHead->framesCount; i++)
		{
#if 0
			char syscom[1024];
			snprintf(syscom, sizeof(syscom), "%s %p", cmd, s_pMemoryHead->frames[i]);
			char** function = backtrace_symbols(&s_pMemoryHead->frames[i], 1);
			meedPlatformPrint(function[0]);
			meedPlatformPrint("\n");
			system(syscom);
#else
			char** function = backtrace_symbols(&s_pMemoryHead->frames[i], 1);
			meedPlatformPrint(function[0]);
			meedPlatformPrint("\n");
#endif
		}
		exit(139); // 139 is the exit code for segmentation fault.
		config.color = MEED_CONSOLE_COLOR_RESET;
		meedPlatformSetConsoleConfig(config);
#else
		MEED_UNTOUCHABLE();
#endif
	}

	// Shutdown complete.

	MEED_ASSERT_MSG(s_totalAllocatedMemory == 0,
					"Memory leak detected: Total allocated memory is %zu bytes during shutdown.",
					s_totalAllocatedMemory);
}

static void* _malloc(meedSize size)
{
	s_totalAllocatedMemory += size;
	return malloc(size);
}

static void _free(void* ptr, meedSize size)
{
	s_totalAllocatedMemory -= size;
	free(ptr);
}

static struct MemoryNode* _findNodeByPtr(void* ptr)
{
	struct MemoryNode* pCurrent = s_pMemoryHead;
	while (pCurrent != MEED_NULL)
	{
		if (pCurrent->ptr == (void*)ptr)
		{
			return pCurrent;
		}
		pCurrent = pCurrent->pNext;
	}

	return MEED_NULL;
}