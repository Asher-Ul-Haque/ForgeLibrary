#include <forgeUtils/dataStructures/hashMap.h>
#include <forgeUtils/memory/tracker.h>
#include <forgeUtils/core/logger.h>
#include <stdlib.h>
#include <string.h>

// Slot header: hash + state
typedef struct forgeSlotHeader
{
  uint64_t                hash;
  ForgeHashMapEntryState  state;
} ForgeSlotHeader;

// Fast 64-bit SplitMix-style hash for <=8 byte keys, fallback to block-hash
static uint64_t defaultFastHash(const void* KEY, size_t KEY_SIZE)
{
  const uint8_t* bytes = (const uint8_t*)KEY;
  uint64_t hash = 14695981039346656037ULL;
  for (size_t i = 0; i < KEY_SIZE; ++i)
  {
    hash ^= bytes[i];
    hash *= 1099511628211ULL;
  }
  return hash;
}

static int32_t defaultComparator(const void* A, const void* B, size_t SIZE)
{
  if (SIZE == sizeof(uint64_t))
  {
    return *(const uint64_t*)A == *(const uint64_t*)B ? 0 : 1;
  }
  return memcmp(A, B, SIZE);
}

static inline size_t nextPowerOfTwo(size_t N)
{
  if (N < FORGE_MAP_DEFAULT_CAPACITY) return FORGE_MAP_DEFAULT_CAPACITY;
  N--;
  N |= N >> 1; N |= N >> 2; N |= N >> 4; N |= N >> 8; N |= N >> 16;
#if UINTPTR_MAX > 0xFFFFFFFF
  N |= N >> 32;
#endif
  N++;
  return N;
}

static inline ForgeSlotHeader* getHeader(const ForgeHashMap* MAP, size_t INDEX)
{
  return (ForgeSlotHeader*)(MAP->slots + (INDEX * MAP->slotStride));
}

static inline void* getKeyPtr(const ForgeHashMap* MAP, size_t INDEX)
{
  return (void*)(MAP->slots + (INDEX * MAP->slotStride) + MAP->keyOffset);
}

static inline void* getValPtr(const ForgeHashMap* MAP, size_t INDEX)
{
  return (void*)(MAP->slots + (INDEX * MAP->slotStride) + MAP->valueOffset);
}

static bool allocateSlots(ForgeHashMap* MAP, size_t CAPACITY)
{
  size_t    totalBytes = CAPACITY * MAP->slotStride;
  uint8_t*  buf        = MAP->allocator
                 ? (uint8_t*) forgeLinearAllocAllocate(MAP->allocator, totalBytes, 16)
                 : (uint8_t*) FORGE_MALLOC(totalBytes);

  if (!buf) return false;

  memset(buf, 0, totalBytes);
  MAP->slots = buf;
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
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASH MAP] : Cannot create a NULL Hashmap");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_SIZE > 0, "[HASH MAP] : Cannot create a Hash map with KEY_SIZE under 1");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_SIZE > 0, "[HASH MAP] : Cannot create a Hash map with VALUE_SIZE under 1");

  MAP->keySize         = KEY_SIZE;
  MAP->valueSize       = VALUE_SIZE;
  MAP->capacity        = nextPowerOfTwo(INITIAL_CAPACITY);
  MAP->mask            = MAP->capacity - 1;
  MAP->count           = 0;
  MAP->tombstoneCount  = 0;
  MAP->hashFunction    = HASHER ? HASHER : defaultFastHash;
  MAP->compareFunction = COMPARATOR ? COMPARATOR : defaultComparator;
  MAP->allocator       = ALLOCATOR;

  // - - - Interleaved slot layout: [Header] [Key] [Pad] [Value] [Pad]
  MAP->keyOffset    = sizeof(ForgeSlotHeader);
  size_t keyAligned = (KEY_SIZE + 7) & ~7;
  MAP->valueOffset  = MAP->keyOffset + keyAligned;
  size_t valAligned = (VALUE_SIZE + 7) & ~7;
  MAP->slotStride   = MAP->valueOffset + valAligned;

  if (!allocateSlots(MAP, MAP->capacity))
  {
    FORGE_LOG_ERROR("[HASHMAP] : Failed to allocate slot memory");
    return false;
  }

  return true;
}

void forgeHashmapDestroy(ForgeHashMap* MAP)
{
  FORGE_ASSERT_DEBUG(MAP != NULL);

  if (MAP->slots && !MAP->allocator)
  {
    FORGE_FREE(MAP->slots);
  }

  MAP->slots          = NULL;
  MAP->capacity       = 0;
  MAP->mask           = 0;
  MAP->count          = 0;
  MAP->tombstoneCount = 0;
  MAP->allocator      = NULL;
}

static bool hashmapResize(ForgeHashMap* MAP, size_t NEW_CAPACITY)
{
  uint8_t* oldSlots     = MAP->slots;
  size_t   oldCap       = MAP->capacity;
  size_t   oldStride    = MAP->slotStride;
  size_t   oldKeyOffset = MAP->keyOffset;
  size_t   oldValOffset = MAP->valueOffset;

  MAP->capacity       = NEW_CAPACITY;
  MAP->mask           = NEW_CAPACITY - 1;
  MAP->count          = 0;
  MAP->tombstoneCount = 0;

  if (!allocateSlots(MAP, NEW_CAPACITY)) return false;

  // - - - Re-insert using cached hash without calling hasher or comparator
  for (size_t i = 0; i < oldCap; ++i)
  {
    ForgeSlotHeader* oldHeader = (ForgeSlotHeader*)(oldSlots + (i * oldStride));
    if (oldHeader->state == FORGE_MAP_OCCUPIED)
    {
      const void* key = (const void*)(oldSlots + (i * oldStride) + oldKeyOffset);
      const void* value = (const void*)(oldSlots + (i * oldStride) + oldValOffset);

      size_t index = oldHeader->hash & MAP->mask;
      while (getHeader(MAP, index)->state == FORGE_MAP_OCCUPIED)
      {
        index = (index + 1) & MAP->mask;
      }

      ForgeSlotHeader* newHdr = getHeader(MAP, index);
      newHdr->hash            = oldHeader->hash;
      newHdr->state           = FORGE_MAP_OCCUPIED;

      memcpy(getKeyPtr(MAP, index), key, MAP->keySize);
      memcpy(getValPtr(MAP, index), value, MAP->valueSize);
      MAP->count++;
    }
  }

  if (!MAP->allocator) FORGE_FREE(oldSlots);
  return true;
}

