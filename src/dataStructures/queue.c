#include <dataStructures/queue.h>
#include <core/asserts.h>
#include <string.h>

bool forgeQueueCreate(
  ForgeQueue*            QUEUE,
  size_t            INITIAL_CAPACITY,
  size_t            ELEMENT_SIZE,
  ForgeLinearAllocator*  ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot create a NULL queue");
  FORGE_ASSERT_DEBUG_MESSAGE(INITIAL_CAPACITY > 0, "[QUEUE] : Cannot create a queue with 0 INITIAL_CAPACITY");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[QUEUE] : Cannot create a queue with 0 ELEMENT_SIZE");
  QUEUE->head = 0;
  return forgeDynamicArrayCreate(
    &QUEUE->array, 
    INITIAL_CAPACITY, 
    ELEMENT_SIZE, 
    ALLOCATOR);
}

void forgeQueueDestroy(ForgeQueue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot destroy a NULL queue");
  forgeDynamicArrayDestroy(&QUEUE->array);
  QUEUE->head = 0;
}

bool forgeQueueEnqueue(ForgeQueue* QUEUE, const void* VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot enqueue in a NULL queue");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[QUEUE] : Cannot enqueue a NULL element");
  return forgeDynamicArrayPush(&QUEUE->array, VALUE_PTR);
}

bool forgeQueueDequeue(ForgeQueue* QUEUE, void* OUT_VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot dequeue in a NULL queue");

  size_t activeCount = forgeQueueSize(QUEUE);
  if (activeCount == 0) return false;

  void* headPtr = forgeDynamicArrayAt(&QUEUE->array, QUEUE->head);
  if (OUT_VALUE_PTR) 
  {
    memcpy(OUT_VALUE_PTR, headPtr, QUEUE->array.elementSize);
  }

  QUEUE->head++;

  // - - - Maintenance Compact Step: If all elements were dequeued, reset cursors back to 0
  if (QUEUE->head == QUEUE->array.size) 
  {
    QUEUE->head = 0;
    QUEUE->array.size = 0;
  }

  // - - - Compact buffer if dead space at front exceeds 128 elements to avoid memory drift
  else if (QUEUE->head > 128 && QUEUE->head > (QUEUE->array.size / 2)) 
  {
    size_t remaining = QUEUE->array.size - QUEUE->head;
    uint8_t* base = QUEUE->array.data;
    memmove(base, base + (QUEUE->head * QUEUE->array.elementSize), remaining * QUEUE->array.elementSize);
    QUEUE->array.size = remaining;
    QUEUE->head = 0;
  }

  return true;
}

void* forgeQueuePeek(const ForgeQueue* QUEUE) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot peek in a NULL queue");
  if (forgeQueueSize(QUEUE) == 0) return NULL;
  return forgeDynamicArrayAt(&QUEUE->array, QUEUE->head);
}

size_t forgeQueueSize(const ForgeQueue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot peek in a NULL queue");
  return QUEUE->array.size - QUEUE->head;
}
