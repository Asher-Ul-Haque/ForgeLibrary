#include <core/asserts.h>
#include <core/logger.h>
#include <memory/objectPool.h>
#include <memory/tracker.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline uintptr_t alignUpPtr(uintptr_t PTR, uintptr_t ALIGNMENT)
{
  if (ALIGNMENT == 0) ALIGNMENT = DEFAULT_ALIGNMENT_BYTES;
  FORGE_ASSERT_DEBUG_MESSAGE((ALIGNMENT & (ALIGNMENT - 1)) == 0, "[OBJECT POOL] : ALIGNMENT must be a power of 2");
  return (PTR + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

bool forgeObjectPoolCreate(
  ForgeObjectPool*  POOL,
  size_t            CAPACITY,
  size_t            OBJECT_SIZE,
  void*             MEMORY)
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL,        "[OBJECT POOL] : Cannot initialize a NULL ObjectPool pointer");
  FORGE_ASSERT_DEBUG_MESSAGE(CAPACITY > 0,        "[OBJECT POOL] : CAPACITY must be at least 1");
  FORGE_ASSERT_DEBUG_MESSAGE(OBJECT_SIZE > 0,     "[OBJECT POOL] : OBJECT_SIZE must be greater than 0");

  POOL->capacity    = CAPACITY;
  POOL->objectSize  = OBJECT_SIZE;
  POOL->ownsMemory  = (MEMORY == NULL);

  // - - - Pad object size to ensure every slot starts at 16-byte alignment boundary
  size_t minSize  = (POOL->objectSize < sizeof(size_t)) ? sizeof(size_t) : POOL->objectSize;
  POOL->stride    = alignUpPtr(minSize, DEFAULT_ALIGNMENT_BYTES);

  if (POOL->objectSize < POOL->stride)
  {
    FORGE_LOG_WARNING("[OBJECT POOL] : Object size padded from %zu to %zu bytes for 16-byte CPU alignment.", POOL->objectSize, POOL->stride);
  }

  // - - - Allocate memory if needed
  if (POOL->ownsMemory)
  {
    POOL->memory = FORGE_MALLOC(POOL->capacity * POOL->stride);
    if (!POOL->memory)
    {
      FORGE_LOG_ERROR("[OBJECT POOL] : Failed to allocate memory buffer!");
      return false;
    }
  }
  else
  {
    POOL->memory = MEMORY;
  }

  // - - - Initialize the linked list storing BYTE OFFSETS to next free slot
  POOL->freeListOffset = 0;
  POOL->freeCount      = POOL->capacity;

  uintptr_t byteptr = (uintptr_t)POOL->memory;
  for (size_t i = 0; i < POOL->capacity - 1; ++i)
  {
    size_t* nextOffsetSlot = (size_t*)(byteptr + (i * POOL->stride));
    *nextOffsetSlot        = (i + 1) * POOL->stride; // Next slot BYTE OFFSET
  }

  // - - - Last slot has special end-of-list marker
  size_t* lastSlot = (size_t*)(byteptr + ((POOL->capacity - 1) * POOL->stride));
  *lastSlot        = POOL_END_OF_LIST;

  return true;
}

void forgeObjectPoolDestroy(ForgeObjectPool* POOL) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[OBJECT POOL] : Cannot destroy a NULL ObjectPool pointer");

  if (POOL->ownsMemory && POOL->memory) 
  {
    FORGE_FREE(POOL->memory);
    POOL->memory = NULL;
  }

  POOL->capacity        = 0;
  POOL->objectSize      = 0;
  POOL->stride          = 0;
  POOL->freeListOffset  = POOL_END_OF_LIST;
  POOL->freeCount       = 0;
}

void* forgeObjectPoolTakeObject(ForgeObjectPool* POOL) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[OBJECT POOL] : Cannot take from NULL pool");
  FORGE_ASSERT_DEBUG_MESSAGE(POOL->memory != NULL, "[OBJECT POOL] : Pool memory is NULL, make sure pool is initialized");

  if (POOL->freeListOffset == POOL_END_OF_LIST) 
  {
    FORGE_LOG_ERROR("[OBJECT POOL] : Out of objects!");
    return NULL;
  }

  // - - - Pop element from free list using byte offset
  uintptr_t objectPtr = ((uintptr_t)POOL->memory) + POOL->freeListOffset;
  POOL->freeListOffset = *(size_t*)objectPtr;
  POOL->freeCount--;

  return (void*)objectPtr;
}

