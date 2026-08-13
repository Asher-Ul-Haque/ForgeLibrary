#include <memory/linearAlloc.h>
#include <memory/tracker.h>
#include <dataStructures/dynamicArray.h>
#include <core/asserts.h>
#include <core/logger.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline uintptr_t alignUpPtr(uintptr_t PTR, uintptr_t ALIGNMENT)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALIGNMENT % 2 == 0, "[LINEAR ALLOC] : ALIGNMENT must be a multiple of 2");
  if (ALIGNMENT == 0) ALIGNMENT = DEFAULT_ALIGNMENT_BYTES;
  return (PTR + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

bool forgeDynamicArrayCreate(ForgeDynamicArray* ARRAY, size_t INITIAL_CAPACITY, size_t ELEMENT_SIZE, ForgeLinearAllocator* ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Target ARRAY pointer cannot be NULL");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[DYNAMIC ARRAY] : Element size must be greater than 0");

  ARRAY->elementSize  = alignUpPtr(ELEMENT_SIZE, DEFAULT_ALIGNMENT_BYTES);
  ARRAY->size         = 0;
  ARRAY->capacity     = (INITIAL_CAPACITY > 0) ? INITIAL_CAPACITY : ARRAY_DEFAULT_CAPACITY;
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

bool forgeDynamicArrayPush(ForgeDynamicArray* ARRAY, const void* VALUE_PTR) 
{
  FORGE_ASSERT_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot push to NULL array");
  FORGE_ASSERT_MESSAGE(VALUE_PTR != NULL, "[DYNAMIC ARRAY] : Cannot push NULL value pointer");

  if (ARRAY->size >= ARRAY->capacity) 
  {
    if (!forgeDynamicArrayReserve(ARRAY, ARRAY->capacity * 2)) 
    {
      return false;
    }
  }

  uint8_t* target = ARRAY->data + (ARRAY->size * ARRAY->elementSize);
  memcpy(target, VALUE_PTR, ARRAY->elementSize);
  ARRAY->size++;

  return true;
}

bool forgeDynamicArrayPop(ForgeDynamicArray* ARRAY, void* OUT_VALUE_PTR) 
{
  FORGE_ASSERT_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot pop from NULL array");

  if (ARRAY->size == 0) return false;

  ARRAY->size--;
  if (OUT_VALUE_PTR) 
  {
    uint8_t* source = ARRAY->data + (ARRAY->size * ARRAY->elementSize);
    memcpy(OUT_VALUE_PTR, source, ARRAY->elementSize);
  }

  return true;
}

void* forgeDynamicArrayAt(const ForgeDynamicArray* ARRAY, size_t INDEX) 
{
  FORGE_ASSERT_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] Cannot access NULL array");
  FORGE_ASSERT_MESSAGE(INDEX < ARRAY->size, "[DYNAMIC ARRAY] Index out of bounds");

  return (void*)(ARRAY->data + (INDEX * ARRAY->elementSize));
}

void forgeDynamicArrayClear(ForgeDynamicArray* ARRAY) 
{
  if (ARRAY) ARRAY->size = 0;
}
