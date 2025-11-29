#pragma once

#if __cplusplus
extern "C" {
#endif

#include "MEEDEngine/platforms/common.h"
#include "linked_list.h"

/**
 * @file stack.h
 *
 * Self implemented general stack container.
 */

/**
 * Needed information for working with the stack.
 */
struct MEEDStack
{
	struct MEEDLinkedList* pList; ///< The underlying linked list used to implement the stack.
};

/**
 * @brief Creates and initializes a new stack.
 *
 * This function allocates memory for a new stack structure,
 * initializes its members, and returns a pointer to the newly created
 * stack.
 *
 * @return Pointer to the newly created MEEDStack.
 */
struct MEEDStack* meedStackCreate(MEEDNodeDataDeleteCallback pDeleteCallback);

/**
 * Pops data from the top of the stack.
 *
 * @param pStack Pointer to the MEEDStack. If NULL, raises an assertion.
 * @return Pointer to the data popped from the top of the stack.
 */
b8 meedStackEmpty(struct MEEDStack* pStack);

/**
 * Gets the number of elements in the stack.
 *
 * @param pStack Pointer to the MEEDStack. If NULL, raises an assertion.
 * @return The number of elements in the stack.
 */
u32 meedStackGetCount(struct MEEDStack* pStack);

/**
 * Pushes data onto the top of the stack.
 *
 * @param pStack Pointer to the MEEDStack. If NULL, raises an assertion.
 * @param pData Pointer to the data to be pushed onto the stack.
 */
void meedStackPush(struct MEEDStack* pStack, void* pData);

/**
 * Retrieves the data at the top of the stack without removing it.
 *
 * @param pStack Pointer to the MEEDStack. If NULL, raises an assertion.
 * @return Pointer to the data at the top of the stack.
 */
void* meedStackTop(struct MEEDStack* pStack);

/**
 * Removes the data at the top of the stack.
 *
 * @param pStack Pointer to the MEEDStack. If NULL, raises an assertion.
 */
void meedStackPop(struct MEEDStack* pStack);

/**
 * Clears all elements from the stack.
 *
 * @param pStack Pointer to the MEEDStack. If NULL, raises an assertion.
 */
void meedStackClear(struct MEEDStack* pStack);

/**
 * Destroys a stack and frees its memory.
 *
 * This function deallocates the memory used by the stack and
 * its underlying linked list. If a delete callback was provided
 * during stack creation, it will be called for each element's data
 * before freeing the stack.
 *
 * @param pStack Pointer to the MEEDStack. If NULL, raises an assertion.
 */
void meedStackDestroy(struct MEEDStack* pStack);

#if __cplusplus
}
#endif