#include "MEEDEngine/MEEDEngine.h"

int main(void)
{
	meedPlatformMemoryInitialize();

	struct MEEDLinkedList* pList = meedLinkedListCreate(MEED_NULL);

	meedLinkedListPush(pList, (void*)1);

	meedLinkedListDestroy(pList);

	meedPlatformMemoryShutdown();
	return 0;
}