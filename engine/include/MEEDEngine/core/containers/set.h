#pragma once

#if __cplusplus
extern "C" {
#endif

#include "MEEDEngine/platforms/common.h"
#include "callback.h"
#include "linked_list.h"

#define MEED_SET_NOT_FOUND_INDEX ((u32)(-1))

/**
 * @file set.h
 *
 * Self implemented general set container.
 *
 * @note Set does not support auto deletion of element data node.
 */

/**
 * The callback type which is used for comparing two elements in the set.
 * Should return:
 *  - A negative value if pA < pB
 *  - Zero if pA == pB
 *  - A positive value if pA > pB
 */
typedef i32 (*MEEDSetCompareCallback)(const void* pA, const void* pB);

/**
 * Needed information for the set container.
 */
struct MEEDSet
{
	struct MEEDLinkedList* pList;			 ///< The underlying linked list to store set elements.
	MEEDSetCompareCallback pCompareCallback; ///< Callback function to compare two elements.
};

/**
 * @brief Creates and initializes a new set.
 *
 * This function allocates memory for a new set structure,
 * initializes its members, and returns a pointer to the newly created
 * set.
 *
 * @param pCompareCallback Callback function to compare two elements. Must not be NULL.
 * @return Pointer to the newly created MEEDSet.
 */
struct MEEDSet* meedSetCreate(MEEDSetCompareCallback pCompareCallback);

/**
 * @brief Retrieves the current number of elements in the set.
 *
 * This function returns the number of elements currently stored
 * in the set.
 *
 * @param pSet Pointer to the MEEDSet. If NULL, raises an assertion.
 * @return The number of elements in the set.
 */
u32 meedSetCount(struct MEEDSet* pSet);

/**
 * @brief Inserts data into the set.
 *
 * This function adds a new element to the set if it does not
 * already exist in the set. If an equivalent element is found,
 * the new data is not added.
 *
 * @param pSet Pointer to the MEEDSet. If NULL, raises an assertion.
 * @param pData Pointer to the data to be added to the set.
 *
 * @note Element data is not copied; the user is responsible for
 * managing the memory of the elements stored in the set. And the element will be
 * sorted based on the comparison callback provided during set creation.
 */
void meedSetPush(struct MEEDSet* pSet, void* pData);

/**
 * @brief Retrieves the data at a specific index in the set.
 *
 * This function returns a pointer to the data stored at the specified
 * index in the set. If the index is out of bounds, an
 * assertion is raised.
 *
 * @param pSet Pointer to the MEEDSet. If NULL, raises an assertion.
 * @param index The zero-based index of the element to retrieve. If out of bounds, raises an assertion.
 * @return Pointer to the data at the specified index.
 */
void* meedSetAt(struct MEEDSet* pSet, u32 index);

/**
 * @brief Erases the element at a specific index in the set.
 *
 * This function removes the element at the specified index from the
 * set and frees its memory. Note that element data is not deleted;
 * the user is responsible for managing the memory of the
 * elements stored in the set. If the index is out of bounds,
 * an assertion is raised.
 *
 * @param pSet Pointer to the MEEDSet. If NULL, raises an assertion.
 * @param index The zero-based index of the element to be erased. If out of bounds, raises an assertion.
 */
void meedSetErase(struct MEEDSet* pSet, u32 index);

/**
 * @brief Clears all elements from the set.
 *
 * This function removes all elements from the set and
 * resets the count to zero. Note that element data is not deleted;
 * the user is responsible for managing the memory of the
 * elements stored in the set.
 *
 * @param pSet Pointer to the MEEDSet to be cleared. If NULL, raises an assertion.
 */
void meedSetClear(struct MEEDSet* pSet);

/**
 * @brief Finds the index of a specific element in the set.
 *
 * This function searches for the specified data in the set
 * and returns its index if found. If the element is not found,
 * it returns a special value indicating that the element does not exist in the set.
 *
 * @param pSet Pointer to the MEEDSet. If NULL, raises an assertion.
 * @param pData Pointer to the data to find in the set.
 * @return The zero-based index of the element if found; otherwise, returns `MEED_SET_NOT_FOUND_INDEX`.
 */
u32 meedSetFind(struct MEEDSet* pSet, void* pData);

/**
 * @brief Destroys a set and frees its memory.
 *
 * This function deallocates the memory used by the set and
 * all its elements. Note that element data is not deleted;
 * the user is responsible for managing the memory of the
 * elements stored in the set.
 *
 * @param pSet Pointer to the MEEDSet to be destroyed. If NULL, raises an assertion.
 */
void meedSetDestroy(struct MEEDSet* pSet);

#if __cplusplus
}
#endif