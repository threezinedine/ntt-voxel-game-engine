#include "MEEDEngine/modules/release_stack/release_stack.h"
#include "MEEDEngine/core/containers/stack.h"

static void releaseStackItemDestroy(void* pData)
{
	MEED_ASSERT(pData != MEED_NULL);
	struct MEEDReleaseStackItem* pItem = (struct MEEDReleaseStackItem*)pData;

	if (pItem->pReleaseFunc != MEED_NULL)
	{
		pItem->pReleaseFunc(pItem->pData);
	}

	MEED_FREE(pItem, struct MEEDReleaseStackItem);
}

struct MEEDReleaseStack* meedReleaseStackCreate()
{
	struct MEEDReleaseStack* pReleaseStack = MEED_MALLOC(struct MEEDReleaseStack);
	MEED_ASSERT(pReleaseStack != MEED_NULL);

	pReleaseStack->pStack = meedStackCreate(releaseStackItemDestroy);
	return pReleaseStack;
}

void meedReleaseStackPush(struct MEEDReleaseStack* pReleaseStack, void* pData, MEEDReleaseFunc pReleaseFunc)
{
	MEED_ASSERT(pReleaseStack != MEED_NULL);
	MEED_ASSERT(pReleaseStack->pStack != MEED_NULL);
	MEED_ASSERT(pReleaseFunc != MEED_NULL);

	struct MEEDReleaseStackItem* pItem = MEED_MALLOC(struct MEEDReleaseStackItem);
	MEED_ASSERT(pItem != MEED_NULL);

	pItem->pData		= pData;
	pItem->pReleaseFunc = pReleaseFunc;

	meedStackPush(pReleaseStack->pStack, pItem);
}

void meedReleaseStackDestroy(struct MEEDReleaseStack* pReleaseStack)
{
	MEED_ASSERT(pReleaseStack != MEED_NULL);

	while (meedStackEmpty(pReleaseStack->pStack) == MEED_FALSE)
	{
		meedStackPop(pReleaseStack->pStack);
	}

	meedStackDestroy(pReleaseStack->pStack);
	MEED_FREE(pReleaseStack, struct MEEDReleaseStack);
}