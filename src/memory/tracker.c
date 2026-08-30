#include <forgeUtils/memory/tracker.h>
#include <forgeUtils/core/asserts.h>
#include <forgeUtils/core/logger.h>

#include <stdlib.h>
#include <string.h>
#include <stdint.h>


// - - -  Magic numbers for canary guards
#define FORGE_HEADER_MAGIC 0xCAFEBABEU
#define FORGE_FOOTER_MAGIC 0xDEADBEEFU
#define FORGE_FREED_MAGIC  0xDDDDDDDDU

/// @brief : Memory tracker header
typedef struct ForgeMemHeader
{
  uint32_t                magic;
  size_t                  requestedSize;
  const char*             file;
  const char*             func;
  int32_t                 line;
  struct ForgeMemHeader*  next;
  struct ForgeMemHeader*  prev;
} ForgeMemHeader;

// - - - Doubly linked list tracking all active allocations
static ForgeMemHeader*  activeAllocations   = NULL;
static size_t           totalAllocatedBytes = 0;

static inline uintptr_t alignUpPtr(uintptr_t PTR, uintptr_t ALIGNMENT)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALIGNMENT % 2 == 0, "[LINEAR ALLOC] : ALIGNMENT must be a multiple of 2");
  if (ALIGNMENT == 0) ALIGNMENT = DEFAULT_ALIGNMENT_BYTES;
  return (PTR + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}


// - - - Memory Tracker Implementation - - - 

void* forgeTrackedMalloc(
  size_t      SIZE, 
  const char* FILE, 
  const char* FUNCTION, 
  int32_t     LINE) 
{
  if (SIZE == 0) return NULL;

  // - - - Calculate total layout size: [Header] + [User Buffer] + [Padding] + [Footer Magic]
  size_t alignedUserSize  = alignUpPtr(SIZE, sizeof(uintptr_t));
  size_t totalSize        = sizeof(ForgeMemHeader) + alignedUserSize + sizeof(uint32_t);

  ForgeMemHeader* header = (ForgeMemHeader*)malloc(totalSize);
  if (!header) 
  {
    FORGE_LOG_FATAL("[MEMORY TRACKER] : Out of memory allocating %zu bytes at %s:%d", SIZE, FILE, LINE);
    abort();
  }

  header->magic         = FORGE_HEADER_MAGIC;
  header->requestedSize = SIZE;
  header->file          = FILE;
  header->func          = FUNCTION;
  header->line          = LINE;

  // - - - Insert canary footer right after the aligned user payload
  uint8_t*  userPtr = (uint8_t*)  (header + 1);
  uint32_t* footer  = (uint32_t*) (userPtr + alignedUserSize);
  *footer = FORGE_FOOTER_MAGIC;

  // - - - Thread onto tracking list
  header->prev = NULL;
  header->next = activeAllocations;
  if (activeAllocations)  activeAllocations->prev = header; 
  activeAllocations = header;

  totalAllocatedBytes += SIZE;

  return userPtr;
}

void forgeTrackedFree(
  void*       PTR,
  const char* FILE,
  const char* FUNCTION,
  int32_t     LINE)
{
  if (!PTR) return;

  ForgeMemHeader* header = ((ForgeMemHeader*)PTR) - 1;

  // - - - 1. Verify header magic (Detect double frees / wild pointers)
  if (header->magic == FORGE_FREED_MAGIC) 
  {
    FORGE_LOG_FATAL("[MEMORY TRACKER] : DOUBLE FREE DETECTED at %s:%d (Func: %s)", FILE, LINE, FUNCTION);
    abort();
  }
  if (header->magic != FORGE_HEADER_MAGIC) 
  {
     FORGE_LOG_FATAL("[MEMORY TRACKER] : INVALID FREE OR HEADER CORRUPTION at %s:%d (Ptr: %p)", FILE, LINE, PTR);
     abort();
  }

  // - - - 2. Verify footer canary magic (Detect buffer overruns)
  size_t    alignedUserSize = alignUpPtr(header->requestedSize, sizeof(uintptr_t));
  uint32_t* footer          = (uint32_t*)((unsigned char*)PTR + alignedUserSize);

  if (*footer != FORGE_FOOTER_MAGIC) 
  {
    FORGE_LOG_FATAL("[MEMORY TRACKER] : BUFFER OVERRUN DETECTED! Allocation at %s:%d (%zu bytes) was overwritten past boundary!",
                      header->file, header->line, header->requestedSize);
    abort();
  }

  // - - - Unlink from active allocations list
  if (header->prev)                 header->prev->next = header->next;
  if (header->next)                 header->next->prev = header->prev;
  if (activeAllocations == header)  activeAllocations = header->next;

  totalAllocatedBytes  -= header->requestedSize;
  header->magic         = FORGE_FREED_MAGIC;

  free(header);
}

