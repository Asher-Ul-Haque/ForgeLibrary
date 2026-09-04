/**
 * @file : hashMap.h 
 * @brief : Simple Hash map in C
 */

#pragma once 

#include <forgeUtils/core/asserts.h>
#include <stddef.h>
#include <stdint.h>
#include <forgeUtils/memory/linearAlloc.h>

#define FORGE_MAP_DEFAULT_CAPACITY  16


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

typedef enum forgeHashMapEntryState 
{
  FORGE_MAP_EMPTY     = 0,
  FORGE_MAP_OCCUPIED  = 1,
  FORGE_MAP_TOMBSTONE = 2
} ForgeHashMapEntryState;

typedef struct forgeHashMap
{
  uint8_t*                slots;          ///< Interleaved flat array of slots
  size_t                  capacity;       ///< Always a power of 2
  size_t                  mask;           ///< capacity - 1
  size_t                  count;          ///< Active key-vale pairs
  size_t                  tombstoneCount; ///< Dead slots 
  size_t                  keySize;        ///< Key size in bytes
  size_t                  valueSize;      ///< Value size in bytes
  size_t                  slotStride;     ///< Total bytes per slot (aligned)
  size_t                  keyOffset;      ///< Byte offset of key inside slot
  size_t                  valueOffset;    ///< Byte offset of value inside slot
  ForgeHashFunction       hashFunction;
  ForgeKeyCompareFunction compareFunction;
  ForgeLinearAllocator*   allocator;
} ForgeHashMap;

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
bool forgeHashmapCreate(
  ForgeHashMap*           MAP,
  size_t                  KEY_SIZE,
  size_t                  VALUE_SIZE,
  size_t                  INITIAL_CAPACITY,
  ForgeHashFunction       HASHER,
  ForgeKeyCompareFunction COMPARATOR,
  ForgeLinearAllocator*   ALLOCATOR);

/**
 * @brief : Destroys the Hash Map and frees backing buffers.
 * @param MAP : Pointer to the map to be destroyed
*/
void forgeHashmapDestroy(ForgeHashMap* MAP);

/**
 * @brief : Inserts or updates a key-value pair.
 * 
 * @param MAP : Pointer to HashMap.
 * @param KEY_PTR : Pointer to key bytes.
 * @param VALUE_PTR : Pointer to value bytes.
 * @return : true if inserted or updated successfully, false on allocation failure.
 */
bool forgeHashmapSet(ForgeHashMap* MAP, const void* KEY_PTR, const void* VALUE_PTR);

/**
 * @brief : Retrieves a value pointer associated with the given key.
 * 
 * @param MAP : Pointer to HashMap.
 * @param KEY_PTR : Pointer to search key.
 * @return : Pointer to value data inside the map, or NULL if key is not found.
 */
void* forgeHashmapGet(const ForgeHashMap* MAP, const void* KEY_PTR);

/**
 * @brief : Removes a key-value pair from the map.
 * 
 * @param MAP : Pointer to HashMap.
 * @param KEY_PTR : Pointer to key bytes to remove.
 * @return : true if key was found and removed, false if not found.
 */
bool forgeHashmapRemove(ForgeHashMap* MAP, const void* KEY_PTR);

/**
 * @brief : Clears all entries without deallocating the underlying buffer.
 * @param MAP : Pointer to the hashmap to be cleared
 */
void forgeHashmapClear(ForgeHashMap* MAP);

/**
 * @brief : Checks if a key exists in the Hash Map.
 * @param MAP : Pointer to the hash map 
 * @param KEY_PTR : Pointer to the key to be searched 
 * @return : true if the hashmap contains the key, false otherwise
 */
static inline bool forgeHashmapContains(const ForgeHashMap* MAP, const void* KEY_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASH MAP] : Cannot check in a NULL MAP");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_PTR != NULL, "[HASH MAP] : Cannot check a NULL KEY_PTR");

  return (forgeHashmapGet(MAP, KEY_PTR) != NULL);
}

/**
 * @brief : Returns total active elements stored.
 * @param MAP : Pointer to the map 
 * @return : Number of elements stored
 */
static inline size_t forgeHashmapSize(const ForgeHashMap* MAP) 
{ 
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASH MAP] : Cannot check in a NULL MAP");

  return MAP->count;
}

/**
 * @brief : Tells whether the map is empty or not
 * @param MAP : Pointer to the map
 * @return : True if empty, false otherwise
*/
static inline bool forgeHashmapIsEmpty(const ForgeHashMap* MAP)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASH MAP] : Cannot check in a NULL MAP");

  return (MAP->count == 0);
}

#define FORGE_HASHMAP_INIT(MAP_PTR, KEY_TYPE, VAL_TYPE, CAP, HASHER, CMP) \
  forgeHashmapCreate((MAP_PTR), sizeof(KEY_TYPE), sizeof(VAL_TYPE), (CAP), (HASHER), (CMP), (NULL))

#define FORGE_HASHMAP_GET(MAP_PTR, VAL_TYPE, KEY_PTR) \
  ((VAL_TYPE*) forgeHashmapGet((MAP_PTR), (KEY_PTR)))

#ifdef __cplusplus
}
#endif

