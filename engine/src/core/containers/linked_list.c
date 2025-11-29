#include "MEEDEngine/core/containers/linked_list.h"

struct MEEDLinkedList* meedLinkedListCreate(MEEDNodeDataDeleteCallback pDeleteCallback)
{
	struct MEEDLinkedList* pList = MEED_MALLOC(struct MEEDLinkedList);
	MEED_ASSERT(pList != MEED_NULL);

	pList->size			   = 0;
	pList->pHead		   = MEED_NULL;
	pList->pTail		   = MEED_NULL;
	pList->pDeleteCallback = pDeleteCallback;

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

void meedLinkedListInsert(struct MEEDLinkedList* pList, u32 index, void* pData)
{
	MEED_ASSERT(pList != MEED_NULL);

	if (index > pList->size)
	{
		MEED_THROW(MEED_EXCEPTION_TYPE_OUT_OF_INDEX,
				   "Index out of bounds: Attempted to insert at index %u in a linked list of size %u.",
				   index,
				   pList->size);
	}

	struct MEEDLinkedListNode* pNewNode = MEED_MALLOC(struct MEEDLinkedListNode);
	MEED_ASSERT(pNewNode != MEED_NULL);

	pNewNode->pData = pData;
	pNewNode->pNext = MEED_NULL;

	if (index == pList->size)
	{
		// Insert at the end
		if (pList->pTail != MEED_NULL)
		{
			pList->pTail->pNext = pNewNode;
		}
		else
		{
			pList->pHead = pNewNode;
		}
		pList->pTail = pNewNode;
	}
	else if (index == 0)
	{
		// Insert at the beginning
		pNewNode->pNext = pList->pHead;
		pList->pHead	= pNewNode;
		if (pList->pTail == MEED_NULL)
		{
			pList->pTail = pNewNode;
		}
	}
	else
	{
		// Insert in the middle
		struct MEEDLinkedListNode* pCurrent = pList->pHead;
		for (u32 i = 0; i < index - 1; i++)
		{
			pCurrent = pCurrent->pNext;
		}
		pNewNode->pNext = pCurrent->pNext;
		pCurrent->pNext = pNewNode;
	}

	pList->size++;
}

void meedLinkedListErase(struct MEEDLinkedList* pList, u32 index)
{
	MEED_ASSERT(pList != MEED_NULL);

	if (index >= pList->size)
	{
		MEED_THROW(MEED_EXCEPTION_TYPE_OUT_OF_INDEX,
				   "Index out of bounds: Attempted to erase index %u in a linked list of size %u.",
				   index,
				   pList->size);
	}

	struct MEEDLinkedListNode* pCurrent = pList->pHead;
	struct MEEDLinkedListNode* pPrev	= MEED_NULL;

	for (u32 i = 0; i < index; i++)
	{
		pPrev	 = pCurrent;
		pCurrent = pCurrent->pNext;
	}

	MEED_ASSERT(pCurrent != MEED_NULL);

	if (pPrev != MEED_NULL)
	{
		pPrev->pNext = pCurrent->pNext;
	}
	else
	{
		pList->pHead = pCurrent->pNext;
	}

	if (pCurrent == pList->pTail)
	{
		pList->pTail = pPrev;
	}

	if (pList->pDeleteCallback != MEED_NULL)
	{
		pList->pDeleteCallback(pCurrent->pData);
	}

	MEED_FREE(pCurrent, struct MEEDLinkedListNode);
	pList->size--;
}

u32 meedLinkedListCount(struct MEEDLinkedList* pList)
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

void meedLinkedListClear(struct MEEDLinkedList* pList)
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

	pList->pHead = MEED_NULL;
	pList->pTail = MEED_NULL;
	pList->size	 = 0;
}

void meedLinkedListDestroy(struct MEEDLinkedList* pList)
{
	MEED_ASSERT(pList != MEED_NULL);

	meedLinkedListClear(pList);

	MEED_FREE(pList, struct MEEDLinkedList);
}