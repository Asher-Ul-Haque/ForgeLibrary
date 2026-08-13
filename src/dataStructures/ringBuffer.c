#include <memory/linearAlloc.h>
#include <dataStructures/ringBuffer.h>
#include <core/asserts.h>
#include <core/logger.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static inline uintptr_t alignUpPtr(uintptr_t PTR, uintptr_t ALIGNMENT)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALIGNMENT % 2 == 0, "[LINEAR ALLOC] : ALIGNMENT must be a multiple of 2");
  if (ALIGNMENT == 0) ALIGNMENT = DEFAULT_ALIGNMENT_BYTES;
  return (PTR + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

bool forgeRingBufferCreate(
  ForgeRingBuffer*       RING,
  size_t            CAPACITY,
  size_t            ELEMENT_SIZE,
  bool              ALLOW_OVERWRITE,
  ForgeLinearAllocator*  ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Target pointer cannot be NULL");
  FORGE_ASSERT_DEBUG_MESSAGE(CAPACITY > 0, "[RING BUFFER] : Capacity must be greater than 0");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[RING BUFFER] : Element size must be greater than 0");

  RING->elementSize     = alignUpPtr(ELEMENT_SIZE, DEFAULT_ALIGNMENT_BYTES);
  RING->capacity        = CAPACITY;
  RING->head            = 0;
  RING->tail            = 0;
  RING->count           = 0;
  RING->allowOverwrite  = ALLOW_OVERWRITE;
  RING->allocator       = ALLOCATOR;

  size_t totalBytes = RING->capacity * RING->elementSize;

  if (RING->allocator)
  {
    RING->data = (uint8_t*) forgeLinearAllocAllocate(RING->allocator, totalBytes, DEFAULT_ALIGNMENT_BYTES);
  }
  else 
  {
    RING->data = (uint8_t*) malloc(totalBytes);
  }

  if (!RING->data)
  {
    FORGE_LOG_ERROR("[RING BUFFER] : Failed to allocate memory for ring buffer!");
    return false;
  }

  return true;
}

void forgeRingBufferDestroy(ForgeRingBuffer* RING)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot destroy a NULL RING BUFFER");

  if (RING->data)
  {
    if (!RING->allocator) free(RING->data);
    RING->data = NULL;
  }

  RING->capacity    = 0;
  RING->elementSize = 0;
  RING->head        = 0;
  RING->tail        = 0;
  RING->count       = 0;
  RING->allocator   = NULL;
}

bool forgeRingBufferPush(ForgeRingBuffer* RING, const void* ITEM_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot push to NULL ring buffer");
  FORGE_ASSERT_DEBUG_MESSAGE(ITEM_PTR != NULL, "[RING BUFFER] : Cannot push NULL item pointer");

  if (RING->count == RING->capacity)
  {
    if (!RING->allowOverwrite)
    {
      FORGE_LOG_WARNING("[RING BUFFER] : Ring buffer full, clear out before pushing");
      return false;
    }

    RING->tail = (RING->tail + 1) % RING->capacity;
    RING->count--;
  }

  uint8_t* target = RING->data + (RING->head * RING->elementSize);
  memcpy(target, ITEM_PTR, RING->elementSize);

  RING->head = (RING->head + 1) % RING->capacity;
  RING->count++;

  return true;
}

bool forgeRingBufferPop(ForgeRingBuffer* RING, void* OUT_ITEM_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot pop from NULL RING BUFFER");

  if (RING->count == 0)
  {
    FORGE_LOG_WARNING("[RING BUFFER] : Ring buffer empty, cannot pop!, add something first");
    return false;
  }

  uint8_t* source = RING->data + (RING->tail * RING->elementSize);
  if (OUT_ITEM_PTR) 
  { memcpy(OUT_ITEM_PTR, source, RING->elementSize); } 

  RING->tail = (RING->tail + 1) % RING->capacity;
  RING->count--;

  return true;
}

void* forgeRingBufferPeek(const ForgeRingBuffer* RING)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot pop from NULL RING BUFFER");

  if (RING->count == 0) return NULL;

  return (void*) (RING->data + (RING->tail * RING->elementSize));
}

void forgeRingBufferClear(ForgeRingBuffer* RING)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Cannot clear a NULL RING BUFFER");

  RING->head  = 0;
  RING->tail  = 0;
  RING->count = 0;
}
