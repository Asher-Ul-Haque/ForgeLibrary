/**
 * @file : tracker.h 
 * @brief : Overwrites of malloc, realloc and free to have memory tracking in debug mode
*/

#pragma once 

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifndef DEFAULT_ALIGNMENT_BYTES  
  #define DEFAULT_ALIGNMENT_BYTES 16 
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief : Allocates memory with surrounding magic canary guards and tracking metadata.
 * @param SIZE : how much to allocate 
 * @param FILE : which file is it allocated in 
 * @param FUNCTION : Which function is it allocated in
 * @param LINE : Which line is it allocated in
 */
void* forgeTrackedMalloc(size_t SIZE, const char* FILE, const char* FUNC, int32_t LINE);

/**
 * @brief : Reallocates memory, updating canary guards and size tracking.
 * @param PTR : Ptr to realloc 
 * @param NEW_SIZE : new allocation size
 * @param FILE : What file is the reallocate in 
 * @param FUNCTION : What function is the reallocate in 
 * @param LINE : What line is the reallocate in
*/
void* forgeTrackedRealloc(void* PTR, size_t NEW_SIZE, const char* FILE, const char* FUNCTION, int32_t LINE);

/**
 * @brief : Allocates zero-initialized memory with canary guards and tracking metadata.
 * @param COUNT : How many objects 
 * @param SIZE : Size of one object 
 * @param FILE : What file is the callocate in 
 * @param FUNCTION : What function is the callocate in 
 * @param LINE : What line is the callocate in
 */
void* forgeTrackedCalloc(
  size_t      COUNT, 
  size_t      SIZE, 
  const char* FILE, 
  const char* FUNC, 
  int32_t     LINE);

/**
 * @brief : Frees memory and verifies Canary safety bounds.
 * @param PTR : What ptr to free 
 * @param FILE : What file is free called in 
 * @param FUNCTION : What function is the free in 
 * @param LINE : What line is the free in
 */
void forgeTrackedFree(void* PTR, const char* FILE, const char* FUNCTION, int32_t LINE);

/**
 * @brief : Validates all active allocations against canary corruption.
 * @return : true if all allocations are intact, false if corruption detected.
 */
bool forgeMemoryCheckBounds(void);

/// @brief Reports all active allocations that haven't been freed (Memory Leaks).
void forgeMemoryReportLeaks(void);

/**
 * @brief : Returns total active allocated bytes currently in use.
 * @return : total active allocated bytes in use
 */
size_t forgeMemoryGetActiveBytes(void);

// - - - Optional Macro Overrides for Debug Mode
#ifdef FORGE_ENABLE_MEMORY_TRACKING
  #define FORGE_MALLOC(size)         forgeTrackedMalloc((size), __FILE__, __func__, __LINE__)
  #define FORGE_REALLOC(ptr, size)   forgeTrackedRealloc((ptr), (size), __FILE__, __func__, __LINE__)
  #define FORGE_CALLOC(count, size)  forgeTrackedCalloc((count), (size), __FILE__, __func__, __LINE__)
  #define FORGE_FREE(ptr)            forgeTrackedFree((ptr), __FILE__, __func__, __LINE__)
#else 
  #define FORGE_MALLOC(size) malloc(size)
  #define FORGE_REALLOC(ptr, size) realloc(ptr, size)
  #define FORGE_CALLOC(count, size) calloc((count), (size))
  #define FORGE_FREE(ptr) free(ptr)
#endif

