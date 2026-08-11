/**
 * @file : orderedSet.h 
 * @brief : Ordered set implementation in C using an AVL tree
 */

#pragma once 

#include <memory/linearAlloc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifndef DEFAULT_ALIGNMENT_BYTES  
  #define DEFAULT_ALIGNMENT_BYTES 16 
#endif

#ifdef __cplusplus
extern "C" {
#endif

/// @brief : Comparison callback signature: returns <0 if a < b, 0 if a == b, >0 if a > b
typedef int32_t (*ForgeCompareFunc)(const void* A, const void* B);

/// @brief : In-order traversal visitor callback
typedef void (*ForgeVisitorFunc)(const void* VALUE, void* USER_DATA);

/// @brief : AVL Tree atom
typedef struct AVLNode 
{
  uint8_t*         data;     ///< Contiguous payload memory
  int32_t          height;   ///< Height of subtree
  struct AVLNode*  left;     ///< Left child pointer
  struct AVLNode*  right;    ///< Right child pointer
} AVLNode;

/// @brief : AVL Tree data structure
typedef struct AVLTree 
{
  AVLNode*         root;          ///< Pointer to root node
  size_t           size;          ///< Total element count
  size_t           elementSize;   ///< Size of an element in bytes
  ForgeCompareFunc compare;       ///< Comparison function (defaults to memcmp if NULL)
  ForgeLinearAllocator* allocator;     ///< Optional custom linear allocator
} AVLTree;

/**
 * @brief : Creates an AVL Tree (Ordered Set).
 * 
 * @param TREE : Pointer to AVLTree struct.
 * @param ELEMENT_SIZE : Size of each element in bytes (sizeof(T)).
 * @param COMPARATOR : Comparison function callback (if NULL, defaults to memcmp).
 * @param ALLOCATOR : Pointer to linear allocator, or NULL to use global memory tracker.
 * @return : true if created successfully, false otherwise.
 */
bool orderedSetCreate(
  AVLTree*          TREE, 
  size_t            ELEMENT_SIZE, 
  ForgeCompareFunc  COMPARATOR, 
  ForgeLinearAllocator*  ALLOCATOR);

/**
 * @brief : Destroys the tree and frees all nodes.
 * @param TREE : Pointer to the AVL tree 
 */
void orderedSetDestroy(AVLTree* TREE);

/**
 * @brief : Inserts a unique element into the tree (Ordered Set behavior).
 * @param TREE : Pointer to the AVL tree 
 * @param VALUE_PTR : Pointer to the value to be inserted
 * @return true if inserted, false if element already exists or allocation fails.
 */
bool orderedSetInsert(AVLTree* TREE, const void* VALUE_PTR);

/**
 * @brief : Removes an element from the tree.
 * @param TREE : Pointer to the AVL Tree 
 * @param VALUE_PTR : Pointer to the value to be removed
 * @return : true if found and removed, false if not present.
 */
bool orderedSetRemove(AVLTree* TREE, const void* VALUE_PTR);

/**
 * @brief : Searches for an element in the tree.
 * @brief TREE : Pointer to the AVL tree
 * @param VALUE_PTR : Pointer to the element to be searched
 * @return : Pointer to element data if found, NULL if not found.
 */
void* orderedSetFind(const AVLTree* TREE, const void* VALUE_PTR);

/**
 * @brief : Checks if the tree contains an element.
 * @param TREE : Pointer to the AVL Tree 
 * @param VALUE_PTR : Pointer to the value to be checked 
 * @return : true if the set contains it, false if not
 */
bool orderedSetContains(const AVLTree* TREE, const void* VALUE_PTR);

/**
 * @brief : Performs an in-order traversal (sorted order) calling visitor for each element.
 * @param TREE : Pointer to the AVL tree 
 * @param VISITOR_FUNC : Visitor function pointer for inorder traversal
 */
void orderedSetTraverseInorder(const AVLTree* TREE, ForgeVisitorFunc VISITOR, void* USER_DATA);

/**
 * @brief Clears all elements from the tree.
 * @param TREE : Pointer to the AVL tree
 */
void orderedSetClear(AVLTree* TREE);

#ifdef __cplusplus
}
#endif
