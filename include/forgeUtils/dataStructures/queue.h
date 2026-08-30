/**
 * @file : queue.h 
 * @brief : Queue implementation using dynamic Array 
 */

#pragma once 
#include <forgeUtils/dataStructures/dynamicArray.h>

#ifdef __cplusplus
extern "C" {
#endif

/// @brief : The queue struct
typedef struct forgeQueue 
{
  ForgeDynamicArray array;  ///< The underlying dynamicArray
  size_t            head;   ///< Head cursor offset for O(1) dequeue operations
} ForgeQueue;

/**
 * @brief : Creates a queue instance.
 * @param QUEUE : A pointer to the Queue to be created 
 * @param INITIAL_CAPACITY : How many elements at the start
 * @param ELEMENT_SIZE : What is the size of an element 
 * @param ALLOCATOR : Optional Linear allocator
 * @return : True if successful, False if not
 */
bool forgeQueueCreate(
  ForgeQueue*           QUEUE, 
  size_t                INITIAL_CAPACITY, 
  size_t                ELEMENT_SIZE, 
  ForgeLinearAllocator* ALLOCATOR);

/**
 * @brief : Destroys the queue.
 * @param QUEUE : Pointer to the queue to be destroyed
*/
void forgeQueueDestroy(ForgeQueue* QUEUE);

/**
 * @brief : Enqueues an item to the back of the queue (O(1)).
 * @param QUEUE : A pointer to the queue 
 * @param VALUE_PTR : Pointer to the value to be enqueued 
 * @return : True if successful, False if not
 */
bool forgeQueueEnqueue(ForgeQueue* QUEUE, const void* VALUE_PTR);

/**
 * @brief : Dequeues an item from the front of the queue in O(1) time.
 * @param QUEUE : A pointer to the queue 
 * @param OUT_VALUE_PTR : Optional pointer to store the dequeued value 
 * @return : True if successful, False if not
 */
bool forgeQueueDequeue(ForgeQueue* QUEUE, void* OUT_VALUE_PTR);

/**
 * @brief : Returns pointer to item at the front without dequeuing.
 * @param QUEUE : The queue to peek from 
 * @return : Pointer to the head variable of the queue
 */
void* forgeQueuePeek(const ForgeQueue* QUEUE);

/**
 * @brief : Returns active item count in queue.
 * @param QUEUE : Pointer to the queue whose size is to be measured 
 * @return : The size of the queue
 */
size_t forgeQueueSize(const ForgeQueue* QUEUE);

#ifdef __cplusplus
}
#endif
