#include <forgeUtils/memory/linearAlloc.h>
#include <forgeUtils/dataStructures/ringBuffer.h>
#include <forgeUtils/core/asserts.h>
#include <forgeUtils/core/logger.h>
#include <forgeUtils/memory/tracker.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


static inline size_t forgeRoundToPowerOfTwo(size_t VAL)
{
  if (VAL < FORGE_RING_BUFFER_MIN_CAPACITY) return FORGE_RING_BUFFER_MIN_CAPACITY;
  VAL--;
  VAL |= VAL >> 1;
  VAL |= VAL >> 2;
  VAL |= VAL >> 4;
  VAL |= VAL >> 8;
  VAL |= VAL >> 16;
#if UINTPTR_MAX > 0xFFFFFFFF
  VAL |= VAL >> 32;
#endif
  VAL++;
  return VAL;
}

bool forgeRingBufferCreate(
  ForgeRingBuffer*        RING,
  size_t                  CAPACITY,
  size_t                  ELEMENT_SIZE,
  bool                    ALLOW_OVERWRITE,
  ForgeLinearAllocator*   ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(RING != NULL, "[RING BUFFER] : Target pointer cannot be NULL");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[RING BUFFER] : Element size must be greater than 0");

  RING->elementSize     = ELEMENT_SIZE;
  RING->capacity        = forgeRoundToPowerOfTwo(CAPACITY);
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
    RING->data = (uint8_t*) FORGE_MALLOC(totalBytes);
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
    if (!RING->allocator) FORGE_FREE(RING->data);
    RING->data = NULL;
  }

  RING->capacity    = 0;
  RING->elementSize = 0;
  RING->head        = 0;
  RING->tail        = 0;
  RING->count       = 0;
  RING->allocator   = NULL;
}
