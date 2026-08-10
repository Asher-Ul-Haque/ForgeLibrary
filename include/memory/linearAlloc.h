/**
 * @file linearAlloc.h 
 * @brief Simple linear allocator in C
 *
 * @see Please keep allocator size as a multiple of 4KB
*/

#pragma once
#include <stdbool.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

#ifndef DEFAULT_ALIGNMENT_BYTES  
  #define DEFAULT_ALIGNMENT_BYTES 16 
#endif

/// @brief Linear Allocator struct 
typedef struct linearAllocator 
{
  size_t totalSize;     ///< The total size that the allocator keeps track of
  size_t allocated;     ///< How much has been used 
  void*  memory;        ///< The actual memory pointer 
  bool   ownsMemory;    ///< Whether this memory was given, or malloced ourselves
  bool   resize;        ///< Whether we ought to be allowed to call realloc on the memory or just let there be an error if allocation fails
} LinearAllocator;

/**
 * @brief creates a linear allocator 
 * @param ALLOCATOR A pointer to the LinearAlloactor struct 
 * @param TOTAL_SIZE The total size that the allocator must have
 * @param MEMORY The memory that the object tracks, pass NULL if you want the object to allocate on its own 
 * @param ALLOW_RESIZE Whether the allocator should be allowed to realloc the memory when full
 * @return true if successful, false if not
*/
bool linearAllocCreate(
  LinearAllocator* ALLOCATOR, 
  size_t           TOTAL_SIZE, 
  void*            MEMORY, 
  bool             ALLOW_RESIZE);

/**
 * @brief deletes a linear allocator 
 * @param ALLOCATOR a pointer to the linear allocator to destroy
*/
void linearAllocDestroy(LinearAllocator* ALLOCATOR);

/**
 * @brief Alloactes memory from the allocator and returns it 
 * @param ALLOACTOR a pointer to the linear allocator from where memory is to be allocated 
 * @param SIZE how much to allocate 
 * @param STRIDE the alignment of the datastructure you are going to store it in, must be a multiple of 2, set 0 for a default of 16
 * @return a pointer to the memory if successful, NULL if fail (for example not being able to resize)
*/
void* linearAllocAllocate(LinearAllocator* ALLOCATOR, size_t SIZE, size_t STRIDE);

/**
 * @brief Frees memory from the allocator 
 * @param ALLOCATOR a pointer to the allocator from which memory is to be freed 
 * @param SIZE how much to free 
*/
void linearAllocFree(LinearAllocator* ALLOCATOR, size_t SIZE);

/**
 * @brief prints debug info on the allocator in debug mode, does nothing in release mode 
 * @param ALLOCATOR a pointer to the allocator to be visualized
*/
void linearAllocDebugPrint(LinearAllocator* ALLOCATOR);

#ifdef __cplusplus
}
#endif
