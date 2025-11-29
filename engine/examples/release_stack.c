#include "MEEDEngine/MEEDEngine.h"

static void releaseInt(void* pData)
{
	meedPlatformFPrint("Releasing integer: %d\n", *(int*)pData);
}

int main(void)
{
	meedPlatformMemoryInitialize();

	int a = 3;
	int b = 4;

	struct MEEDReleaseStack* pReleaseStack = meedReleaseStackCreate();

	meedReleaseStackPush(pReleaseStack, &a, releaseInt);
	meedReleaseStackPush(pReleaseStack, &b, releaseInt);

	meedReleaseStackDestroy(pReleaseStack);
	meedPlatformMemoryShutdown();
	return 0;
}