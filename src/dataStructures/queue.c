#include <forgeUtils/core/logger.h>
#include <forgeUtils/memory/linearAlloc.h>
#include <forgeUtils/dataStructures/queue.h>
#include <forgeUtils/memory/tracker.h>
#include <forgeUtils/core/asserts.h>
#include <stdint.h>
#include <string.h>

static inline size_t forgeRoundToPowerOfTwo(size_t VAL)
{
  if (VAL < FORGE_QUEUE_DEFAULT_CAPACITY) return FORGE_QUEUE_DEFAULT_CAPACITY;
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

bool forgeQueueCreate(
  ForgeQueue*           QUEUE,
  size_t                INITIAL_CAPACITY,
  size_t                ELEMENT_SIZE,
  ForgeLinearAllocator* ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot create a NULL queue");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[QUEUE] : Cannot create a queue with 0 ELEMENT_SIZE");

  QUEUE->head         = 0;
  QUEUE->tail         = 0;
  QUEUE->size         = 0;
  QUEUE->capacity     = forgeRoundToPowerOfTwo(INITIAL_CAPACITY);
  QUEUE->elementSize  = ELEMENT_SIZE;
  QUEUE->mask         = QUEUE->capacity - 1;
  QUEUE->allocator    = ALLOCATOR;

  size_t totalBytes = QUEUE->capacity * QUEUE->elementSize;

  if (QUEUE->allocator)
  {
    QUEUE->data = (uint8_t*) forgeLinearAllocAllocate(QUEUE->allocator, totalBytes, 0);
  }
  else
  {
    QUEUE->data = (uint8_t*) FORGE_MALLOC(totalBytes);
  }

  if (!QUEUE->data)
  {
    FORGE_LOG_ERROR("[QUEUE] : Failed to allocate initial queue buffer!");
    QUEUE->capacity = 0;
    QUEUE->mask     = 0;
    return false;
  }

  return true;
}

void forgeQueueDestroy(ForgeQueue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot destroy a NULL queue");

  if (QUEUE->data)
  {
    if (!QUEUE->allocator)
    {
      FORGE_FREE(QUEUE->data);
    }
    QUEUE->data = NULL;
  }

  QUEUE->capacity    = 0;
  QUEUE->mask        = 0;
  QUEUE->size        = 0;
  QUEUE->head        = 0;
  QUEUE->tail        = 0;
  QUEUE->elementSize = 0;
  QUEUE->allocator   = NULL;
}

bool __forgeQueueGrow(ForgeQueue* QUEUE)
{
  size_t    oldCap    = QUEUE->capacity;
  size_t    newCap    = oldCap ? (oldCap * 2) : FORGE_QUEUE_DEFAULT_CAPACITY;
  size_t    newBytes  = newCap * QUEUE->elementSize;
  uint8_t*  newData   = NULL;

  if (QUEUE->allocator)
  {
    newData = (uint8_t*) forgeLinearAllocAllocate(QUEUE->allocator, newBytes, 0);
  }
  else
  {
    newData = (uint8_t*) FORGE_MALLOC(newBytes);
  }

  if (!newData)
  {
    FORGE_LOG_ERROR("[QUEUE] : Failed to allocate memory for queue expansion!");
    return false;
  }

  if (QUEUE->data && QUEUE->size > 0)
  {
    size_t firstPartCount = oldCap - QUEUE->head;
    if (QUEUE->size <= firstPartCount)
    {
      uint8_t* src = (uint8_t*)QUEUE->data + (QUEUE->head * QUEUE->elementSize);
      memcpy(newData, src, QUEUE->size * QUEUE->elementSize);
    }
    else
    {
      size_t firstPartBytes   = firstPartCount * QUEUE->elementSize;
      size_t secondPartBytes  = (QUEUE->size - firstPartCount) * QUEUE->elementSize;

      memcpy(newData, (uint8_t*) QUEUE->data + (QUEUE->head * QUEUE->elementSize), firstPartBytes);
      memcpy(newData + firstPartBytes, QUEUE->data, secondPartBytes);
    }

    if (!QUEUE->allocator)
    {
      FORGE_FREE(QUEUE->data);
    }
  }

  QUEUE->data     = newData;
  QUEUE->head     = 0;
  QUEUE->tail     = QUEUE->size;
  QUEUE->capacity = newCap;
  QUEUE->mask     = newCap - 1;

  return true;
}

bool forgeQueueReserve(ForgeQueue* QUEUE, size_t TARGET_CAPACITY)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot reserve size in a NULL QUEUE");

  size_t newCap = forgeRoundToPowerOfTwo(TARGET_CAPACITY);

  if (newCap < QUEUE->size)                             newCap = forgeRoundToPowerOfTwo(QUEUE->size);
  if (newCap == QUEUE->capacity && QUEUE->data != NULL) return true;

  size_t    newBytes  = newCap * QUEUE->elementSize;
  uint8_t*  newData   = QUEUE->allocator 
                       ? (uint8_t*) forgeLinearAllocAllocate(QUEUE->allocator, newBytes, 0)
                       : (uint8_t*) FORGE_MALLOC(newBytes);

  if (!newData)
  {
    FORGE_LOG_ERROR("[QUEUE] : Failed to allocate buffer for capacity %zu", newCap);
    return false;
  }

  // - - - Unroll ring buffer into clean linear order [0 ... size)
  if (QUEUE->data && QUEUE->size > 0)
  {
    size_t firstPart = QUEUE->capacity - QUEUE->head;
    if (QUEUE->size <= firstPart)
    {
      memcpy(newData, (uint8_t*) QUEUE->data + (QUEUE->head * QUEUE->elementSize), QUEUE->size * QUEUE->elementSize);
    }
    else
    {
      size_t firstBytes   = firstPart * QUEUE->elementSize;
      size_t secondBytes  = (QUEUE->size - firstPart) * QUEUE->elementSize;

      memcpy(newData, (uint8_t*) QUEUE->data + (QUEUE->head * QUEUE->elementSize), firstBytes);
      memcpy(newData + firstBytes, QUEUE->data, secondBytes);
    }

    if (!QUEUE->allocator) FORGE_FREE(QUEUE->data);
  }

  QUEUE->data     = newData;
  QUEUE->head     = 0;
  QUEUE->tail     = QUEUE->size;
  QUEUE->capacity = newCap;
  QUEUE->mask     = newCap - 1;

  return true;
}
