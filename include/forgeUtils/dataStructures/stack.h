/**
 * @file : stack.h 
 * @brief : Stack implementation using dynamicArray 
 * @see : dynamicArray.h
 */

#pragma once 
#include <forgeUtils/dataStructures/dynamicArray.h>
#include <forgeUtils/core/asserts.h>
#include <forgeUtils/core/logger.h>

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
  FORGE_ASSERT_DEBUG_MESSAGE(STACK != NULL, "[STACK] : Cannot create a NULL STACK");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[STACK] : Element size must be greater than 0");
  return forgeDynamicArrayCreate(&STACK->array, INITIAL_CAPACITY, ELEMENT_SIZE, ALLOCATOR);

}

/**
 * @brief : Destroys the stack and frees internal resources.
 * @param STACK : The stack to destroy 
 */
static inline void forgeStackDestroy(ForgeStack* STACK) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(STACK != NULL, "[STACK] : Cannot destroy a NULL STACK");
  forgeDynamicArrayDestroy(&STACK->array); 
}

/**
 * @brief : Pushes an element onto the top of the stack (O(1)).
 * @param STACK : The stack to push into 
 * @param VALUE_PTR : The value to push
 * @return : True if successful, false if not
*/
static inline bool forgeStackPush(ForgeStack* STACK, const void* VALUE_PTR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(STACK != NULL, "[STACK] : Cannot push to a NULL stack");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[STACK] : Cannot push a NULL VALUE_PTR to a stack");

  return forgeDynamicArrayPush(&STACK->array, VALUE_PTR);
}

/**
 * @brief : Reserves a slot at the top and returns a direct pointer to uninitialized element memory.
 * @param STACK : The stack to be emplaced
 * @return : Pointer to uninitialized data
 * @warning: The element is uninitialized, use the pointer to Initialize
 */
static inline void* forgeStackEmplace(ForgeStack* STACK)
{
  FORGE_ASSERT_DEBUG_MESSAGE(STACK != NULL, "[STACK] : Cannot emplace in a NULL STACK");

  return forgeDynamicArrayEmplace(&STACK->array);
}

/**
 * @brief : Pops the top element from the stack (O(1)).
 * @param STACK : The stack to pop from 
 * @param OUT_VALUE_PTR : Optional pointer to store the value in
 * @return : True if successful, False if not
 */
static inline bool forgeStackPop(ForgeStack* STACK, void* OUT_VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(STACK != NULL, "[STACK] : Cannot pop from a NULL ARRAY");

  if (STACK->array.size == 0)
  {
    FORGE_LOG_WARNING("[STACK] : Cannot pop, stack is empty");
    return false;
  }

  return forgeDynamicArrayPop(&STACK->array, OUT_VALUE_PTR);
}

/**
 * @brief : Views the top element without removing it.
 * @param STACK : The stack to peek from 
 * @return : Pointer to the top of the stack
 */
static inline void* forgeStackPeek(const ForgeStack* STACK) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(STACK != NULL, "[STACK] : Cannot peek into a NULL STACK");

  if (STACK->array.size == 0) return NULL;
  return forgeDynamicArrayAt(&STACK->array, STACK->array.size - 1);
}

/**
 * @brief : Returns current element count.
 * @param STACK : The stack whose size is to be known
 * @return : how many elements in the stack
 */
static inline size_t forgeStackSize(const ForgeStack* STACK) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(STACK != NULL, "[STACK] : Cannot check size of a NULL STACK");
  return STACK->array.size; 
}

/**
 * @brief : Checks if stack is empty.
 * @param STACK : The stack to check 
 * @return : True if stack empty, False if not
 */
static inline bool forgeStackIsEmpty(const ForgeStack* STACK) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(STACK != NULL, "[STACK] : Cannot check if a NULL STACK is empty");
  return STACK->array.size == 0; 
}

// - - - Ergonomic & Type-Safe Macros - - -

#define FORGE_STACK_INIT(STACK_PTR, CAPACITY, TYPE, ALLOCATOR_PTR) \
  forgeStackCreate((STACK_PTR), (CAPACITY), sizeof(TYPE), (ALLOCATOR_PTR))

/// @brief Direct typed top element inspection: *FORGE_STACK_TOP(s, MyType)
#define FORGE_STACK_TOP(STACK_PTR, TYPE) \
  (&((TYPE*)(STACK_PTR)->array.data)[(STACK_PTR)->array.size - 1])

/// @brief Zero-copy emplace onto stack
#define FORGE_STACK_EMPLACE(STACK_PTR, TYPE) \
  ((TYPE*) forgeStackEmplace(STACK_PTR))

/// @brief Type-safe push by value
#define FORGE_STACK_PUSH_VAL(STACK_PTR, TYPE, VALUE) \
  FORGE_ARRAY_PUSH_VAL(&(STACK_PTR)->array, TYPE, (VALUE))

#ifdef __cplusplus
}
#endif
