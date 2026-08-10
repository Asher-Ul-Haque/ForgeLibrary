#include <dataStructures/queue.h>
#include <core/asserts.h>
#include <string.h>

bool queueCreate(
  Queue*            QUEUE,
  size_t            INITIAL_CAPACITY,
  size_t            ELEMENT_SIZE,
  LinearAllocator*  ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot create a NULL queue");
  FORGE_ASSERT_DEBUG_MESSAGE(INITIAL_CAPACITY > 0, "[QUEUE] : Cannot create a queue with 0 INITIAL_CAPACITY");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[QUEUE] : Cannot create a queue with 0 ELEMENT_SIZE");
  QUEUE->head = 0;
  return dynamicArrayCreate(
    &QUEUE->array, 
    INITIAL_CAPACITY, 
    ELEMENT_SIZE, 
    ALLOCATOR);
}

void queueDestroy(Queue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot destroy a NULL queue");
  dynamicArrayDestroy(&QUEUE->array);
  QUEUE->head = 0;
}

bool queueEnqueue(Queue* QUEUE, const void* VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot enqueue in a NULL queue");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[QUEUE] : Cannot enqueue a NULL element");
  return dynamicArrayPush(&QUEUE->array, VALUE_PTR);
}

bool queueDequeue(Queue* QUEUE, void* OUT_VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot dequeue in a NULL queue");

  size_t activeCount = queueSize(QUEUE);
  if (activeCount == 0) return false;

  void* headPtr = dynamicArrayAt(&QUEUE->array, QUEUE->head);
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

void* queuePeek(const Queue* QUEUE) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot peek in a NULL queue");
  if (queueSize(QUEUE) == 0) return NULL;
  return dynamicArrayAt(&QUEUE->array, QUEUE->head);
}

size_t queueSize(const Queue* QUEUE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(QUEUE != NULL, "[QUEUE] : Cannot peek in a NULL queue");
  return QUEUE->array.size - QUEUE->head;
}
