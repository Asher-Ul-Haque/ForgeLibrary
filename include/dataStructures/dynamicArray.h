/**
 * @file : dynamicArray.h 
 * @brief : Dynamic Array implementation in C
*/

#pragma once 

#include <memory/linearAlloc.h>
#include <stdint.h>
#ifdef __cplusplus 
extern "C" {
#endif 

#ifndef DEFAULT_ALIGNMENT_BYTES  
  #define DEFAULT_ALIGNMENT_BYTES 16 
#endif

#define ARRAY_DEFAULT_CAPACITY 8 

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
 * @brief : Pushes a new element value to the back of the array.
 * @param ARRAY : The dynamic array to which the value is to be pushed 
 * @param VALUE_PTR : Pointer to the value being stored
 * @warning : VALUE_PTR's value will be copied
 * @return : True if push was succesful, false if not 
*/
bool forgeDynamicArrayPush(ForgeDynamicArray* ARRAY, const void* VALUE_PTR);

/**
 * @brief : Pops the last element from the array 
 * @param OUT_VALUE_PTR : Optional pointer to receive the popped element bytes
 * @warning : the size of OUT_VALUE_PTR should be big enough to store the element
 * @return : whether the pop was successfull
*/
bool forgeDynamicArrayPop(ForgeDynamicArray* ARRAY, void* OUT_VALUE_PTR);

/**
 * @brief : Returns a pointer to the element at the given index 
 * @param ARRAY : Pointer to the array which is to be accessed
 * @param INDEX : The index to access at 
 * @warning : The index must be in bounds
 * @warning : Since this returns a void*, you can override it directly, but be careful, since you get access to the memory underneath
 * @return : A pointer to the object in the array at the given index
*/
void* forgeDynamicArrayAt(const ForgeDynamicArray* ARRAY, size_t INDEX);

/**
 * @brief : Clears all elements without freeing memory.
 * @param ARRAY : A pointer to the array to be cleared
*/
void forgeDynamicArrayClear(ForgeDynamicArray* ARRAY);


// - - - Helper Macros for Ergonomic Usage - - - 

/**
 * @brief : Helper macro to initialize array with implicit type sizing 
 * @see dynamicArrayCreate
*/
#define FORGE_ARRAY_INIT(ARRAY_PTR, CAPACITY, TYPE, ALLOCATOR_PTR) \
  forgeDynamicArrayCreate((ARRAY_PTR), (CAPACITY), sizeof(TYPE), (ALLOCATOR_PTR))

/**
 * @brief : Type-safe push macro taking value directly by value/expression
 * @see : dynamicArrayPush
*/
#define FORGE_ARRAY_PUSH_VAL(ARRAY_PTR, TYPE, VALUE)  \
  do                                                  \
  {                                                   \
    TYPE _temp_val = (VALUE);                         \
    forgeDynamicArrayPush((ARRAY_PTR), &_temp_val);   \
  } while(0) 

/**
 * @brief : Type-safe get element macro
 * @see : dynamicArrayAt
*/
#define FORGE_ARRAY_GET(ARRAY_PTR, TYPE, INDEX) \
  (*(TYPE*) forgeDynamicArrayAt((ARRAY_PTR), (INDEX)))
