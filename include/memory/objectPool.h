/**
 * @file objectPool.h 
 * @brief Great memory management tool for objects of the same size
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

#define POOL_END_OF_LIST ((size_t)-1)

/// @brief ObjectPool metadata
typedef struct ObjectPool
{
  size_t  stride;         ///< Stride in bytes per slot including alignment padding, calculated by the pool, not part of the config
  size_t  objectSize;     ///< Size of the object in bytes that you intend to store in the pool 
  size_t  capacity;       ///< how many objects should this track
  size_t  freeListOffset; ///< Offset in memory to the head of free list ((u64)-1 if full)
  size_t  freeCount;      ///< Number of currently free slots
  void*   memory;         ///< Backing memory block
  bool    ownsMemory;     ///< Indicates if the pool allocated the memory itself or did you the user pass it
} ObjectPool;

/**
 * @brief ObjectPool creation function, the struct itself is the config, set fields before calling this function 
 * @param POOL a pointer to the pool to be initialized, also acts as the config 
 * @param CAPACITY how many objects should be in the pool 
 * @param OBJECT_SIZE size of an object in buyes 
 * @param MEMORY the backing memory, pass NULL if you want the pool to handle memory itself
 * @return True on success, false on fail
*/
bool objectPoolCreate(
  ObjectPool* POOL,
  size_t      CAPACITY,
  size_t      OBJECT_SIZE,
  void*       MEMORY);

/**
 * @brief Takes an object from object pool 
 * @param POOL a pointer to the pool from which the object is to be taken 
 * @return Pointer to object, or NULL if full and cannot resize
*/
void* objectPoolTakeObject(ObjectPool* POOL);

/**
 * @brief Returns an object back to the pool for reuse 
 * @param POOL the pool to be returned to 
 * @param OBJECT the object to be returned
*/
void objectPoolReturnObject(ObjectPool* POOL, void* OBJECT);

/**
 * @brief Destroys the Object Pool and frees backing memory if owned 
 * @param POOL Pointer to the pool to be destroyed 
*/
void objectPoolDestroy(ObjectPool* POOL);

/**
 * @brief debug pritns the object pool in debug mode, does nothing in release mode 
 * @param POOL a pointer to the pool to be visualized
*/
void objectPoolDebugPrint(ObjectPool* POOL);


#ifdef __cplusplus
}
#endif
