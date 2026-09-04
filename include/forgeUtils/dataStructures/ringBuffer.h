/**
 * @file : ringBuffer.h 
 * @brief : Ring buffer implementation in C
 */

#pragma once 

#include <forgeUtils/core/logger.h>
#include <forgeUtils/core/asserts.h>
#include <forgeUtils/memory/linearAlloc.h>
#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FORGE_RING_BUFFER_MIN_CAPACITY 8

#ifdef __cplusplus
extern "C" {
#endif


/// @brief : Ring Buffer data structure
typedef struct forgeRingBuffer 
{
  uint8_t*              data;            ///< Pointer to contiguous ring memory
  size_t                capacity;        ///< Total element capacity (fixed)
  size_t                mask;            ///< capacity - 1 for bitwise modulo
  size_t                elementSize;     ///< Size of each element in bytes
  size_t                head;            ///< Write index
  size_t                tail;            ///< Read index
  size_t                count;           ///< Number of active items in buffer
  bool                  allowOverwrite;  ///< If true, pushes overwrite oldest item when full
  ForgeLinearAllocator* allocator;       ///< Optional linear allocator used for initial buffer
} ForgeRingBuffer;

/**
 * @brief : Initializes a fixed-capacity Ring Buffer.
 * 
 * @param RING : Pointer to RingBuffer struct.
 * @param CAPACITY : Maximum number of elements buffer can hold.
 * @param ELEMENT_SIZE : Size of an individual element in bytes (sizeof(T)).
 * @param ALLOW_OVERWRITE : If true, pushing to a full buffer overwrites the oldest element.
 * @param ALLOCATOR : Pointer to linear allocator, or NULL to use global memory tracker.
 * @return : true if initialized successfully, false otherwise.
 */
bool forgeRingBufferCreate(
  ForgeRingBuffer*      RING,
  size_t                CAPACITY,
  size_t                ELEMENT_SIZE,
  bool                  ALLOW_OVERWRITE,
  ForgeLinearAllocator* ALLOCATOR);

/**
 * @brief : Destroys the Ring Buffer and frees backing memory.
 * @param RING : The ring buffer 
 */
void forgeRingBufferDestroy(ForgeRingBuffer* RING);

/**
 * @brief : Reserves a slot at the head for writing and returns a direct pointer.
 * @warning : If full and allowOverwrite is true, advances the tail.
 * @warning : Returns NULL if full and allowOverwrite is false.
 * @warning : The element is not initialized, use the pointer to initialize it.
 * @param RING : Pointer to the ring to be emplaced
 * @return : Pointer to the unitialized element.
 */
static inline void* forgeRingBufferEmplace(ForgeRingBuffer* RING)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot emplace in a NULL RING buffer");

  if (RING->count == RING->capacity)
  {
    if (!RING->allowOverwrite)
    {
      FORGE_LOG_WARNING("[RING BUFFER] : Cannot emplace, ring is full");
      return NULL;
    }

    // - - - Overwrite oldest item : advance read cursor
    RING->tail = (RING->tail + 1) & RING->mask;
    RING->count--;
  }

  void* slot = RING->data + (RING->head * RING->elementSize);
  RING->head = (RING->head + 1) & RING->mask;
  RING->count++;

  return slot;
}

/**
 * @brief : Writes/Pushes an element into the Ring Buffer.
 * 
 * @param RING : Pointer to RingBuffer.
 * @param ITEM_PTR : Pointer to the element bytes to write.
 * @return : true if item was written, false if buffer is full and allow_overwrite is false.
 */
static inline bool forgeRingBufferPush(ForgeRingBuffer* RING, const void* ITEM_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot push to a NULL RING");
  FORGE_ASSERT_DEBUG_MESSAGE(ITEM_PTR != NULL, "[RING BUFFER] : Cannot push a NULL ITEM_PTR");

  void* slot = forgeRingBufferEmplace(RING);
  if (!slot) return false;

  memcpy(slot, ITEM_PTR, RING->elementSize);
  return true;
}

/**
 * @brief : Reads/Pops an element from the Ring Buffer.
 * 
 * @param RING : Pointer to RingBuffer.
 * @param OUT_ITEM_PTR : Optional pointer to receive popped element bytes.
 * @return : true if item was read, false if buffer is empty.
 */
static inline bool forgeRingBufferPop(ForgeRingBuffer* RING, void* OUT_ITEM_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot pop from a NULL RING");

  if (RING->count == 0) return false;

  if (OUT_ITEM_PTR)
  {
    void* source = RING->data + (RING->tail * RING->elementSize);
    memcpy(OUT_ITEM_PTR, source, RING->elementSize);
  }

  RING->tail = (RING->tail + 1) & RING->mask;
  RING->count--;

  return true;
}

/**
 * @brief : Views the oldest element in the Ring Buffer without removing it.
 * @param RING : Pointer to the ring buffer 
 * @return : pointer to the first element in the buffer
 */
static inline void* forgeRingBufferPeek(const ForgeRingBuffer* RING)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot peek in a NULL RING");

  if (RING->count == 0)
  {
    FORGE_LOG_WARNING("[RING BUFFER] : Cannot peek, buffer is empty");
    return NULL;
  }

  return (void*)(RING->data + (RING->tail * RING->elementSize));
}

/**
 * @brief : Resets read and write indices to empty the buffer without deallocating.
 * @param RING : Pointer to the ring buffer 
 */
static inline void forgeRingBufferClear(ForgeRingBuffer* RING)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot clear a NULL RING");

  RING->head  = 0;
  RING->tail  = 0;
  RING->count = 0;
}

/**
 * @brief : Returns true if buffer is empty.
 * @param RING : Pointer to the ring buffer 
 * @return : true if the buffer is empty, false otherwise
 */
static inline bool forgeRingBufferIsEmpty(const ForgeRingBuffer* RING) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot check if a NULL RING is empty");
  return RING ? (RING->count == 0) : true; 
}

/**
 * @brief : Returns true if buffer is full.
 * @param RING : Pointer to teh ring buffer to be tested 
 * @return : true if the buffer is full, false otherwise
 */
static inline bool forgeRingBufferIsFull(const ForgeRingBuffer* RING) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot check if a NULL RING is full");
  return RING ? (RING->count == RING->capacity) : false; 
}

/**
 * @brief : Returns current active element count.
 * @param RING : Pointer to ring buffer 
 * @return : The size of the ring buffer
 */
static inline size_t forgeRingBufferSize(const ForgeRingBuffer* RING) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot check size of a NULL RING");
  return RING ? RING->count : 0; 
}


// - - - Ergonomic & Type-Safe Macros - - -

#define FORGE_RING_BUFFER_INIT(RING_PTR, CAPACITY, TYPE, ALLOW_OVERWRITE) \
  forgeRingBufferCreate((RING_PTR), (CAPACITY), sizeof(TYPE), (ALLOW_OVERWRITE), NULL)

#define FORGE_RING_BUFFER_EMPLACE(RING_PTR, TYPE) \
  ((TYPE*) forgeRingBufferEmplace(RING_PTR))

#define FORGE_RING_BUFFER_PEEK(RING_PTR, TYPE) \
  ((TYPE*) forgeRingBufferPeek(RING_PTR))

#define FORGE_RING_BUFFER_PUSH_VAL(RING_PTR, TYPE, VALUE) \
  do { \
    TYPE* _slot = FORGE_RING_BUFFER_EMPLACE(RING_PTR, TYPE); \
    if (_slot) *_slot = (VALUE); \
  } while(0)

#ifdef __cplusplus
}
#endif
