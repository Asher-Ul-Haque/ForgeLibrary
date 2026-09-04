/**
 * @file : dynamicArray.h 
 * @brief : Dynamic Array implementation in C
*/

#pragma once 

#include <forgeUtils/memory/linearAlloc.h>
#include <forgeUtils/core/asserts.h>
#include <forgeUtils/core/logger.h>
#include <stdint.h>
#include <memory.h>
#ifdef __cplusplus 
extern "C" {
#endif 

#ifndef DEFAULT_ALIGNMENT_BYTES  
  #define DEFAULT_ALIGNMENT_BYTES 16 
#endif

#define FORGE_ARRAY_DEFAULT_CAPACITY 8 

/// @brief Dynamic Array : similar to std::vector in c++
typedef struct forgeDynamicArray
{
  uint8_t*                data;         ///< POinter to contiguois element memory 
  size_t                  capacity;     ///< Total number of elements allocated
  size_t                  size;         ///< Current number of elements stored
  size_t                  elementSize;  ///< Size of an individual element in bytes
  ForgeLinearAllocator*   allocator;    ///< Optional custom linear allocator, NULL for the vector to manage its own memory
} ForgeDynamicArray;


// - - - C API - - - 

/**
 * @brief : Creates a dynamic array instance.
 * @param ARRAY: Pointer to dynamic array struct 
 * @param INITIAL_CAPACITY : Initial element capacity (0 defaults to 8)
 * @param ELEMENT_SIZE : Size of each element in bytes 
 * @param ALLOCATOR : Pointer to linear allocator or NULL for the dynamic array to allocate memory on its own 
 * @return true if initialized successfully, false otherwise
*/
bool forgeDynamicArrayCreate(
  ForgeDynamicArray*    ARRAY, 
  size_t                INITIAL_CAPACITY, 
  size_t                ELEMENT_SIZE, 
  ForgeLinearAllocator* ALLOCATOR);

/**
 * @brief : Destroys the dynamic array and releases memory if owned. 
 * @param ARRAY : Pointer to the dynamic array to be destroyed
*/
void forgeDynamicArrayDestroy(ForgeDynamicArray* ARRAY);

/**
 * @brief : Ensures capacity exists for at least MIN_CAPACITY elements.
 * @param ARRAY : The array to be reserved 
 * @param MIN_CAPACITY : How much to reserve as count of elements 
 * @return : True if successfull and false if not
*/
bool forgeDynamicArrayReserve(ForgeDynamicArray* ARRAY, size_t MIN_CAPACITY);

/**
 * @brief : Internal slow-path growth function
 * @warning: Internal function
 * @param ARRAY : The array to be grown
 * @return : True if succesful and false if not
 */
bool __forgeDynamicArrayGrow(ForgeDynamicArray* ARRAY);

/**
 * @brief : Appends a contigous range of elements via a single block memcpy
 * @param ARRAY : The dynamic array to which the range is to be pushed.
 * @param SRC_BUFFER : Pointer to the source to be copied
 * @param COUNT : How many elements in the buffer
 * @return : True if succesful, false if not
 */
bool forgeDynamicArrayPushRange(ForgeDynamicArray* ARRAY, const void* SRC_BUFFER, size_t COUNT);

/**
 * @brief : Reserves a slot at the end and returns a direct pointer to unitialized element memory. Enables zero-copy costruction directly into array storage, and bypasses memcpy
 * @param ARRAY : The dynamic array pointer
 * @return : Pointer to the unitialized element
 * @warning : Does not intialize the element, use the pointer to initialize
 */
static inline void* forgeDynamicArrayEmplace(ForgeDynamicArray* ARRAY)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot emplace in a NULL array");

  if (ARRAY->size >= ARRAY->capacity)
  {
    if (!__forgeDynamicArrayGrow(ARRAY)) return NULL;

    void* slot = ARRAY->data + (ARRAY->size * ARRAY->elementSize);
    ARRAY->size++;
    return slot;
  }

  void* slot = ARRAY->data + (ARRAY->size * ARRAY->elementSize);
  ARRAY->size++;
  return slot;
}

/**
 * @brief : Pushes a new element value to the back of the array.
 * @param ARRAY : The dynamic array to which the value is to be pushed 
 * @param VALUE_PTR : Pointer to the value being stored
 * @warning : VALUE_PTR's value will be copied
 * @return : True if push was succesful, false if not 
*/
static inline bool forgeDynamicArrayPush(ForgeDynamicArray* ARRAY, const void* VALUE_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot push into null ARRAY");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[DYNAMIC ARRAY] : Cannot push a null VALUE_PTR");

  void* slot = forgeDynamicArrayEmplace(ARRAY);
  if (!slot) return false;

  memcpy(slot, VALUE_PTR, ARRAY->elementSize);
  return true;
}

