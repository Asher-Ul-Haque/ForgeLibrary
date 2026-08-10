#include <memory/linearAlloc.h>
#include <core/asserts.h>
#include <core/logger.h>
#include <stdalign.h>
#include <stdint.h>
#include <stdlib.h>

static inline uintptr_t alignUpPtr(uintptr_t PTR, uintptr_t ALIGNMENT)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALIGNMENT % 2 == 0, "[LINEAR ALLOC] : ALIGNMENT must be a multiple of 2");
  if (ALIGNMENT == 0) ALIGNMENT = DEFAULT_ALIGNMENT_BYTES;
  return (PTR + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

bool linearAllocCreate(
  LinearAllocator* ALLOCATOR, 
  size_t           TOTAL_SIZE, 
  void*            MEMORY, 
  bool             ALLOW_RESIZE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALLOCATOR, "[LINEAR ALLOCATOR] : Cannot create an allocator which is null");
  FORGE_ASSERT_DEBUG_MESSAGE(TOTAL_SIZE > 0,  "[LINEAR ALLOCATOR] : Cannot create an allocator with 0 TOTAL_SIZE");

  ALLOCATOR->totalSize  = TOTAL_SIZE;
  ALLOCATOR->allocated  = 0;
  ALLOCATOR->resize     = ALLOW_RESIZE;
  ALLOCATOR->ownsMemory = (MEMORY == NULL);

  if (ALLOCATOR->ownsMemory)
  {
    ALLOCATOR->memory = malloc(ALLOCATOR->totalSize);
    if (ALLOCATOR->memory == NULL) 
    {
      FORGE_LOG_ERROR("[LINEAR ALLOCATOR] : Failed to allocate memory %ulld", ALLOCATOR->totalSize);
      return false;
    }
  }
  else ALLOCATOR->memory = MEMORY;

  FORGE_LOG_TRACE("[LINEAR ALLOCATOR] : Created a linear allocator with %ulld size", ALLOCATOR->totalSize);
  return true;
}

void linearAllocDestroy(LinearAllocator* ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALLOCATOR, "[LINEAR ALLOCATOR] : Cannot create an allocator which is null");

  if (ALLOCATOR->ownsMemory) 
  {
    free(ALLOCATOR->memory);
    ALLOCATOR->memory = false;
  }

  ALLOCATOR->totalSize = 0;
  ALLOCATOR->allocated = 0;

  FORGE_LOG_TRACE("[LINEAR ALLOCATOR] : Destroyed a linear allocator");
}

void* linearAllocAllocate(LinearAllocator* ALLOCATOR, size_t SIZE, size_t ALIGNMENT)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALLOCATOR != NULL, "[LINEAR ALLOCATOR] : Linear Allocator cannot be null");

  if (SIZE == 0)
  {
    FORGE_LOG_ERROR("[LINEAR ALLOCATOR] : Cannot allocate memory with 0 size")
    return NULL; 
  }

  uintptr_t currentAddr       = (uintptr_t) ALLOCATOR->memory + ALLOCATOR->allocated;
  uintptr_t alignedAddr       = alignUpPtr(currentAddr, ALIGNMENT);
  size_t    alignmentPadding  = alignedAddr - currentAddr;
  size_t    requiredBytes     = SIZE + alignmentPadding;

  // - - - Check if allocation exceeds remaining capacity
  if (ALLOCATOR->allocated + requiredBytes > ALLOCATOR->totalSize)
  {
    // - - - expand capacity 
    if (ALLOCATOR->resize && ALLOCATOR->ownsMemory)
    {
      size_t newCapacity = ALLOCATOR->totalSize * 2;
      if (newCapacity < ALLOCATOR->allocated + requiredBytes)
      { newCapacity = ALLOCATOR->allocated + requiredBytes; }

      void* newMem = realloc(ALLOCATOR->memory, newCapacity);
      if (!newMem) 
      {
        FORGE_LOG_ERROR("[LINEAR ALLOC] : Resize with realloc failed");
        return NULL;
      }

      ALLOCATOR->memory     = newMem;
      ALLOCATOR->totalSize  = newCapacity;

      // - - - Recalculate pointers after buffer move 
      currentAddr = (uintptr_t)ALLOCATOR->memory + ALLOCATOR->allocated;
      alignedAddr = alignUpPtr(currentAddr, ALIGNMENT);
      alignmentPadding = alignedAddr - currentAddr;
      requiredBytes = SIZE + alignmentPadding;
    }
    else 
    {
      FORGE_LOG_ERROR("[LINEAR ALLOC] : Out of memory and cannot resize");
      return NULL;
    }
  }

  ALLOCATOR->allocated += requiredBytes;
  return (void*) alignedAddr;
}

void linearAllocFree(LinearAllocator* ALLOCATOR, size_t SIZE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALLOCATOR != NULL, "[LINEAR ALLOCATOR] : Cannot free from a null LinearAllocator");

  if (SIZE >= ALLOCATOR->allocated) ALLOCATOR->allocated  = 0;
  else                              ALLOCATOR->allocated -= SIZE;

  // - - - Check if resize is needed 
  if (!ALLOCATOR->resize)                               return;
  if (ALLOCATOR->allocated >= ALLOCATOR->totalSize / 2) return;

  uintptr_t newCapacity = ALLOCATOR->totalSize / 2;
  if (newCapacity < 1) newCapacity = 1;
  void* newMem = realloc(ALLOCATOR->memory, newCapacity);
  if (!newMem) 
  {
    FORGE_LOG_ERROR("[LINEAR ALLOC] : Resize with realloc failed");
    return;
  }

  ALLOCATOR->memory     = newMem;
  ALLOCATOR->totalSize  = newCapacity;
}

void linearAllocDebugPrint(LinearAllocator* ALLOCATOR)
{
  #ifdef DEBUG 
    #include <memory.h>
    FORGE_ASSERT_DEBUG_MESSAGE(ALLOCATOR != NULL, "[LINEAR ALLOCATOR] : Cannot debug print a NULL ALLOCATOR");

    size_t BAR_WIDTH = 50;

    size_t used = ALLOCATOR->allocated;
    if (used > ALLOCATOR->totalSize) used = ALLOCATOR->totalSize;

    size_t freeBytes = ALLOCATOR->totalSize - used;

    double usedPercent = 0.0;
    double freePercent = 0.0;

    if (ALLOCATOR->totalSize > 0)
    {
      usedPercent = 100.0 * (double)used / (double)ALLOCATOR->totalSize;
      freePercent = 100.0 - usedPercent;
    }

    size_t filled = (ALLOCATOR->totalSize == 0)
      ? 0
      : (used * BAR_WIDTH) / ALLOCATOR->totalSize;

    char bar[BAR_WIDTH + 1];

    memset(bar, '_', BAR_WIDTH);
    memset(bar, '*', filled);

    if (filled < BAR_WIDTH) bar[filled] = '@';

    bar[(int)BAR_WIDTH] = '\0';

    FORGE_LOG_INFO(
      "Pool [%s]\n"
      "Capacity : %zu bytes\n"
      "Used     : %zu (%.1f%%)\n"
      "Free     : %zu (%.1f%%)",
      bar,
      ALLOCATOR->totalSize,
      used,
      usedPercent,
      freeBytes,
      freePercent);
  #elif 
    (void)ALLOCATOR;
  #endif
}
