#pragma once
#include "MEEDEngine/platforms/common.h"

/**
 * @file linked_list.h
 *
 * Self implemented general linked list container.
 */

/**
 * @struct MEEDLinkedListNode
 * @brief A node in a linked list.
 *
 * This structure represents a single node in a linked list, containing
 * a pointer to the data and a pointer to the next node in the list.
 */
struct MEEDLinkedListNode
{
	void* pData; ///< Pointer to the data stored in the node.

	struct MEEDLinkedListNode* pNext; ///< Pointer to the next node in the linked list.
};

/**
 * The callback type which is called when deleting each node's data during linked list destruction.
 */
typedef void (*MEEDLinkedListDeleteCallback)(void* pData);

struct MEEDLinkedList
{
	u32							 size;			  ///< The number of nodes in the linked list.
	struct MEEDLinkedListNode*	 pHead;			  ///< Pointer to the head node of the linked list.
	struct MEEDLinkedListNode*	 pTail;			  ///< Pointer to the tail node of the linked list.
	MEEDLinkedListDeleteCallback pDeleteCallback; ///< Callback function to delete node data.
};

/**
 * @brief Creates and initializes a new linked list.
 *
 * This function allocates memory for a new linked list structure,
 * initializes its members, and returns a pointer to the newly created
 * linked list.
 *
 * @param pDeleteCallback Optional callback function to delete node data when the list is destroyed. Can be NULL.
 * If NULL, node data will not be deleted automatically.
 *
 * @return Pointer to the newly created MEEDLinkedList.
 */
struct MEEDLinkedList* meedLinkedListCreate(MEEDLinkedListDeleteCallback pDeleteCallback);

/**
 * Append the data to the end of the linked list.
 *
 * @param pList Pointer to the MEEDLinkedList. If NULL, raises an assertion.
 * @param pData Pointer to the data to be added to the list.
 */
void meedLinkedListPush(struct MEEDLinkedList* pList, void* pData);

/**
 * @brief Inserts data at a specific index in the linked list.
 *
 * This function inserts a new node containing the provided data
 * at the specified index in the linked list. If the index is equal
 * to the size of the list, the data is appended to the end.
 * If the index is out of bounds, an assertion is raised.
 *
 * @param pList Pointer to the MEEDLinkedList. If NULL, raises an assertion.
 * @param index The zero-based index where the data should be inserted. If out of bounds, raises an assertion.
 * @param pData Pointer to the data to be inserted into the list.
 */
void meedLinkedListInsert(struct MEEDLinkedList* pList, u32 index, void* pData);

/**
 * @brief Gets the number of elements in the linked list.
 *
 * This function returns the current size of the linked list,
 * which represents the number of nodes it contains.
 *
 * @param pList Pointer to the MEEDLinkedList. If NULL, raises an assertion.
 * @return The number of elements in the linked list.
 */
u32 meedLinkedListGetCount(struct MEEDLinkedList* pList);

/**
 * @brief Checks if the linked list is empty.
 *
 * This function checks whether the linked list contains any elements.
 *
 * @param pList Pointer to the MEEDLinkedList. If NULL, raises an assertion.
 * @return MEED_TRUE if the list is empty, MEED_FALSE otherwise.
 */
b8 meedLinkedListEmpty(struct MEEDLinkedList* pList);

/**
 * @brief Retrieves the data at a specific index in the linked list.
 *
 * This function traverses the linked list to find the node at the
 * specified index and returns a pointer to its data. If the index
 * is out of bounds, an assertion is raised.
 *
 * @param pList Pointer to the MEEDLinkedList. If NULL, raises an assertion.
 * @param index The zero-based index of the element to retrieve. If out of bounds, raises an assertion.
 * @return Pointer to the data at the specified index.
 */
void* meedLinkedListAt(struct MEEDLinkedList* pList, u32 index);

/**
 * @brief Clears all elements from the linked list.
 *
 * This function removes all nodes from the linked list and frees
 * their memory. If a delete callback was provided during list creation,
 * it will be called for each node's data before freeing the node.
 *
 * @param pList Pointer to the MEEDLinkedList. If NULL, raises an assertion.
 */
void meedLinkedListClear(struct MEEDLinkedList* pList);

/**
 * @brief Destroys a linked list and frees its memory.
 *
 * This function deallocates the memory used by the linked list and
 * all its nodes. It does not free the memory of the data stored in
 * the nodes; that is the responsibility of the caller.
 *
 * @param pList Pointer to the MEEDLinkedList to be destroyed. If NULL, raises an assertion.
 */
void meedLinkedListDestroy(struct MEEDLinkedList* pList);