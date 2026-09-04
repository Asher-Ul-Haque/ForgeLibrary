/**
 * @file : queue.h 
 * @brief : Queue implementation using dynamic Array 
 */

#pragma once 
#include <forgeUtils/core/asserts.h>
#include <forgeUtils/memory/linearAlloc.h>
#include <forgeUtils/dataStructures/dynamicArray.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FORGE_QUEUE_DEFAULT_CAPACITY 8


/// @brief : The queue struct
typedef struct forgeQueue
{
  uint8_t*              data;         ///< Contigous element storage
  size_t                capacity;     ///< Allocated capacity (always a power of 2)
  size_t                mask;         ///< capacity - 1 for bitwise modulo
  size_t                size;         ///< Current number of active elements
  size_t                head;         ///< Index of oldest element
  size_t                tail;         ///< Next write index
  size_t                elementSize;  ///< sizeof(T)
  ForgeLinearAllocator* allocator;    ///< Optional linear allocator, NULL for system heap
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
 * @brief : Sets queue size, growth and shrink both
 * @param QUEUE : The queue to be grown or shrunk
 * @param TARGET_CAPACITY : The new size of the queue
 * @return : True if succesful and false if not
 */
bool forgeQueueReserve(ForgeQueue* QUEUE, size_t TARGET_CAPACITY);

/**
 * @brief : Reserves a slot at the tail and returns a pointer for direct
 * @param QUEUE : The queue to be emplaced
 * @return : Pointer to the unitialized element 
 * @warning : The element is not initialized, use the pointer to initialize it
 */
static inline void* forgeQueueEmplace(ForgeQueue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot emplace in a NULL QUEUE");

  if (QUEUE->size >= QUEUE->capacity)
  {
    size_t newCap = QUEUE->capacity ? (QUEUE->capacity * 2) : FORGE_QUEUE_DEFAULT_CAPACITY;
    if (!forgeQueueReserve(QUEUE, newCap)) return NULL;
  }

  void* slot  = (uint8_t*) QUEUE->data + (QUEUE->tail * QUEUE->elementSize);
  QUEUE->tail = (QUEUE->tail + 1) & QUEUE->mask;
  QUEUE->size++;
  return slot;
}

/**
 * @brief : Enqueues an item to the back of the queue (O(1)).
 * @param QUEUE : A pointer to the queue 
 * @param VALUE_PTR : Pointer to the value to be enqueued 
 * @return : True if successful, False if not
 */
static inline bool forgeQueueEnqueue(ForgeQueue* QUEUE, const void* VALUE_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot enqueue to a NULL QUEUE");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[QUEUE] : Cannot enqueue a NULL VALUE_PTR");

  void* slot = forgeQueueEmplace(QUEUE);
  if (!slot) return false;

  memcpy(slot, VALUE_PTR, QUEUE->elementSize);
  return true;
}

/**
 * @brief : Dequeues an item from the front of the queue in O(1) time.
 * @param QUEUE : A pointer to the queue 
 * @param OUT_VALUE_PTR : Optional pointer to store the dequeued value 
 * @return : True if successful, False if not
 */
static inline bool forgeQueueDequeue(ForgeQueue* QUEUE, void* OUT_VALUE_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot dequeue from a NULL QUEUE");

  if (QUEUE->size == 0) return false;

  if (OUT_VALUE_PTR)
  {
    void* slot = (uint8_t*)QUEUE->data + (QUEUE->head * QUEUE->elementSize);
    memcpy(OUT_VALUE_PTR, slot, QUEUE->elementSize);
  }

  QUEUE->head = (QUEUE->head + 1) & QUEUE->mask;
  QUEUE->size--;

  if (!QUEUE->allocator && QUEUE->capacity > 0)
  {
    if (QUEUE->size <= (QUEUE->capacity >> 2))
    {
      forgeQueueReserve(QUEUE, QUEUE->capacity / 2);
    }
  }

  return true;
}

/**
 * @brief : Returns pointer to item at the front without dequeuing.
 * @param QUEUE : The queue to peek from 
 * @return : Pointer to the head variable of the queue
 */
static inline void* forgeQueuePeek(const ForgeQueue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot peek into a NULL QUEUE");

  if (QUEUE->size == 0) return NULL;
  return (void*) ((uint8_t*)QUEUE->data + (QUEUE->head * QUEUE->elementSize));
}

/**
 * @brief : Returns active item count in queue.
 * @param QUEUE : Pointer to the queue whose size is to be measured 
 * @return : The size of the queue
 */
static inline size_t forgeQueueSize(const ForgeQueue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot check size of a NULL QUEUE");

  return QUEUE->size;
}

/**
 * @brief : Tells whether the queue is empty
 * @param QUEUE : Pointer to the queue whose size is to be measured 
 * @return : True if the queue is empty, false otherwise
 */
static inline size_t forgeQueueIsEmpty(const ForgeQueue* QUEUE)
{
  return forgeQueueSize(QUEUE) == 0;
}

/**
 * @brief : Clears a queue 
 * @param QUEUE : The queue to be cleared
*/
static inline void forgeQueueClear(ForgeQueue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot clear a NULL QUEUE");

  QUEUE->size = 0;
  QUEUE->head = 0;
  QUEUE->tail = 0;
}

/** 
 * @brief : Trims capacity to the smallest power of two that fits current size 
 * @param QUEUE : The queue to be shrunk
 * @return : True if shrunk, false otherwise
*/
static inline bool forgeQueueShrinkToFit(ForgeQueue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot shrink a NULL QUEUE");

  return forgeQueueReserve(QUEUE, QUEUE->size);
}

// - - - Ergonomic Macros - - -

#define FORGE_QUEUE_INIT(QUEUE_PTR, CAPACITY, TYPE) \
  forgeQueueCreate((QUEUE_PTR), (CAPACITY), sizeof(TYPE), NULL)

#define FORGE_QUEUE_EMPLACE(QUEUE_PTR, TYPE) \
  ((TYPE*) forgeQueueEmplace(QUEUE_PTR))

#define FORGE_QUEUE_PEEK(QUEUE_PTR, TYPE) \
  ((TYPE*) forgeQueuePeek(QUEUE_PTR))

#define FORGE_QUEUE_ENQUEUE_VAL(QUEUE_PTR, TYPE, VALUE) \
  do { \
    TYPE* _slot = FORGE_QUEUE_EMPLACE(QUEUE_PTR, TYPE); \
    if (_slot) *_slot = (VALUE); \
  } while(0)

#ifdef __cplusplus
}
#endif
