#include "MEEDEngine/core/containers/stack.h"

struct MEEDStack* meedStackCreate(MEEDNodeDataDeleteCallback pDeleteCallback)
{
	struct MEEDStack* pStack = MEED_MALLOC(struct MEEDStack);
	MEED_ASSERT(pStack != MEED_NULL);

	pStack->pList = meedLinkedListCreate(pDeleteCallback);
	MEED_ASSERT(pStack->pList != MEED_NULL);

	return pStack;
}

b8 meedStackEmpty(struct MEEDStack* pStack)
{
	MEED_ASSERT(pStack != MEED_NULL);
	return meedLinkedListEmpty(pStack->pList);
}

u32 meedStackGetCount(struct MEEDStack* pStack)
{
	MEED_ASSERT(pStack != MEED_NULL);
	return meedLinkedListGetCount(pStack->pList);
}

void meedStackPush(struct MEEDStack* pStack, void* pData)
{
	MEED_ASSERT(pStack != MEED_NULL);
	meedLinkedListPush(pStack->pList, pData);
}

void* meedStackTop(struct MEEDStack* pStack)
{
	MEED_ASSERT(pStack != MEED_NULL);
	MEED_ASSERT(meedStackEmpty(pStack) == MEED_FALSE);

	u32 count = meedStackGetCount(pStack);
	return meedLinkedListAt(pStack->pList, count - 1);
}

void meedStackPop(struct MEEDStack* pStack)
{
	MEED_ASSERT(pStack != MEED_NULL);

	if (meedStackEmpty(pStack) == MEED_TRUE)
	{
		MEED_THROW(MEED_EXCEPTION_TYPE_EMPTY_CONTAINER, "Attempted to pop from an empty stack.");
	}

	u32 count = meedStackGetCount(pStack);
	meedLinkedListErase(pStack->pList, count - 1);
}

void meedStackClear(struct MEEDStack* pStack)
{
	MEED_ASSERT(pStack != MEED_NULL);

	while (meedStackEmpty(pStack) == MEED_FALSE)
	{
		meedStackPop(pStack);
	}
}

void meedStackDestroy(struct MEEDStack* pStack)
{
	MEED_ASSERT(pStack != MEED_NULL);

	meedLinkedListDestroy(pStack->pList);
	MEED_FREE(pStack, struct MEEDStack);
}