bool forgeHashmapSet(ForgeHashMap* MAP, const void* KEY_PTR, const void* VALUE_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASH MAP] : Cannot set in a NULL MAP");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_PTR != NULL, "[HASH MAP] : Cannot set with a KEY_PTR");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[HASH MAP] : Cannot set with a VALUE_PTR");

  // - - - Integer load factor check: (count + tombstones + 1) >= capacity * 0.75
  if ((MAP->count + MAP->tombstoneCount + 1) * 4 >= MAP->capacity * 3)
  {
    if (!hashmapResize(MAP, MAP->capacity * 2)) return false;
  }

  uint64_t hash      = MAP->hashFunction(KEY_PTR, MAP->keySize);
  size_t   index     = hash & MAP->mask;
  int64_t  firstTomb = -1;

  for (size_t i = 0; i < MAP->capacity; ++i)
  {
    size_t            probeIdx  = (index + i) & MAP->mask;
    ForgeSlotHeader*  header    = getHeader(MAP, probeIdx);

    if (header->state == FORGE_MAP_EMPTY)
    {
      size_t            targetIdx = (firstTomb != -1) ? (size_t)firstTomb : probeIdx;
      ForgeSlotHeader*  targetHdr = getHeader(MAP, targetIdx);

      if (targetHdr->state == FORGE_MAP_TOMBSTONE)
      {
        MAP->tombstoneCount--;
      }

      targetHdr->hash  = hash;
      targetHdr->state = FORGE_MAP_OCCUPIED;

      memcpy(getKeyPtr(MAP, targetIdx), KEY_PTR, MAP->keySize);
      memcpy(getValPtr(MAP, targetIdx), VALUE_PTR, MAP->valueSize);
      MAP->count++;
      return true;
    }

    if (header->state == FORGE_MAP_TOMBSTONE)
    {
      if (firstTomb == -1) firstTomb = (int64_t)probeIdx;
    }
    else if (header->hash == hash)
    {
      if (MAP->compareFunction(getKeyPtr(MAP, probeIdx), KEY_PTR, MAP->keySize) == 0)
      {
        memcpy(getValPtr(MAP, probeIdx), VALUE_PTR, MAP->valueSize);
        return true;
      }
    }
  }

  return false;
}

void* forgeHashmapGet(const ForgeHashMap* MAP, const void* KEY_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASH MAP] : Cannot get in a NULL MAP");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_PTR != NULL, "[HASH MAP] : Cannot get with a KEY_PTR");

  uint64_t hash = MAP->hashFunction(KEY_PTR, MAP->keySize);
  size_t   idx  = hash & MAP->mask;

  for (size_t i = 0; i < MAP->capacity; ++i)
  {
    size_t            probeIdx  = (idx + i) & MAP->mask;
    ForgeSlotHeader*  header    = getHeader(MAP, probeIdx);

    if (header->state == FORGE_MAP_EMPTY) return NULL;

    if (header->state == FORGE_MAP_OCCUPIED && header->hash == hash)
    {
      if (MAP->compareFunction(getKeyPtr(MAP, probeIdx), KEY_PTR, MAP->keySize) == 0)
      {
        return getValPtr(MAP, probeIdx);
      }
    }
  }

  return NULL;
}

bool forgeHashmapRemove(ForgeHashMap* MAP, const void* KEY_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASH MAP] : Cannot remove from a NULL MAP");
  FORGE_ASSERT_DEBUG_MESSAGE(KEY_PTR != NULL, "[HASH MAP] : Cannot remove with a NULL KEY_PTR");

  if (MAP->count == 0) return false;

  uint64_t hash   = MAP->hashFunction(KEY_PTR, MAP->keySize);
  size_t   index  = hash & MAP->mask;

  for (size_t i = 0; i < MAP->capacity; ++i)
  {
    size_t            probeIdx  = (index + i) & MAP->mask;
    ForgeSlotHeader*  header    = getHeader(MAP, probeIdx);

    if (header->state == FORGE_MAP_EMPTY) return false;

    if (header->state == FORGE_MAP_OCCUPIED && header->hash == hash)
    {
      if (MAP->compareFunction(getKeyPtr(MAP, probeIdx), KEY_PTR, MAP->keySize) == 0)
      {
        header->state = FORGE_MAP_TOMBSTONE;
        MAP->count--;
        MAP->tombstoneCount++;
        return true;
      }
    }
  }

  return false;
}

void forgeHashmapClear(ForgeHashMap* MAP)
{
  FORGE_ASSERT_DEBUG_MESSAGE(MAP != NULL, "[HASH MAP] : Cannot clear a NULL MAP");
  if (!MAP->slots) return;

  memset(MAP->slots, 0, MAP->capacity * MAP->slotStride);
  MAP->count          = 0;
  MAP->tombstoneCount = 0;
}
