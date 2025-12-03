#include "MEEDEngine/platforms/memory.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#if MD_DEBUG
#if PLATFORM_IS_LINUX
#include <execinfo.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#elif PLATFORM_IS_WINDOWS
#include <Windows.h>
#else
#error "Backtrace capturing is not implemented for this platform."
#endif

/**
 * The specific linked list node which is used only for tracking the allocated memory blocks.
 * The `linked list` itself is implemented in other files, this linked list is used for this file only.
 */
struct MemoryNode
{
	void*			   ptr;		  ///< Store the pointer address for later checking the freed memory.
	mdSize			   size;	  ///< Store the size of the allocated memory block for later checking.
	struct MdTraceInfo traceInfo; ///< The trace information when the memory was allocated.

	struct MemoryNode* pNext;
	struct MemoryNode* pPrev;
};
/**
 * Must be tracked to avoid multiple initializations and non-initializations.
 */
static b8 s_isInitialized = MD_FALSE;

/**
 * The needed information for the linked list head and tail pointers.
 */
static struct MemoryNode* s_pMemoryHead = MD_NULL;
static struct MemoryNode* s_pMemoryTail = MD_NULL;

/**
 * Be modified by the `_malloc` and `_free` functions, just be used for
 * debugging purposes to track the total allocated memory size.
 */
static mdSize s_totalAllocatedMemory = 0;

/**
 * Internal function for allocating memory without tracking but adding the allocation size to the total allocated
 * memory.
 * @param size The size of memory to allocate in bytes.
 * @return A pointer to the allocated memory block.
 */
static void* _malloc(mdSize size);

/**
 * Internal function for freeing memory without tracking but subtracting the allocation size from the total allocated
 * memory.
 * @param ptr A pointer to the memory block to free.
 * @param size The size of memory to free in bytes.
 */
static void _free(void* ptr, mdSize size);

void mdMemoryInitialize()
{
	MD_ASSERT(s_isInitialized == MD_FALSE);
	MD_ASSERT(s_totalAllocatedMemory == 0);
	MD_ASSERT(s_pMemoryHead == MD_NULL);
	MD_ASSERT(s_pMemoryTail == MD_NULL);

	// Initialize code below this line if needed in the future.

	// Initialization complete.

	s_isInitialized = MD_TRUE;
}

void* mdMalloc(mdSize size)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);

	struct MemoryNode* pNode = (struct MemoryNode*)_malloc(sizeof(struct MemoryNode));
	mdMemorySet(pNode, 0, sizeof(struct MemoryNode));

	pNode->ptr	= _malloc(size);
	pNode->size = size;

#if PLATFORM_IS_LINUX
	mdMemorySet(pNode->traceInfo.frames, 0, sizeof(void*) * MD_MAX_TRACE_FRAMES);
	pNode->traceInfo.framesCount = backtrace(pNode->traceInfo.frames, MD_MAX_TRACE_FRAMES);
	pNode->traceInfo.threadId	 = getpid();
#endif

	if (s_pMemoryHead == MD_NULL)
	{
		s_pMemoryHead = pNode;
		pNode->pNext  = MD_NULL;
	}
	else
	{
		s_pMemoryTail->pNext = pNode;
	}

	pNode->pPrev  = s_pMemoryTail;
	s_pMemoryTail = pNode;

	MD_ASSERT(pNode->ptr != MD_NULL);
	return pNode->ptr;
}

static struct MemoryNode* _findNodeByPtr(void* ptr);

void mdFree(void* ptr, mdSize size)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);

	struct MemoryNode* pNode = _findNodeByPtr(ptr);
	MD_ASSERT_MSG(pNode != MD_NULL, "Attempting to free untracked or already freed memory at address %p.", ptr);

	MD_ASSERT_MSG(pNode->size == size,
				  "Freeing memory size mismatch at address %p: expected %zu bytes, got %zu bytes.",
				  ptr,
				  pNode->size,
				  size);

	_free(ptr, pNode->size); // Note: size should be tracked and passed here for accurate memory tracking.

	// Remove the node from the linked list.
	if (pNode->pPrev != MD_NULL)
	{
		pNode->pPrev->pNext = pNode->pNext;
	}
	else
	{
		s_pMemoryHead = pNode->pNext;
	}

	if (pNode->pNext != MD_NULL)
	{
		pNode->pNext->pPrev = pNode->pPrev;
	}
	else
	{
		s_pMemoryTail = pNode->pPrev;
	}

	_free(pNode, sizeof(struct MemoryNode));
}

void* mdMemoryCopy(void* pDest, const void* pSrc, mdSize size)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);
	return memcpy(pDest, pSrc, size);
}

void* mdMemorySet(void* pDest, u8 value, mdSize size)
{
	MD_ASSERT(s_isInitialized == MD_TRUE);
	return memset(pDest, value, size);
}

void mdMemoryShutdown()
{
	MD_ASSERT(s_isInitialized == MD_TRUE);

	// Shutdown code below this line if needed in the future.
	if (s_pMemoryHead != MD_NULL)
	{
		mdPrintTrace(&s_pMemoryHead->traceInfo);
#if PLATFORM_IS_LINUX
		exit(139); // 139 is the exit code for segmentation fault.
#elif PLATFORM_IS_WINDOWS
		exit(139);
#else
#error "Memory leak handling exit code is not implemented for this platform."
#endif
	}

	// Shutdown complete.

	MD_ASSERT_MSG(s_totalAllocatedMemory == 0,
				  "Memory leak detected: Total allocated memory is %zu bytes during shutdown.",
				  s_totalAllocatedMemory);
}

static void* _malloc(mdSize size)
{
	s_totalAllocatedMemory += size;
	return malloc(size);
}

static void _free(void* ptr, mdSize size)
{
	s_totalAllocatedMemory -= size;
	free(ptr);
}

static struct MemoryNode* _findNodeByPtr(void* ptr)
{
	struct MemoryNode* pCurrent = s_pMemoryHead;
	while (pCurrent != MD_NULL)
	{
		if (pCurrent->ptr == (void*)ptr)
		{
			return pCurrent;
		}
		pCurrent = pCurrent->pNext;
	}

	return MD_NULL;
}

#else // MD_RELEASE

void mdMemoryInitialize()
{
	// No-op in release mode.
}

void* mdMalloc(mdSize size)
{
	return malloc(size);
}

void mdFree(void* ptr, mdSize size)
{
	(void)size; // Unused parameter in release mode.
	free(ptr);
}

void* mdMemoryCopy(void* pDest, const void* pSrc, mdSize size)
{
	return memcpy(pDest, pSrc, size);
}

void* mdMemorySet(void* pDest, u8 value, mdSize size)
{
	return memset(pDest, value, size);
}

void mdMemoryShutdown()
{
	// No-op in release mode.
}

#endif // MD_DEBUG