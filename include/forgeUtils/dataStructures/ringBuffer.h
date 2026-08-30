/**
 * @file : ringBuffer.h 
 * @brief : Ring buffer implementation in C
 */

#pragma once 

#include <forgeUtils/memory/linearAlloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef DEFAULT_ALIGNMENT_BYTES  
  #define DEFAULT_ALIGNMENT_BYTES 16 
#endif

#ifdef __cplusplus
extern "C" {
#endif


/// @brief : Ring Buffer data structure
typedef struct forgeRingBuffer 
{
  uint8_t*              data;            ///< Pointer to contiguous ring memory
  size_t                capacity;        ///< Total element capacity (fixed)
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
 * @brief : Writes/Pushes an element into the Ring Buffer.
 * 
 * @param RING : Pointer to RingBuffer.
 * @param ITEM_PTR : Pointer to the element bytes to write.
 * @return : true if item was written, false if buffer is full and allow_overwrite is false.
 */
bool forgeRingBufferPush(ForgeRingBuffer* RING, const void* ITEM_PTR);

/**
 * @brief : Reads/Pops an element from the Ring Buffer.
 * 
 * @param RING : Pointer to RingBuffer.
 * @param OUT_ITEM_PTR : Optional pointer to receive popped element bytes.
 * @return : true if item was read, false if buffer is empty.
 */
bool forgeRingBufferPop(ForgeRingBuffer* RING, void* OUT_ITEM_PTR);

/**
 * @brief : Views the oldest element in the Ring Buffer without removing it.
 * @param RING : Pointer to the ring buffer 
 * @return : pointer to the first element in the buffer
 */
void* forgeRingBufferPeek(const ForgeRingBuffer* RING);

/**
 * @brief : Resets read and write indices to empty the buffer without deallocating.
 * @param RING : Pointer to the ring buffer 
 */
void forgeRingBufferClear(ForgeRingBuffer* RING);

/**
 * @brief : Returns true if buffer is empty.
 * @param RING : Pointer to the ring buffer 
 * @return : true if the buffer is empty, false otherwise
 */
static inline bool forgeRingBufferIsEmpty(const ForgeRingBuffer* RING) 
{ return RING ? (RING->count == 0) : true; }

/**
 * @brief : Returns true if buffer is full.
 * @param RING : Pointer to teh ring buffer to be tested 
 * @return : true if the buffer is full, false otherwise
 */
static inline bool forgeRingBufferIsFull(const ForgeRingBuffer* RING) 
{ return RING ? (RING->count == RING->capacity) : false; }

/**
 * @brief : Returns current active element count.
 * @param RING : Pointer to ring buffer 
 * @return : The size of the ring buffer
 */
static inline size_t forgeRingBufferSize(const ForgeRingBuffer* RING) 
{ return RING ? RING->count : 0; }

#ifdef __cplusplus
}
#endif
