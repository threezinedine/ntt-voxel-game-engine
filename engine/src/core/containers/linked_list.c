#include "MEEDEngine/core/containers/linked_list.h"

struct MEEDLinkedList* meedLinkedListCreate(MEEDLinkedListDeleteCallback callback)
{
	struct MEEDLinkedList* pList = MEED_MALLOC(struct MEEDLinkedList);
	MEED_ASSERT(pList != MEED_NULL);

	pList->size			   = 0;
	pList->pHead		   = MEED_NULL;
	pList->pTail		   = MEED_NULL;
	pList->pDeleteCallback = callback;

	return pList;
}

void meedLinkedListPush(struct MEEDLinkedList* pList, void* pData)
{
	MEED_ASSERT(pList != MEED_NULL);

	struct MEEDLinkedListNode* pNewNode = MEED_MALLOC(struct MEEDLinkedListNode);
	MEED_ASSERT(pNewNode != MEED_NULL);

	pNewNode->pData = pData;
	pNewNode->pNext = MEED_NULL;

	if (pList->pTail != MEED_NULL)
	{
		pList->pTail->pNext = pNewNode;
	}
	else
	{
		pList->pHead = pNewNode;
	}

	pList->pTail = pNewNode;
	pList->size++;
}

u32 meedLinkedListGetCount(struct MEEDLinkedList* pList)
{
	MEED_ASSERT(pList != MEED_NULL);
	return (u32)(pList->size);
}

b8 meedLinkedListEmpty(struct MEEDLinkedList* pList)
{
	MEED_ASSERT(pList != MEED_NULL);
	return (pList->size == 0) ? MEED_TRUE : MEED_FALSE;
}

void* meedLinkedListAt(struct MEEDLinkedList* pList, u32 index)
{
	MEED_ASSERT(pList != MEED_NULL);

	if (index >= pList->size)
	{
		MEED_THROW(MEED_EXCEPTION_TYPE_OUT_OF_INDEX,
				   "Index out of bounds: Attempted to access index %u in a linked list of size %u.",
				   index,
				   pList->size);
	}

	struct MEEDLinkedListNode* pCurrent = pList->pHead;
	for (u32 i = 0; i < index; i++)
	{
		pCurrent = pCurrent->pNext;
	}

	MEED_ASSERT(pCurrent != MEED_NULL);
	return pCurrent->pData;
}

void meedLinkedListDestroy(struct MEEDLinkedList* pList)
{
	MEED_ASSERT(pList != MEED_NULL);

	struct MEEDLinkedListNode* pCurrent = pList->pHead;

	while (pCurrent != MEED_NULL)
	{
		struct MEEDLinkedListNode* pNext = pCurrent->pNext;

		MEED_ASSERT(pCurrent != MEED_NULL);

		if (pList->pDeleteCallback != MEED_NULL)
		{
			pList->pDeleteCallback(pCurrent->pData);
		}

		MEED_FREE(pCurrent, struct MEEDLinkedListNode);
		pCurrent = pNext;
	}

	MEED_FREE(pList, struct MEEDLinkedList);
}