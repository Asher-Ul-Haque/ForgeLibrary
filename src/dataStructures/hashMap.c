#include <forgeUtils/dataStructures/hashMap.h>
#include <forgeUtils/core/asserts.h>
#include <forgeUtils/memory/tracker.h>
#include <forgeUtils/core/logger.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static inline uintptr_t alignUpPtr(uintptr_t PTR, uintptr_t ALIGNMENT)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALIGNMENT % 2 == 0, "[LINEAR ALLOC] : ALIGNMENT must be a multiple of 2");
  if (ALIGNMENT == 0) ALIGNMENT = DEFAULT_ALIGNMENT_BYTES;
  return (PTR + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

///@brief : FNV-1a 64-bit Hash Algorithm 
static uint64_t fnv1aHash(const void* KEY, size_t KEY_SIZE)
{
  const uint8_t* bytes = (const uint8_t*) KEY;
  uint64_t hash = 14695981039346656037ULL;
  for (size_t i = 0; i < KEY_SIZE; ++i) 
  {
    hash ^= bytes[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}

static int32_t defaultComparator(
  const void* KEY_A,
  const void* KEY_B,
  size_t      KEY_SIZE)
{ return memcmp(KEY_A, KEY_B, KEY_SIZE); }

static size_t nextPowerOftTwo(size_t N) 
{
  if (N < MAP_DEFAULT_CAPACITY) return MAP_DEFAULT_CAPACITY;
  size_t p = 1;
  while (p < N) p <<= 1;
  return p;
}

/// @brief Helper to allocate individual slot memory contigously 
static bool allocateEntriesBuffer(ForgeHashMap* MAP, size_t CAPACITY, ForgeHashMapEntry** OUT_ENTRIES)
{
  size_t keySizeAligned = alignUpPtr(MAP->keySize, DEFAULT_ALIGNMENT_BYTES);
  size_t valSizeAligned = alignUpPtr(MAP->valueSize, DEFAULT_ALIGNMENT_BYTES);

  size_t entryStructBytes = CAPACITY * sizeof(ForgeHashMapEntry);
  size_t keysPayloadBytes = CAPACITY * keySizeAligned;
  size_t valsPayloadBytes = CAPACITY * valSizeAligned;

  size_t totalBytes = entryStructBytes + keysPayloadBytes + valsPayloadBytes;

  uint8_t* buffer = NULL;
  if (MAP->allocator)
  {
    buffer = (uint8_t*) forgeLinearAllocAllocate(MAP->allocator, totalBytes, DEFAULT_ALIGNMENT_BYTES);
  }
  else 
  {
    buffer = (uint8_t*) FORGE_MALLOC(totalBytes);
  }

  if (!buffer) return false;
  memset(buffer, 0, totalBytes);

  ForgeHashMapEntry* entires = (ForgeHashMapEntry*) buffer;
  uint8_t* keysBase = buffer + entryStructBytes;
  uint8_t* valsBase = keysBase + keysPayloadBytes;

  for (size_t i = 0; i < CAPACITY; ++i)
  {
    entires[i].key = keysBase + (i * keySizeAligned);
    entires[i].value = valsBase + (i * valSizeAligned);
    entires[i].state = FORGE_MAP_EMPTY;
  }

  *OUT_ENTRIES = entires;
  return true;
}

bool forgeHashmapCreate(
  ForgeHashMap*           MAP, 
  size_t                  KEY_SIZE, 
  size_t                  VALUE_SIZE, 
  size_t                  INITIAL_CAPACITY, 
  ForgeHashFunction       HASHER, 
  ForgeKeyCompareFunction COMPARATOR, 
  ForgeLinearAllocator*   ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASHMAP] : Target MAP pointer cannot be NULL");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_SIZE > 0, "[HASHMAP] : KEY_SIZE must be greater than 0");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_SIZE > 0, "[HASHMAP] : VALUE_SIZE must be greater than 0");
  FORGE_ASSERT_DEBUG_MESSAGE(INITIAL_CAPACITY > 0, "[HASHMAP] : INITIAL_CAPACITY must be greater than 0")

  MAP->keySize          = KEY_SIZE;
  MAP->valueSize        = VALUE_SIZE;
  MAP->capacity         = nextPowerOftTwo(INITIAL_CAPACITY);
  MAP->count            = 0;
  MAP->tombstoneCount   = 0;
  MAP->hashFunction     = HASHER ? HASHER : fnv1aHash;
  MAP->compareFunction  = COMPARATOR ? COMPARATOR : defaultComparator;
  MAP->allocator        = ALLOCATOR;

  if (!allocateEntriesBuffer(MAP, MAP->capacity, &MAP->entries))
  {
    FORGE_LOG_ERROR("[HASHMAP] : Failed to allocate memory buffer for Hash Map");
    return false;
  }

  return true;
}

void forgeHashmapDestroy(ForgeHashMap* MAP)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASHMAP] : Cannot destroy a NULL Hashmap");

  if (MAP->entries)
  {
    if (!MAP->allocator) FORGE_FREE(MAP->entries);
    MAP->entries = NULL;
  }

  MAP->capacity       = 0;
  MAP->count          = 0;
  MAP->tombstoneCount = 0;
  MAP->allocator      = NULL;
}

