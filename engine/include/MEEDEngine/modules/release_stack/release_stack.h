#pragma once

#if __cplusplus
extern "C" {
#endif

/**
 * @file release_stack.h
 * Used for working with some resources which is need to be released in a certain order.
 */

#include "MEEDEngine/core/containers/stack.h"
#include "MEEDEngine/platforms/common.h"

/**
 * Structure representing a release stack.
 * Used to manage resources that need to be released in a specific order.
 */
struct MEEDReleaseStack
{
	struct MEEDStack* pStack;
};

/**
 * User must pass this function pointer to free the attached instance.
 * @param pData Pointer to the data to be released.
 */
typedef void (*MEEDReleaseFunc)(void*);

/**
 * Structure representing an item in the release stack.
 * Contains the data pointer and the associated release function.
 */
struct MEEDReleaseStackItem
{
	void*			pData;		  ///< Pointer to the data associated with the resource.
	MEEDReleaseFunc pReleaseFunc; ///< Function pointer to release the resource.
};

/**
 * Initialize an instance of MEEDReleaseStack.
 * @return Pointer to the created MEEDReleaseStack instance.
 */
struct MEEDReleaseStack* meedReleaseStackCreate();

/**
 * Push a release function and its associated data onto the release stack.
 * @param pReleaseStack Pointer to the MEEDReleaseStack instance.
 * @param pData Pointer to the data to be passed to the release function.
 * @param pReleaseFunc Pointer to the function to be called for releasing the resource. CANNOT be NULL.
 */
void meedReleaseStackPush(struct MEEDReleaseStack* pReleaseStack, void* pData, MEEDReleaseFunc pReleaseFunc);

/**
 * Free the MEEDReleaseStack instance and release all resources in the stack.
 * @param pReleaseStack Pointer to the MEEDReleaseStack instance to be destroyed. if NULL, raise an exception.
 *
 * @note All resources and the callbacks will be released in the reverse order they were added (LIFO order).
 */
void meedReleaseStackDestroy(struct MEEDReleaseStack* pReleaseStack);

#if __cplusplus
}
#endif