void forgeObjectPoolReturnObject(ForgeObjectPool* POOL, void* OBJECT) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[OBJECT POOL] Cannot return object to NULL pool");
  FORGE_ASSERT_DEBUG_MESSAGE(OBJECT != NULL, "[OBJECT POOL] Cannot return NULL object");

  uintptr_t objByte   = (uintptr_t)OBJECT;
  uintptr_t memStart  = (uintptr_t)POOL->memory;
  size_t    offset    = objByte - memStart;

  // - - - Bounds and Alignment Verification using STRIDE
  FORGE_ASSERT_DEBUG_MESSAGE(objByte >= memStart && offset < (POOL->capacity * POOL->stride),
                          "[OBJECT POOL] : Returned OBJECT pointer is out of bounds of this pool!");
  FORGE_ASSERT_DEBUG_MESSAGE(offset % POOL->stride == 0,
                          "[OBJECT POOL] : Returned OBJECT pointer is misaligned with pool stride!");

  // - - - Push object back onto free list head
  *(size_t*)OBJECT = POOL->freeListOffset;
  POOL->freeListOffset = offset;
  POOL->freeCount++;
}

void forgeObjectPoolDebugPrint(ForgeObjectPool* POOL)
{
  #ifdef DEBUG
    FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[OBJECT POOL] Cannot visualize a NULL pool");

    if (!POOL || POOL->capacity == 0) return;

    bool* freeSlots = (bool*)FORGE_MALLOC(POOL->capacity * sizeof(bool));
    if (!freeSlots) return;
    memset(freeSlots, 0, POOL->capacity * sizeof(bool));

    char* bar = (char*)FORGE_MALLOC(POOL->capacity + 1);
    if (!bar)
    {
      FORGE_FREE(freeSlots);
      return;
    }

    // - - - Walk free list using byte offsets
    size_t curr_offset   = POOL->freeListOffset;
    size_t visited_count = 0;

    while (curr_offset != POOL_END_OF_LIST)
    {
      size_t slot_index = curr_offset / POOL->stride;

      if (slot_index >= POOL->capacity || (curr_offset % POOL->stride != 0))
      {
        FORGE_LOG_ERROR("[OBJECT POOL] : Pool is corrupted (invalid offset %zu)", curr_offset);
        break;
      }

      if (visited_count >= POOL->capacity || freeSlots[slot_index])
      {
        FORGE_LOG_ERROR("[OBJECT POOL] : Cycle detected in free list!");
        break;
      }

      freeSlots[slot_index] = true;
      visited_count++;

      uintptr_t slot_addr = (uintptr_t)POOL->memory + curr_offset;
      curr_offset = *(size_t*)slot_addr;
    }

    // - - - - Build bar representation
    size_t head_index = (POOL->freeListOffset != POOL_END_OF_LIST) ? (POOL->freeListOffset / POOL->stride) : POOL_END_OF_LIST;
    for (size_t i = 0; i < POOL->capacity; ++i)
    {
      if (i == head_index && freeSlots[i]) {
        bar[i] = '@'; // Free list head
      } else {
        bar[i] = freeSlots[i] ? '_' : '*'; // _ = Free, * = Occupied
      }
    }
    bar[POOL->capacity] = '\0';

    const size_t used = POOL->capacity - POOL->freeCount;

    FORGE_LOG_INFO(
      "Pool [%s]\n"
      "Capacity : %zu\n"
      "Used     : %zu (%.1f%%)\n"
      "Free     : %zu (%.1f%%)\n",
      bar,
      POOL->capacity,
      used,
      POOL->capacity ? (100.0 * (double)used) / (double)POOL->capacity : 0.0,
      POOL->freeCount,
      POOL->capacity ? (100.0 * (double)POOL->freeCount) / (double)POOL->capacity : 0.0
    );

    FORGE_FREE(bar);
    FORGE_FREE(freeSlots);
  #else 
    (void)POOL;
  #endif
}
