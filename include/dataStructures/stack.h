/**
 * @file : stack.h 
 * @brief : Stack implementation using dynamicArray 
 * @see : dynamicArray.h
 */

#pragma once 
#include <dataStructures/dynamicArray.h>

#ifdef __cplusplus
  extern "C" {
#endif

/// @brief : Stack just has an underlying dynamic Array 
typedef struct forgeStack 
{
  ForgeDynamicArray array; ///< The underlying dynamic Array
} ForgeStack;

/**
 * @brief : Initializes a stack instance.
 * @param STACK : The stack to create 
 * @param INITIAL_CAPACITY : The initial capacity of the stack 
 * @param ELEMENT_SIZE : The size of one element in the stack 
 * @param ALLOCATOR : Optional linear allocator
 * @return : True if successful, false if not
*/
static inline bool forgeStackCreate(
  ForgeStack*           STACK,
  size_t                INITIAL_CAPACITY,
  size_t                ELEMENT_SIZE,
  ForgeLinearAllocator* ALLOCATOR)
{
  return forgeDynamicArrayCreate(&STACK->array, INITIAL_CAPACITY, ELEMENT_SIZE, ALLOCATOR);
}

/**
 * @brief : Destroys the stack and frees internal resources.
 * @param STACK : The stack to destroy 
 */
static inline void forgeStackDestroy(ForgeStack* STACK) 
{ forgeDynamicArrayDestroy(&STACK->array); }

/**
 * @brief : Pushes an element onto the top of the stack (O(1)).
 * @param STACK : The stack to push into 
 * @param VALUE_PTR : The value to push
 * @return : True if successful, false if not
*/
static inline bool forgeStackPush(ForgeStack* STACK, const void* VALUE_PTR) 
{
  return forgeDynamicArrayPush(&STACK->array, VALUE_PTR);
}

/**
 * @brief : Pops the top element from the stack (O(1)).
 * @param STACK : The stack to pop from 
 * @param OUT_VALUE_PTR : Optional pointer to store the value in
 * @return : True if successful, False if not
 */
static inline bool forgeStackPop(ForgeStack* STACK, void* OUT_VALUE_PTR) 
{
  return forgeDynamicArrayPop(&STACK->array, OUT_VALUE_PTR);
}

/**
 * @brief : Views the top element without removing it.
 * @param STACK : The stack to peek from 
 * @return : Pointer to the top of the stack
 */
static inline void* forgeStackPeek(const ForgeStack* STACK) 
{
  if (STACK->array.size == 0) return NULL;
  return forgeDynamicArrayAt(&STACK->array, STACK->array.size - 1);
}

/**
 * @brief : Returns current element count.
 * @param STACK : The stack whose size is to be known
 * @return : how many elements in the stack
 */
static inline size_t forgeStackSize(const ForgeStack* STACK) 
{ return STACK->array.size; }

/**
 * @brief : Checks if stack is empty.
 * @param STACK : The stack to check 
 * @return : True if stack empty, False if not
 */
static inline bool forgeStackIsEmpty(const ForgeStack* STACK) 
{ return STACK->array.size == 0; }

#ifdef __cplusplus
}
#endif
