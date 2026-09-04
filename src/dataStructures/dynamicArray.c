#include <forgeUtils/core/logger.h>
#include <forgeUtils/memory/tracker.h>
#include <forgeUtils/dataStructures/dynamicArray.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool forgeDynamicArrayCreate(ForgeDynamicArray* ARRAY, size_t INITIAL_CAPACITY, size_t ELEMENT_SIZE, ForgeLinearAllocator* ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot create a NULL ARRAY");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[DYNAMIC ARRAY] : Element size must be greater than 0");

  ARRAY->elementSize  = ELEMENT_SIZE;
  ARRAY->size         = 0;
  ARRAY->capacity     = (INITIAL_CAPACITY > 0) ? INITIAL_CAPACITY : FORGE_ARRAY_DEFAULT_CAPACITY;
  ARRAY->allocator    = ALLOCATOR;

  size_t totalBytes = ARRAY->capacity * ARRAY->elementSize;

  if (ARRAY->allocator) 
  {
    ARRAY->data = (uint8_t*) forgeLinearAllocAllocate(ARRAY->allocator, totalBytes, 0);
  }
  else 
  {
    ARRAY->data = (uint8_t*) FORGE_MALLOC(totalBytes);
  }

  if (!ARRAY->data)
  {
    FORGE_LOG_ERROR("[DYNAMIC ARRAY] : Failed to allocate memory for array!");
    ARRAY->capacity = 0;
    return false;
  }

  return true;
}

void forgeDynamicArrayDestroy(ForgeDynamicArray* ARRAY)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot destroy a NULL array.");

  if (ARRAY->data)
  {
    if (!ARRAY->allocator) FORGE_FREE(ARRAY->data);
    ARRAY->data = NULL;
  }

  ARRAY->capacity     = 0;
  ARRAY->size         = 0;
  ARRAY->elementSize  = 0;
  ARRAY->allocator    = NULL;
}

bool forgeDynamicArrayReserve(ForgeDynamicArray* ARRAY, size_t MIN_CAPACITY)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot reserve capcity in a NULL ARRAY");

  if (MIN_CAPACITY <= ARRAY->capacity) return true;

  size_t newCapacity = ARRAY->capacity * 2;
  if (newCapacity < MIN_CAPACITY) newCapacity = MIN_CAPACITY;

  size_t    newBytes  = newCapacity * ARRAY->elementSize;
  uint8_t*  newData   = NULL;

  // - - - allocate new chunk from Linear Allocator and copy existing data
  if (ARRAY->allocator)
  {
    newData = (uint8_t*) forgeLinearAllocAllocate(ARRAY->allocator, newBytes, 0);
    if (!newData)
    {
      FORGE_LOG_ERROR("[DYNAMIC ARRAY] : Failed to allocate new chunks via Linear Allocator");
      return false;
    }
    memcpy(newData, ARRAY->data, ARRAY->size * ARRAY->elementSize);
  }
  else 
  {
    newData = FORGE_REALLOC(ARRAY->data, newBytes);
  }

  if (!newData)
  {
    FORGE_LOG_ERROR("[DYNAMIC ARRAY] : Failed to expenad array capacity!");
    return false;
  }

  ARRAY->data     = newData;
  ARRAY->capacity = newCapacity;
  return true;
}

bool __forgeDynamicArrayGrow(ForgeDynamicArray* ARRAY)
{
  FORGE_ASSERT_DEBUG(ARRAY != NULL);

  // 2x growth, or initial default if capacity was zero
  size_t newCapacity = ARRAY->capacity ? (ARRAY->capacity * 2) : FORGE_ARRAY_DEFAULT_CAPACITY;
  return forgeDynamicArrayReserve(ARRAY, newCapacity);
}

bool forgeDynamicArrayPushRange(ForgeDynamicArray* ARRAY, const void* SRC_BUFFER, size_t COUNT)
{
  FORGE_ASSERT_DEBUG(ARRAY != NULL);
  if (!SRC_BUFFER || COUNT == 0) return true;

  size_t requiredCapacity = ARRAY->size + COUNT;
  if (requiredCapacity > ARRAY->capacity)
  {
    size_t targetCapacity = ARRAY->capacity ? ARRAY->capacity : FORGE_ARRAY_DEFAULT_CAPACITY;
    while (targetCapacity < requiredCapacity) 
    {
      targetCapacity *= 2;
    }

    if (!forgeDynamicArrayReserve(ARRAY, targetCapacity)) 
    {
      return false;
    }
  }

  uint8_t* dest = ARRAY->data + (ARRAY->size * ARRAY->elementSize);
  memcpy(dest, SRC_BUFFER, COUNT * ARRAY->elementSize);
  ARRAY->size += COUNT;

  return true;
}

bool forgeDynamicArrayShrinkToFit(ForgeDynamicArray* ARRAY)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot shrink a NULL ARRAY");

  // - - - Linear allocators cannot free or shrink intermediate allocations
  if (ARRAY->allocator) 
  {
    FORGE_LOG_ERROR("[DYNAMIC ARRAY] : Cannot shrink an ARRAY that uses a linear allocator");
    return false;
  }

  size_t targetCap = ARRAY->size > 0 ? ARRAY->size : FORGE_ARRAY_DEFAULT_CAPACITY;
  if (targetCap >= ARRAY->capacity) return true;

  size_t  newBytes  = targetCap * ARRAY->elementSize;
  void*   newData   = FORGE_REALLOC(ARRAY->data, newBytes);
  if (!newData)
  {
    FORGE_LOG_ERROR("[DYNAMIC ARRAY] : Failed to shrink array buffer!");
    return false;
  }

  ARRAY->data     = newData;
  ARRAY->capacity = targetCap;
  return true;
}
