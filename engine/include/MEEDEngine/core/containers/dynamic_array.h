#pragma once

#if __cplusplus
extern "C" {
#endif

#include "MEEDEngine/platforms/common.h"
#include "callback.h"

#define MEED_DEFAULT_DYNAMIC_ARRAY_CAPACITY 2

/**
 * @file dynamic_array.h
 *
 * Self implemented general dynamic array container.
 */

struct MEEDDynamicArray
{
	u32 count;	  ///< The actual number of elements in the dynamic array.
	u32 capacity; ///< The total capacity of the dynamic array. It will be doubled when the count exceeds the capacity.

	void**					   pData;			///< Pointer to the array of data pointers.
	MEEDNodeDataDeleteCallback pDeleteCallback; ///< Callback function to delete element data.
};

/**
 * @brief Creates and initializes a new dynamic array.
 *
 * This function allocates memory for a new dynamic array structure,
 * initializes its members, and returns a pointer to the newly created
 * dynamic array.
 *
 * @param initialCapacity The initial capacity of the dynamic array.
 * If zero, a default capacity will be used.
 * @param pDeleteCallback Optional callback function to delete element data when the array is destroyed. Can be NULL.
 * @return Pointer to the newly created MEEDDynamicArray.
 */
struct MEEDDynamicArray* meedDynamicArrayCreate(u32 initialCapacity, MEEDNodeDataDeleteCallback pDeleteCallback);

/**
 * @brief Appends data to the end of the dynamic array.
 *
 * This function adds a new element to the end of the dynamic array.
 * If the current count exceeds the capacity, the array's capacity
 * will be doubled to accommodate the new element.
 *
 * @param pArray Pointer to the MEEDDynamicArray. If NULL, raises an assertion.
 * @param pData Pointer to the data to be added to the array.
 */
void meedDynamicArrayPush(struct MEEDDynamicArray* pArray, void* pData);

/**
 * @brief Retrieves the data at a specific index in the dynamic array.
 *
 * This function returns a pointer to the data stored at the specified
 * index in the dynamic array. If the index is out of bounds, an
 * assertion is raised.
 *
 * @param pArray Pointer to the MEEDDynamicArray. If NULL, raises an assertion.
 * @param index The zero-based index of the element to retrieve. If out of bounds, raises an assertion.
 * @return Pointer to the data at the specified index.
 */
void* meedDynamicArrayAt(struct MEEDDynamicArray* pArray, u32 index);

/**
 * @brief Resizes the dynamic array to a new capacity.
 *
 * This function changes the capacity of the dynamic array to the
 * specified new capacity. If the new capacity is less than the
 * current count, the count will be adjusted accordingly.
 *
 * @param pArray Pointer to the MEEDDynamicArray. If NULL, raises an assertion.
 * @param newCapacity The new capacity for the dynamic array. Must be greater than the current capacity or
 *      raises an assertion.
 */
void meedDynamicArrayResize(struct MEEDDynamicArray* pArray, u32 newCapacity);

/**
 * @brief Destroys a dynamic array and frees its memory.
 *
 * This function deallocates the memory used by the dynamic array and
 * all its elements. If a delete callback was provided during array creation,
 * it will be called for each element's data before freeing the array.
 *
 * @param pArray Pointer to the MEEDDynamicArray to be destroyed. If NULL, raises an assertion.
 */
void meedDynamicArrayDestroy(struct MEEDDynamicArray* pArray);

#if __cplusplus
}
#endif