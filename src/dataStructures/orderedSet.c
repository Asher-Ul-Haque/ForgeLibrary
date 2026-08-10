#include "memory/linearAlloc.h"
#include <dataStructures/orderedSet.h>
#include <core/asserts.h>
#include <core/logger.h>
#include <stdint.h>
#include <memory.h>
#include <stdlib.h>

static inline uintptr_t alignUpPtr(uintptr_t PTR, uintptr_t ALIGNMENT)
{
  FORGE_ASSERT_DEBUG_MESSAGE(ALIGNMENT % 2 == 0, "[LINEAR ALLOC] : ALIGNMENT must be a multiple of 2");
  if (ALIGNMENT == 0) ALIGNMENT = DEFAULT_ALIGNMENT_BYTES;
  return (PTR + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

static int32_t defaultMemcmp(const void* A, const void* B)
{ return memcmp(A, B, sizeof(uintptr_t)); }

static uint32_t nodeHeight(AVLNode* NODE)
{ return NODE ? NODE->height : 0; }

static int32_t maxInt(int32_t A, int32_t B)
{ return (A > B) ? A : B; }

static int32_t getBalance(AVLNode* NODE)
{ return NODE ? (nodeHeight(NODE->left) - nodeHeight(NODE->right)) : 0; }


// - - - Rotations - - - 

static AVLNode* rotateRight(AVLNode* Y)
{
  AVLNode* x  = Y->left;
  AVLNode* T2 = x->right;

  x->right = Y;
  Y->left = T2;

  Y->height = maxInt(nodeHeight(Y->left), nodeHeight(Y->right)) + 1;
  x->height = maxInt(nodeHeight(x->left), nodeHeight(x->right)) + 1;

  return x;
}

static AVLNode* rotateLeft(AVLNode* X)
{
  AVLNode* y = X->right;
  AVLNode* T2 = y->left;

  y->left = X;
  X->right = T2;

  X->height = maxInt(nodeHeight(X->left), nodeHeight(X->right)) + 1;
  y->height = maxInt(nodeHeight(y->left), nodeHeight(y->right)) + 1;

  return y;
}

// - - - Core Tree Helpers - - - 

static AVLNode* createNode(AVLTree* TREE, const void* VALUE_PTR) 
{
  size_t nodeStructSize = alignUpPtr(sizeof(AVLNode), DEFAULT_ALIGNMENT_BYTES);
  size_t totalBytes = nodeStructSize + TREE->elementSize;

  AVLNode* node = NULL;
  if (TREE->allocator) 
  {
    node = (AVLNode*)linearAllocAllocate(TREE->allocator, totalBytes, DEFAULT_ALIGNMENT_BYTES);
  } 
  else 
  {
    node = (AVLNode*)malloc(totalBytes);
  }

  if (!node) return NULL;

  node->height  = 1;
  node->left    = NULL;
  node->right   = NULL;
  node->data    = ((uint8_t*)node) + nodeStructSize;
  memcpy(node->data, VALUE_PTR, TREE->elementSize);

  return node;
}

static void freeNode(AVLTree* TREE, AVLNode* NODE) 
{
  if (!NODE) return;
  if (!TREE->allocator) free(NODE);
}

static void destroySubtree(AVLTree* TREE, AVLNode* NODE) 
{
  if (!NODE) return;
  destroySubtree(TREE, NODE->left);
  destroySubtree(TREE, NODE->right);
  freeNode(TREE, NODE);
}


// - - - Insertion & Balancing - - - 

static AVLNode* insertRecursive(
  AVLTree*    TREE,
  AVLNode*    NODE,
  const void* VALUE_PTR,
  bool*       OUT_INSERTED)
{
  if (!NODE) 
  {
    *OUT_INSERTED = true;
    return createNode(TREE, VALUE_PTR);
  }

  int32_t cmp = TREE->compare(VALUE_PTR, NODE->data);

  if (cmp < 0) 
  {
    NODE->left = insertRecursive(TREE, NODE->left, VALUE_PTR, OUT_INSERTED);
  } 
  else if (cmp > 0) 
  {
    NODE->right = insertRecursive(TREE, NODE->right, VALUE_PTR, OUT_INSERTED);
  } 
  
  // - - - Value already exists (Set property - no duplicates)
  else 
  {
    *OUT_INSERTED = false;
    return NODE;
  }

  // - - - Update height
  NODE->height = 1 + maxInt(nodeHeight(NODE->left), nodeHeight(NODE->right));

  // - - - Get balance factor
  int32_t balance = getBalance(NODE);

  // - - - Left Left Case
  if (balance > 1 && TREE->compare(VALUE_PTR, NODE->left->data) < 0) 
  { return rotateRight(NODE); }

  // - - - Right Right Case
  if (balance < -1 && TREE->compare(VALUE_PTR, NODE->right->data) > 0) 
  { return rotateLeft(NODE); }

  // - - - Left Right Case
  if (balance > 1 && TREE->compare(VALUE_PTR, NODE->left->data) > 0) 
  {
    NODE->left = rotateLeft(NODE->left);
    return rotateRight(NODE);
  }

  // - - - Right Left Case
  if (balance < -1 && TREE->compare(VALUE_PTR, NODE->right->data) < 0) 
  {
    NODE->right = rotateRight(NODE->right);
    return rotateLeft(NODE);
  }

  return NODE;
}


// - - - Deletion & Balancing - - -

static AVLNode* minValueNode(AVLNode* NODE)
{
  AVLNode* current = NODE;
  while (current->left != NULL) current = current->left;
  return current;
}

static AVLNode* removeRecursive(
  AVLTree*    TREE,
  AVLNode*    ROOT,
  const void* VALUE_PTR,
  bool*       OUT_REMOVED)
{
  if (!ROOT) 
  {
    *OUT_REMOVED = false;
    return NULL;
  }

  int32_t cmp = TREE->compare(VALUE_PTR, ROOT->data);

  if (cmp < 0) 
  {
    ROOT->left = removeRecursive(TREE, ROOT->left, VALUE_PTR, OUT_REMOVED);
  } 
  else if (cmp > 0) 
  {
    ROOT->right = removeRecursive(TREE, ROOT->right, VALUE_PTR, OUT_REMOVED);
  } 
  else 
  {
    // - - - Node found
    *OUT_REMOVED = true;

    if (!ROOT->left || !ROOT->right) 
    {
      AVLNode* temp = ROOT->left ? ROOT->left : ROOT->right;

      // - - - No child
      if (!temp) 
      {
        temp = ROOT;
        ROOT = NULL;
      }

      // - - - One child
      else *ROOT = *temp;

      freeNode(TREE, temp);
    }

    // - - - Two children: Get in-order successor
    else
    {
      AVLNode* temp = minValueNode(ROOT->right);
      memcpy(ROOT->data, temp->data, TREE->elementSize);
      ROOT->right = removeRecursive(TREE, ROOT->right, temp->data, OUT_REMOVED);
    }
  }

  if (!ROOT) return NULL;

  // - - - Update height & rebalance
  ROOT->height    = 1 + maxInt(nodeHeight(ROOT->left), nodeHeight(ROOT->right));
  int32_t balance = getBalance(ROOT);

  // - - - Left Left
  if (balance > 1 && getBalance(ROOT->left) >= 0) 
  { return rotateRight(ROOT); }

  // - - - Left Right
  if (balance > 1 && getBalance(ROOT->left) < 0) 
  {
    ROOT->left = rotateLeft(ROOT->left);
    return rotateRight(ROOT);
  }

  // - - - Right Right
  if (balance < -1 && getBalance(ROOT->right) <= 0) 
  { return rotateLeft(ROOT); }

  // - - - Right Left
  if (balance < -1 && getBalance(ROOT->right) > 0) 
  {
    ROOT->right = rotateRight(ROOT->right);
    return rotateLeft(ROOT);
  }

  return ROOT;
}


// - - - Public API

bool orderedSetCreate(AVLTree* TREE, size_t ELEMENT_SIZE, ForgeCompareFunc COMPARATOR, LinearAllocator* ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Target pointer cannot be NULL");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[ORDERED SET] : Element size must be greater than 0");

  TREE->root        = NULL;
  TREE->size        = 0;
  TREE->elementSize = alignUpPtr(ELEMENT_SIZE, sizeof(uintptr_t));
  TREE->compare     = COMPARATOR ? COMPARATOR : defaultMemcmp;
  TREE->allocator   = ALLOCATOR;

  return true;
}

void orderedSetDestroy(AVLTree* TREE) 
{
  if (!TREE) return;
  destroySubtree(TREE, TREE->root);
  TREE->root = NULL;
  TREE->size = 0;
}

bool orderedSetInsert(AVLTree* TREE, const void* VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Cannot insert into NULL tree");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[ORDERED SET] : Value pointer cannot be NULL");

  bool inserted = false;
  TREE->root    = insertRecursive(TREE, TREE->root, VALUE_PTR, &inserted);
  if (inserted) TREE->size++;

  return inserted;
}