void* forgeTrackedRealloc(
  void*       PTR,
  size_t      NEW_SIZE,
  const char* FILE,
  const char* FUNCTION,
  int32_t     LINE)
{
  if (!PTR) return forgeTrackedMalloc(NEW_SIZE, FILE, FUNCTION, LINE);
  if (NEW_SIZE == 0)
  {
    forgeTrackedFree(PTR, FILE, FUNCTION, LINE);
    return NULL;
  }

  void*           newPtr    = forgeTrackedMalloc(NEW_SIZE, FILE, FUNCTION, LINE);
  ForgeMemHeader* header    = ((ForgeMemHeader*)PTR) - 1;
  size_t          copySize  = (header->requestedSize < NEW_SIZE) ? header->requestedSize : NEW_SIZE;
  
  memcpy(newPtr, PTR, copySize);
  forgeTrackedFree(PTR, FILE, FUNCTION, LINE);

  return newPtr;
}

void* forgeTrackedCalloc(
  size_t      COUNT, 
  size_t      SIZE, 
  const char* FILE, 
  const char* FUNC, 
  int32_t     LINE) 
{
  size_t  totalSize = COUNT * SIZE;
  void*   ptr       = forgeTrackedMalloc(totalSize, FILE, FUNC, LINE);
  if (ptr) 
  {  memset(ptr, 0, totalSize); }
  return ptr;
}
bool forgeMemoryCheckBounds(void)
{
  bool            result  = true;
  ForgeMemHeader* curr    = activeAllocations;

  while (curr) 
  {
    uint8_t*  userPtr         = (uint8_t*) (curr + 1);
    size_t    alignedUserSize = alignUpPtr(curr->requestedSize, sizeof(uintptr_t));
    uint32_t* footer          = (uint32_t*)(userPtr + alignedUserSize);

    if (curr->magic != FORGE_HEADER_MAGIC) 
    {
      FORGE_LOG_FATAL("[MEMORY TRACKER] : Header corrupted for allocation at %s:%d", curr->file, curr->line);
      result = false;
    }
    if (*footer != FORGE_FOOTER_MAGIC) 
    {
      FORGE_LOG_FATAL("[MEMORY TRACKER] : Buffer overrun detected for allocation at %s:%d (%zu bytes)",
                          curr->file, curr->line, curr->requestedSize);
      result = false;
    }
    curr = curr->next;
  }

  return result;
}

void forgeMemoryReportLeaks(void) 
{
  if (!activeAllocations) 
  {
    FORGE_LOG_INFO("[MEMORY TRACKER] No memory leaks detected!");
    return;
  }

  FORGE_LOG_ERROR("[MEMORY TRACKER] MEMORY LEAKS DETECTED:");
  ForgeMemHeader* curr = activeAllocations;
  while (curr) 
  {
    FORGE_LOG_WARNING("  -> Leak of %zu bytes at %s:%d (Function: %s)", 
                    curr->requestedSize, curr->file, curr->line, curr->func);
    curr = curr->next;
  }
}

size_t forgeMemoryGetActiveBytes(void) { return totalAllocatedBytes; }
