#include "MEEDEngine/core/containers/set.h"

struct MEEDSet* meedSetCreate(MEEDSetCompareCallback pCompareCallback)
{
	MEED_ASSERT(pCompareCallback != MEED_NULL);

	struct MEEDSet* pSet = MEED_MALLOC(struct MEEDSet);
	MEED_ASSERT(pSet != MEED_NULL);

	pSet->pList = meedLinkedListCreate(MEED_NULL);
	MEED_ASSERT(pSet->pList != MEED_NULL);

	pSet->pCompareCallback = pCompareCallback;

	return pSet;
}

u32 meedSetCount(struct MEEDSet* pSet)
{
	MEED_ASSERT(pSet != MEED_NULL);
	return meedLinkedListCount(pSet->pList);
}

void meedSetPush(struct MEEDSet* pSet, void* pData)
{
	MEED_ASSERT(pSet != MEED_NULL);
	MEED_ASSERT(pSet->pCompareCallback != MEED_NULL);
	MEED_ASSERT(pSet->pList != MEED_NULL);

	if (pData == MEED_NULL)
	{
		MEED_THROW(MEED_EXCEPTION_TYPE_INVALID_OPERATION, "Cannot insert NULL data into the set.");
	}

	struct MEEDLinkedListNode* pCurrent = pSet->pList->pHead;
	u32						   index	= 0;

	while (pCurrent != MEED_NULL)
	{
		i32						   cmpResult = pSet->pCompareCallback(pCurrent->pData, pData);
		struct MEEDLinkedListNode* pNextNode = pCurrent->pNext;

		if (cmpResult == 0)
		{
			return;
		}
		else if (cmpResult > 0)
		{
			meedLinkedListInsert(pSet->pList, index, pData);
			return;
		}
		else
		{
			pCurrent = pNextNode;
			index++;
		}
	}

	meedLinkedListPush(pSet->pList, pData);
}

void* meedSetAt(struct MEEDSet* pSet, u32 index)
{
	MEED_ASSERT(pSet != MEED_NULL);
	MEED_ASSERT(pSet->pList != MEED_NULL);

	return meedLinkedListAt(pSet->pList, index);
}

void meedSetErase(struct MEEDSet* pSet, u32 index)
{
	MEED_ASSERT(pSet != MEED_NULL);
	MEED_ASSERT(pSet->pList != MEED_NULL);

	meedLinkedListErase(pSet->pList, index);
}

void meedSetClear(struct MEEDSet* pSet)
{
	MEED_ASSERT(pSet != MEED_NULL);
	MEED_ASSERT(pSet->pList != MEED_NULL);

	meedLinkedListClear(pSet->pList);
}

u32 meedSetFind(struct MEEDSet* pSet, void* pData)
{
	MEED_ASSERT(pSet != MEED_NULL);
	MEED_ASSERT(pSet->pCompareCallback != MEED_NULL);
	MEED_ASSERT(pSet->pList != MEED_NULL);

	struct MEEDLinkedListNode* pCurrent = pSet->pList->pHead;
	u32						   index	= 0;

	while (pCurrent != MEED_NULL)
	{
		i32						   cmpResult = pSet->pCompareCallback(pCurrent->pData, pData);
		struct MEEDLinkedListNode* pNextNode = pCurrent->pNext;

		if (cmpResult == 0)
		{
			return index;
		}
		else if (cmpResult > 0)
		{
			break;
		}
		else
		{
			pCurrent = pNextNode;
			index++;
		}
	}

	return MEED_SET_NOT_FOUND_INDEX;
}

void meedSetDestroy(struct MEEDSet* pSet)
{
	MEED_ASSERT(pSet != MEED_NULL);

	meedLinkedListDestroy(pSet->pList);
	MEED_FREE(pSet, struct MEEDSet);
}