bool orderedSetRemove(AVLTree* TREE, const void* VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] :  Cannot remove from NULL tree");
  if (!TREE->root) return false;

  bool removed  = false;
  TREE->root    = removeRecursive(TREE, TREE->root, VALUE_PTR, &removed);
  if (removed) TREE->size--;

  return removed;
}

void* orderedSetFind(const AVLTree* TREE, const void* VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Cannot search NULL tree");

  AVLNode* curr = TREE->root;
  while (curr) 
  {
    int32_t cmp = TREE->compare(VALUE_PTR, curr->data);

    if (cmp == 0)     return (void*)curr->data;
    else if (cmp < 0) curr = curr->left;
    else              curr = curr->right;
  }

  return NULL;
}

bool orderedSetContains(const AVLTree* TREE, const void* VALUE_PTR) 
{
  return (orderedSetFind(TREE, VALUE_PTR) != NULL);
}

static void inorderRecursive(AVLNode* NODE, ForgeVisitorFunc VISITOR, void* USER_DATA)
{
  if (!NODE) return;
  inorderRecursive(NODE->left, VISITOR, USER_DATA);
  VISITOR((const void*)NODE->data, USER_DATA);
  inorderRecursive(NODE->right, VISITOR, USER_DATA);
}

void orderedSetTraverseInorder(
  const AVLTree*    TREE,
  ForgeVisitorFunc  VISITOR,
  void*             USER_DATA)
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Cannot traverse NULL tree");
  FORGE_ASSERT_DEBUG_MESSAGE(VISITOR != NULL, "[ORDERED SET] : Visitor callback cannot be NULL");

  inorderRecursive(TREE->root, VISITOR, USER_DATA);
}

void orderedSetClear(AVLTree* TREE) 
{
  if (TREE) 
  {
    destroySubtree(TREE, TREE->root);
    TREE->root = NULL;
    TREE->size = 0;
  }
}
