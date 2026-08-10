/**
 * @file : hashMap.h 
 * @brief : Simple Hash map in C
 */

#pragma once 

#include <stddef.h>
#include <stdint.h>
#include <memory/linearAlloc.h>

#define MAP_DEFAULT_CAPACITY  16
#define MAP_MAX_LOAD_FACTOR   0.75f

#ifndef DEFAULT_ALIGNMENT_BYTES  
  #define DEFAULT_ALIGNMENT_BYTES 16 
#endif


#ifdef __cplusplus
extern "C" {
#endif

/** @brief : Custom hash function signature (if NULL, defaults to FNV-1a)
 * @param KEY : Hash key 
 * @param KEY_SIZE : The size of the key 
 * @return : index
*/
typedef uint64_t (*ForgeHashFunction)(const void* KEY, size_t KEY_SIZE);

/** @brief : Custom key comparison function (if NULL, defaults to memcmp)
 * @param KEY_A : One key 
 * @param KEY_B : Another key 
 * @param KEY_SIZE : The size of the key 
 * @return : comparison result of the two keys
*/
typedef int32_t (*ForgeKeyCompareFunction)(const void* KEY_A, const void* KEY_B, size_t KEY_SIZE);

typedef enum HashMapEntryState 
{
  FORGE_MAP_EMPTY     = 0,
  FORGE_MAP_OCCUPIED  = 1,
  FORGE_MAP_TOMBSTONE = 2
} HashMapEntryState;

typedef struct HashMapEntry 
{
  uint8_t*          key;
  uint8_t*          value;
  uint64_t          hash;
  HashMapEntryState state;
} HashMapEntry;

typedef struct HashMap
{
  HashMapEntry*           entries;
  size_t                  capacity;
  size_t                  count;
  size_t                  tombstoneCount;
  size_t                  keySize;
  size_t                  valueSize;
  ForgeHashFunction       hashFunction;
  ForgeKeyCompareFunction compareFunction;
  LinearAllocator*        allocator;
} HashMap;

/**
 * @brief : Initializes a Hash Map.
 * 
 * @param MAP : Pointer to HashMap struct.
 * @param KEY_SIZE : Size of key in bytes (e.g. sizeof(int) or string pointer size).
 * @param VALUE_SIZE : Size of value in bytes.
 * @param INITIAL_CAPACITY : Initial capacity (rounded up to power of 2, minimum 16).
 * @param HASHER :  Custom hash function or NULL for default FNV-1a.
 * @param COMPARATOR : Custom key compare function or NULL for default memcmp.
 * @param ALLOCATOR : Pointer to linear allocator or NULL for global memory tracker.
 * @return : true if initialized successfully, false otherwise.
 */
bool hashmapCreate(
  HashMap*                MAP,
  size_t                  KEY_SIZE,
  size_t                  VALUE_SIZE,
  size_t                  INITIAL_CAPACITY,
  ForgeHashFunction       HASHER,
  ForgeKeyCompareFunction COMPARATOR,
  LinearAllocator*        ALLOCATOR);

/**
 * @brief : Destroys the Hash Map and frees backing buffers.
 * @param MAP : Pointer to the map to be destroyed
*/
void hashmapDestroy(HashMap* MAP);

/**
 * @brief : Inserts or updates a key-value pair.
 * 
 * @param MAP : Pointer to HashMap.
 * @param KEY_PTR : Pointer to key bytes.
 * @param VALUE_PTR : Pointer to value bytes.
 * @return : true if inserted or updated successfully, false on allocation failure.
 */
bool hashmapSet(HashMap* MAP, const void* KEY_PTR, const void* VALUE_PTR);

/**
 * @brief : Retrieves a value pointer associated with the given key.
 * 
 * @param MAP : Pointer to HashMap.
 * @param KEY_PTR : Pointer to search key.
 * @return : Pointer to value data inside the map, or NULL if key is not found.
 */
void* hashmapGet(const HashMap* MAP, const void* KEY_PTR);

/**
 * @brief : Removes a key-value pair from the map.
 * 
 * @param MAP : Pointer to HashMap.
 * @param KEY_PTR : Pointer to key bytes to remove.
 * @return : true if key was found and removed, false if not found.
 */
bool hashmapRemove(HashMap* MAP, const void* KEY_PTR);

/**
 * @brief : Checks if a key exists in the Hash Map.
 * @param MAP : Pointer to the hash map 
 * @param KEY_PTR : Pointer to the key to be searched 
 * @return : true if the hashmap contains the key, false otherwise
 */
bool hashmapContains(const HashMap* MAP, const void* KEY_PTR);

/**
 * @brief : Clears all entries without deallocating the underlying buffer.
 * @param MAP : Pointer to the hashmap to be cleared
 */
void hashmapClear(HashMap* MAP);

/**
 * @brief : Returns total active elements stored.
 * @param MAP : Pointer to the map 
 * @return : Number of elements stored
 */
static inline size_t hashmapSize(const HashMap* MAP) 
{ return MAP ? MAP->count : 0; }

#ifdef __cplusplus
}
#endif