static bool hashmapResize(ForgeHashMap* MAP, size_t NEW_CAPACITY)
{
  ForgeHashMapEntry* oldEntries  = MAP->entries;
  size_t        oldCapacity = MAP->capacity;

  ForgeHashMapEntry* newEntries = NULL;
  if (!allocateEntriesBuffer(MAP, NEW_CAPACITY, &newEntries))
  { return false; }

  MAP->entries        = newEntries;
  MAP->capacity       = NEW_CAPACITY;
  MAP->count          = 0;
  MAP->tombstoneCount = 0;

  // - - - Rehash old entries 
  for (size_t i = 0; i < oldCapacity; ++i)
  {
    if (oldEntries[i].state == FORGE_MAP_OCCUPIED)
    {
      forgeHashmapSet(MAP, oldEntries[i].key, oldEntries[i].value);
    }
  }

  if (!MAP->allocator) FORGE_FREE(oldEntries);

  return true;
}

bool forgeHashmapSet(
  ForgeHashMap* MAP, 
  const void*   KEY_PTR, 
  const void*   VALUE_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASHMAP] : Cannot set on NULL map");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_PTR != NULL, "[HASHMAP] : KEY_PTR cannot be NULL");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[HASHMAP] : VALUE_PTR pointer cannot be NULL");

  // - - - Check load factor (count + tombstoneCount)
  size_t  load        = MAP->count + MAP->tombstoneCount + 1;
  float   loadFactor  = (float) load / (float) MAP->capacity;

  if (loadFactor >= MAP_MAX_LOAD_FACTOR)
  {
    if (!hashmapResize(MAP, MAP->capacity * 2))
    { return false; }
  }

  uint64_t  hash                = MAP->hashFunction(KEY_PTR, MAP->keySize);
  size_t    index               = hash & (MAP->capacity - 1);
  int32_t   firstTombStoneIndex = -1;

  for (size_t i = 0; i < MAP->capacity; ++i)
  {
    size_t        probeIndex  = (index + i) & (MAP->capacity - 1);
    ForgeHashMapEntry* entry       = &(MAP->entries[probeIndex]);

    // - - - Target slot found
    if (entry->state == FORGE_MAP_EMPTY)
    {
      size_t        targetIndex = (firstTombStoneIndex != -1) ? (size_t) firstTombStoneIndex : probeIndex;
      ForgeHashMapEntry* targetEntry = &(MAP->entries[targetIndex]);

      memcpy(targetEntry->key, KEY_PTR, MAP->keySize);
      memcpy(targetEntry->value, VALUE_PTR, MAP->valueSize);
      targetEntry->hash = hash;

      if (targetEntry->state == FORGE_MAP_TOMBSTONE)
      {
        MAP->tombstoneCount--;
      }
      targetEntry->state = FORGE_MAP_OCCUPIED;
      MAP->count++;
      return true;
    }

    // - - - Tombstone found, keep looking
    if (entry->state == FORGE_MAP_TOMBSTONE)
    {
      if (firstTombStoneIndex == -1)
      { firstTombStoneIndex = (int32_t) probeIndex; }
    }

    // - - - occupied 
    else if (entry->state == FORGE_MAP_OCCUPIED)
    {
      // - - - Check if key matches 
      if (entry->hash == hash && MAP->compareFunction(entry->key, KEY_PTR, MAP->keySize) == 0)
      {
        memcpy(entry->value, VALUE_PTR, MAP->valueSize);
        return true;
      }
    }
  }

  return false;
}

void* forgeHashmapGet(const ForgeHashMap* MAP, const void* KEY_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASHMAP] : Cannot search NULL MAP");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_PTR != NULL, "[HASHMAP] : Search KEY_PTR cannot be NULL");

  uint64_t hash   = MAP->hashFunction(KEY_PTR, MAP->keySize);
  size_t   index  = hash & (MAP->capacity - 1);

  for (size_t i = 0; i < MAP->capacity; ++i)
  {
    size_t              probeIndex  = (index + i) & (MAP->capacity - 1);
    const ForgeHashMapEntry* entry  = &MAP->entries[probeIndex];

    if (entry->state == FORGE_MAP_EMPTY) return NULL;

    if (entry->state == FORGE_MAP_OCCUPIED)
    {
      if (entry->hash == hash && MAP->compareFunction(entry->key, KEY_PTR, MAP->keySize) == 0)
      {
        return (void*) entry->value;
      }
    }
  }

  return NULL;
}

bool forgeHashmapRemove(ForgeHashMap* MAP, const void* KEY_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASHMAP] : Cannot remove from NULL MAP");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_PTR != NULL, "[HASHMAP] : Key pointer cannot be NULL");

  if (MAP->count == 0) return false;

  uint64_t  hash  = MAP->hashFunction(KEY_PTR, MAP->keySize);
  size_t    index = hash & (MAP->capacity - 1);

  for (size_t i = 0; i < MAP->capacity; ++i)
  {
    size_t        probeIndex  = (index + i) & (MAP->capacity - 1);
    ForgeHashMapEntry* entry       = &MAP->entries[probeIndex];

    if (entry->state == FORGE_MAP_EMPTY) return false;

    if (entry->state == FORGE_MAP_OCCUPIED)
    {
      if (entry->hash == hash && MAP->compareFunction(entry->key, KEY_PTR, MAP->keySize) == 0)
      {
        entry->state = FORGE_MAP_TOMBSTONE;
        MAP->count--;
        MAP->tombstoneCount++;
        return true;
      }
    }
  }

  return false;
}

bool forgeHashmapContains(const ForgeHashMap* MAP, const void* KEY_PTR)
{ return (forgeHashmapGet(MAP, KEY_PTR) != NULL); }

void forgeHashmapClear(ForgeHashMap* MAP)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASHMAP] : Cannot clear a NULL MAP");
  if (!MAP->entries) return;

  for (size_t i = 0; i < MAP->capacity; ++i)
  {
    MAP->entries[i].state = FORGE_MAP_EMPTY;
  }

  MAP->count          = 0;
  MAP->tombstoneCount = 0;
}
