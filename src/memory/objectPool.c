#include <core/asserts.h>
#include <core/logger.h>
#include <memory/objectPool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

static inline uintptr_t alignUpPtr(uintptr_t PTR, uintptr_t ALIGNMENT)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALIGNMENT % 2 == 0, "[LINEAR ALLOC] : ALIGNMENT must be a multiple of 2");
  if (ALIGNMENT == 0) ALIGNMENT = DEFAULT_ALIGNMENT_BYTES;
  return (PTR + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

bool objectPoolCreate(
  ObjectPool* POOL,
  size_t      CAPACITY,
  size_t      OBJECT_SIZE,
  void*       MEMORY)
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL,    "[OBJECT POOL] : Cannot initialize a NULL ObjectPool pointer");
  FORGE_ASSERT_DEBUG_MESSAGE(CAPACITY > 0,    "[OBJECT POOL] : CAPACITY must be at least 1");
  FORGE_ASSERT_DEBUG_MESSAGE(OBJECT_SIZE > 0, "[OBJECT POOL] : OBJECT_SIZE must be greater than 0");

  POOL->capacity    = CAPACITY;
  POOL->objectSize  = OBJECT_SIZE;
  POOL->ownsMemory  = (MEMORY == NULL);

  // - - - Pad object size to ensure every slot starts at a 16 byte aligned boundary
  size_t minSize = (POOL->objectSize < sizeof(size_t)) ? sizeof(size_t) : POOL->objectSize;
  POOL->stride = alignUpPtr(minSize, DEFAULT_ALIGNMENT_BYTES);

  if (POOL->objectSize < POOL->stride)
  {
    FORGE_LOG_WARNING("[OBJECT POOL] : Object size padded from %zu to %zu bytes for 16 byte CPU alignment. May break assumptions about POOL memory size when memory is allocated by user", POOL->objectSize, POOL->stride);
  }


  // - - - Allocatte memory if needed 
  if (POOL->ownsMemory)
  {
    POOL->memory = malloc(POOL->capacity * POOL->stride);
    if (!POOL->memory)
    {
      FORGE_LOG_ERROR("[OBJECT POOL] : Failed to allocate memory buffer!");
      return false;
    }
  }

  // - - - Initialize the linked list 
  POOL->freeListOffset  = 0;
  POOL->freeCount       = POOL->capacity;

  uintptr_t byteptr = (uintptr_t) POOL->memory;
  for (size_t i = 0; i < POOL->capacity - 1; ++i)
  {
    size_t* nextOffsetSlot  = (size_t*)(byteptr + (i * POOL->stride));
    *nextOffsetSlot         = (i + 1) * POOL->objectSize;
  }

  // - - - Last slot has special end of list value 
  size_t* lastSlot  = (size_t*) (byteptr + ((POOL->capacity - 1) * POOL->stride));
  *lastSlot         = POOL_END_OF_LIST;

  return true;
}

void forge_object_pool_destroy(ObjectPool* POOL) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL,    "[OBJECT POOL] : Cannot destroy a NULL ObjectPool pointer");

  if (POOL->ownsMemory && POOL->memory) 
  {
    free(POOL->memory);
    POOL->memory = NULL;
  }

  POOL->capacity        = 0;
  POOL->objectSize      = 0;
  POOL->freeListOffset  = POOL_END_OF_LIST;
  POOL->freeCount       = 0;
}

void* objectPoolTake(ObjectPool* POOL) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[OBJECT POOL] : Cannot take from NULL pool");
  FORGE_ASSERT_DEBUG_MESSAGE(POOL->memory != NULL, "[OBJECT POOL] : Pool memory is NULL, make sure pool is initialized");

  // - - - Handle full pool & dynamic resizing
  if (POOL->freeListOffset == POOL_END_OF_LIST) 
  {
    FORGE_LOG_ERROR("[OBJECT POOL] : Out of objects!");
    return NULL;
  }

  // - - - Pop element from free list
  uintptr_t objectPtr = ((uintptr_t)POOL->memory) + POOL->freeListOffset;
  POOL->freeListOffset = *(size_t*)objectPtr;
  POOL->freeCount--;

  return (void*)objectPtr;
}

void objectPoolReturn(ObjectPool* POOL, void* OBJECT) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[OBJECT POOL] Cannot return object to NULL pool");
  FORGE_ASSERT_DEBUG_MESSAGE(OBJECT != NULL, "[OBJECT POOL] Cannot return NULL object");

  uintptr_t objByte   = (uintptr_t) OBJECT;
  uintptr_t memStart  = (uintptr_t) POOL->memory;
  size_t    offset    = objByte - memStart;

  // - - - Bounds and Alignment Verification
  FORGE_ASSERT_DEBUG_MESSAGE(objByte >= memStart && offset < (POOL->capacity * POOL->objectSize),
                        "[OBJECT POOL] : Returned OBJECT pointer is out of bounds of this pool!, It may not belong to this pool");
  FORGE_ASSERT_DEBUG_MESSAGE(offset % POOL->objectSize == 0,
                        "[OBJECT POOL] : Returned OBJECT pointer is misaligned with object size!, It may not belong to this pool");

  // - - - Push object back onto free list head
  *(size_t*)OBJECT = POOL->freeListOffset;
  POOL->freeListOffset = offset;
  POOL->freeCount++;
}

void objectPoolDebugPrint(ObjectPool* POOL)
{
  #ifdef DEBUG
    FORGE_ASSERT_DEBUG_MESSAGE(POOL != NULL, "[OBJECT POOL] Cannot visualize a NULL pool");

    bool* freeSlots = (bool*)calloc(POOL->capacity, sizeof(bool));
    if (!freeSlots)
    {
      FORGE_LOG_ERROR("[OBJECT POOL] : failed to allocate debug buffer");
      return;
    }

    char* bar = (char*)malloc(POOL->capacity + 1);
    if (!bar)
    {
      free(freeSlots);
      FORGE_LOG_ERROR("[OBJECT POOL] : failed to allocate debug buffer");
      return;
    }

    // - - -  Walk the free list and mark free slots.
    size_t index = POOL->freeListOffset;
    while (index != POOL_END_OF_LIST)
    {
      if (index >= POOL->capacity)
      {
        FORGE_LOG_ERROR("[OBJECT POOL] : Pool seems to be corrupted, cannot visualize");
        break;
      }

      freeSlots[index] = true;

      uintptr_t slot = (uintptr_t) POOL->memory + index * POOL->stride;
      index = *(size_t*)slot;
    }

    // - - -  Build visualization.
    for (size_t i = 0; i < POOL->capacity; ++i)
    {
      if (i == POOL->freeListOffset)  bar[i] = '@';
      else                            bar[i] = freeSlots[i] ? '_' : '*';
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
      POOL->capacity ? (100.0 * used) / POOL->capacity : 0.0,
      POOL->freeCount,
      POOL->capacity ? (100.0 * POOL->freeCount) / POOL->capacity : 0.0
    );

    free(bar);
    free(freeSlots);
  #else 
    (void)POOL;
  #endif
}
