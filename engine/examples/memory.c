#include "MEEDEngine/MEEDEngine.h"

int main()
{
	meedPlatformMemoryInitialize();

	void* ptr = meedPlatformMalloc(128);
	MEED_ASSERT_MSG(ptr != MEED_NULL, "Memory allocation failed!");

	meedPlatformFree(ptr, 128);

	void* ptr2 = meedPlatformMalloc(256);
	void* ptr3 = meedPlatformMalloc(512);

	meedPlatformFree(ptr2, 256);
	meedPlatformFree(ptr3, 512);

	meedPlatformMemoryShutdown();
	return 0;
}