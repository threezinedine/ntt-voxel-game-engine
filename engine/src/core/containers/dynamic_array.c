#include "MEEDEngine/core/containers/dynamic_array.h"

struct MEEDDynamicArray* meedDynamicArrayCreate(u32 initialCapacity, MEEDNodeDataDeleteCallback pDeleteCallback)
{
	struct MEEDDynamicArray* pArray = MEED_MALLOC(struct MEEDDynamicArray);
	MEED_ASSERT(pArray != MEED_NULL);

	if (initialCapacity == 0)
	{
		initialCapacity = MEED_DEFAULT_DYNAMIC_ARRAY_CAPACITY;
	}

	pArray->count	 = 0;
	pArray->capacity = initialCapacity;
	pArray->pData	 = MEED_MALLOC_ARRAY(void*, initialCapacity);
	MEED_ASSERT(pArray->pData != MEED_NULL);
	pArray->pDeleteCallback = pDeleteCallback;

	return pArray;
}

void meedDynamicArrayPush(struct MEEDDynamicArray* pArray, void* pData)
{
	MEED_ASSERT(pArray != MEED_NULL);

	if (pArray->count >= pArray->capacity)
	{
		meedDynamicArrayResize(pArray, pArray->capacity * 2);
	}

	pArray->pData[pArray->count] = pData;
	pArray->count++;
}

void* meedDynamicArrayAt(struct MEEDDynamicArray* pArray, u32 index)
{
	MEED_ASSERT(pArray != MEED_NULL);

	return pArray->pData[index];
}

void meedDynamicArrayResize(struct MEEDDynamicArray* pArray, u32 newCapacity)
{
	MEED_ASSERT(pArray != MEED_NULL);

	if (newCapacity <= pArray->capacity)
	{
		MEED_THROW(MEED_EXCEPTION_TYPE_INVALID_OPERATION,
				   "New capacity %u must be greater than current capacity %u.",
				   newCapacity,
				   pArray->capacity);
	}

	void** pNewData = MEED_MALLOC_ARRAY(void*, newCapacity);
	MEED_ASSERT(pNewData != MEED_NULL);

	meedPlatformMemoryCopy(pNewData, pArray->pData, sizeof(void*) * pArray->count);
	MEED_FREE_ARRAY(pArray->pData, void*, pArray->capacity);

	pArray->pData	 = pNewData;
	pArray->capacity = newCapacity;
}

void meedDynamicArrayDestroy(struct MEEDDynamicArray* pArray)
{
	MEED_ASSERT(pArray != MEED_NULL);

	if (pArray->pDeleteCallback != MEED_NULL)
	{
		for (u32 i = 0; i < pArray->count; i++)
		{
			pArray->pDeleteCallback(pArray->pData[i]);
		}
	}

	MEED_FREE_ARRAY(pArray->pData, void*, pArray->capacity);
	MEED_FREE(pArray, struct MEEDDynamicArray);
}