/**
 * @brief : Pops the last element from the array 
 * @param OUT_VALUE_PTR : Optional pointer to receive the popped element bytes
 * @warning : the size of OUT_VALUE_PTR should be big enough to store the element
 * @return : whether the pop was successfull
*/
static inline bool forgeDynamicArrayPop(ForgeDynamicArray* ARRAY, void* OUT_VALUE_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot Pop from a NULL ARRAY");

  if (ARRAY->size == 0)
  {
    FORGE_LOG_WARNING("[DYNAMIC ARRAY] : The size of the ARRAY is 0, cannot pop");
    return false;
  }

  ARRAY->size--;
  if (OUT_VALUE_PTR)
  {
    uint8_t* source = ARRAY->data + (ARRAY->size * ARRAY->elementSize);
    memcpy(OUT_VALUE_PTR, source, ARRAY->elementSize);
  }

  // - - - Automatic downscale (heap-only; linear allocators cannot shrink)
  if (!ARRAY->allocator && ARRAY->capacity > FORGE_ARRAY_DEFAULT_CAPACITY)
  {
    if (ARRAY->size <= ARRAY->capacity / 4)
    {
      forgeDynamicArrayReserve(ARRAY, ARRAY->capacity / 2);
    }
  }

  return true;
}

/**
 * @brief : Returns a pointer to the element at the given index 
 * @param ARRAY : Pointer to the array which is to be accessed
 * @param INDEX : The index to access at 
 * @warning : The index must be in bounds
 * @warning : Since this returns a void*, you can override it directly, but be careful, since you get access to the memory underneath
 * @return : A pointer to the object in the array at the given index
*/
static inline void* forgeDynamicArrayAt(const ForgeDynamicArray* ARRAY, size_t INDEX)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot access a NULL ARRAY");
  FORGE_ASSERT_DEBUG_MESSAGE(INDEX < ARRAY->size, "[DYNAMIC ARRAY] : INDEX out of bounds");

  return (void*) (ARRAY->data + (INDEX * ARRAY->elementSize));
}

/**
 * @brief : Clears all elements without freeing memory.
 * @param ARRAY : A pointer to the array to be cleared
*/
static inline void forgeDynamicArrayClear(ForgeDynamicArray* ARRAY)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ARRAY != NULL, "[DYNAMIC ARRAY] : Cannot clear a NULL ARRAY");
  ARRAY->size = 0;
}

/**
 * @brief : Shrinks the array on demand
 * @param ARRAY : Pointer to the dynamic array to be shrunk
 * @return : True if the array shrunk, false otherwise
 */
bool forgeDynamicArrayShrinkToFit(ForgeDynamicArray* ARRAY);


// - - - Helper Macros for Ergonomic Usage - - - 

/**
 * @brief : Helper macro to initialize array with implicit type sizing 
 * @see dynamicArrayCreate
*/
#define FORGE_ARRAY_INIT(ARRAY_PTR, CAPACITY, TYPE) \
  forgeDynamicArrayCreate((ARRAY_PTR), (CAPACITY), sizeof(TYPE), NULL)

/// @brief : View the dynamic array as a standard C array. Like dynamicArray to []
#define FORGE_ARRAY_DATA(ARRAY_PTR, TYPE) \
  ((TYPE*) (ARRAY_PTR)->data)

/**
 * @brief : Type-safe get element macro
 * @see : dynamicArrayAt
*/
#define FORGE_ARRAY_GET(ARRAY_PTR, TYPE, INDEX) \
  (FORGE_ARRAY_DATA(ARRAY_PTR, TYPE)[INDEX])

/// @brief : Direct zero-copy typed emplace
#define FORGE_ARRAY_EMPLACE(ARRAY_PTR, TYPE) \
  ((TYPE*) forgeDynamicArrayEmplace(ARRAY_PTR))

/// @brief : Fast-path push value
#define FORGE_ARRAY_PUSH_VAL(ARRAY_PTR, TYPE, VALUE) \
  do { \
    TYPE* _slot = FORGE_ARRAY_EMPLACE(ARRAY_PTR, TYPE); \
    if (_slot) *_slot = (VALUE); \
  } while(0)

/// @brief : Batch range append
#define FORGE_ARRAY_PUSH_RANGE(ARRAY_PTR, SRC_PTR, COUNT) \
  forgeDynamicArrayPushRange((ARRAY_PTR), (const void*)(SRC_PTR), (COUNT))


#ifdef __cplusplus
}
#endif
