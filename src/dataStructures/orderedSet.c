#include <forgeUtils/memory/linearAlloc.h>
#include <forgeUtils/dataStructures/orderedSet.h>
#include <forgeUtils/core/asserts.h>
#include <forgeUtils/core/logger.h>
#include <forgeUtils/memory/tracker.h>
#include <stdint.h>
#include <memory.h>
#include <stdlib.h>

static int32_t defaultMemcmp(const void* A, const void* B)
{ return memcmp(A, B, sizeof(uintptr_t)); }

static uint32_t nodeHeight(ForgeAVLNode* NODE)
{ return NODE ? NODE->height : 0; }

static int32_t maxInt(int32_t A, int32_t B)
{ return (A > B) ? A : B; }

static int32_t getBalance(ForgeAVLNode* NODE)
{ return NODE ? (nodeHeight(NODE->left) - nodeHeight(NODE->right)) : 0; }


// - - - Rotations - - - 

static inline ForgeAVLNode* rotateRight(ForgeAVLNode* Y)
{
  ForgeAVLNode* x  = Y->left;
  ForgeAVLNode* T2 = x->right;

  x->right  = Y;
  Y->left   = T2;

  Y->height = maxInt(nodeHeight(Y->left), nodeHeight(Y->right)) + 1;
  x->height = maxInt(nodeHeight(x->left), nodeHeight(x->right)) + 1;

  return x;
}

static inline ForgeAVLNode* rotateLeft(ForgeAVLNode* X)
{
  ForgeAVLNode* y   = X->right;
  ForgeAVLNode* T2  = y->left;

  y->left   = X;
  X->right  = T2;

  X->height = maxInt(nodeHeight(X->left), nodeHeight(X->right)) + 1;
  y->height = maxInt(nodeHeight(y->left), nodeHeight(y->right)) + 1;

  return y;
}

// - - - Core Tree Helpers - - - 

static ForgeAVLNode* createNode(ForgeAVLTree* TREE, const void* VALUE_PTR) 
{
  size_t totalBytes = sizeof(ForgeAVLNode) + TREE->elementSize;

  ForgeAVLNode* node = TREE->allocator
      ? (ForgeAVLNode*) forgeLinearAllocAllocate(TREE->allocator, totalBytes, 0)
      : (ForgeAVLNode*) FORGE_MALLOC(totalBytes);

  if (!node) 
  {
    FORGE_LOG_WARNING("[ORDERED SET] : Failed to allocate space for a new node");
    return NULL;
  }

  node->height  = 1;
  node->left    = NULL;
  node->right   = NULL;
  memcpy(node->data, VALUE_PTR, TREE->elementSize);

  return node;
}

static inline void freeNode(ForgeAVLTree* TREE, ForgeAVLNode* NODE) 
{
  if (!NODE) return;
  if (!TREE->allocator) FORGE_FREE(NODE);
}

static void destroySubtree(ForgeAVLTree* TREE, ForgeAVLNode* NODE) 
{
  if (!NODE) return;
  destroySubtree(TREE, NODE->left);
  destroySubtree(TREE, NODE->right);
  freeNode(TREE, NODE);
}


// - - - Insertion & Balancing - - - 

static ForgeAVLNode* insertRecursive(
  ForgeAVLTree* TREE,
  ForgeAVLNode* NODE,
  const void*   VALUE_PTR,
  bool*         OUT_INSERTED)
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
  if (balance > 1)
  {
    if (getBalance(NODE->left) < 0)
    {
      NODE->left = rotateLeft(NODE->left);
    }
    return rotateRight(NODE); 
  }

  // - - - Right Right Case
  if (balance < -1) 
  {
    if (getBalance(NODE->right) > 0)
    {
      NODE->right = rotateRight(NODE->right);
    }
    return rotateLeft(NODE);
  }
  return NODE;
}


// - - - Deletion & Balancing - - -

static ForgeAVLNode* minValueNode(ForgeAVLNode* NODE)
{
  ForgeAVLNode* current = NODE;
  while (current->left != NULL) current = current->left;
  return current;
}

static ForgeAVLNode* removeRecursive(
  ForgeAVLTree*   TREE,
  ForgeAVLNode*   ROOT,
  const void*     VALUE_PTR,
  bool*           OUT_REMOVED)
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
      ForgeAVLNode* temp = ROOT->left ? ROOT->left : ROOT->right;

      // - - - No child
      if (!temp) 
      {
        freeNode(TREE, ROOT);
        return NULL;
      }

      // - - - One child
      else
      {
        memcpy(ROOT->data, temp->data, TREE->elementSize);
        ROOT->left    = temp->left;
        ROOT->right   = temp->right;
        ROOT->height  = temp->height;
        freeNode(TREE, temp);
      }
    }

    // - - - Two children: Get in-order successor
    else
    {
      ForgeAVLNode* temp = minValueNode(ROOT->right);
      memcpy(ROOT->data, temp->data, TREE->elementSize);
      ROOT->right = removeRecursive(TREE, ROOT->right, temp->data, OUT_REMOVED);
    }
  }

  // - - - Update height & rebalance
  ROOT->height    = 1 + maxInt(nodeHeight(ROOT->left), nodeHeight(ROOT->right));
  int32_t balance = getBalance(ROOT);

  // - - - Left Left
  if (balance > 1)
  {
    if (getBalance(ROOT->left) < 0)
    {
      ROOT->left = rotateLeft(ROOT->left);
    }
    return rotateRight(ROOT); 
  }

  // - - - Left Right
  if (balance < -1)
  {
    if (getBalance(ROOT->right) > 0)
    {
      ROOT->right = rotateRight(ROOT->right);
    }
    return rotateLeft(ROOT);
  }
  
  return ROOT;
}


// - - - Public API

bool forgeOrderedSetCreate(ForgeAVLTree* TREE, size_t ELEMENT_SIZE, ForgeCompareFunc COMPARATOR, ForgeLinearAllocator* ALLOCATOR)
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Target pointer cannot be NULL");
  FORGE_ASSERT_DEBUG_MESSAGE(ELEMENT_SIZE > 0, "[ORDERED SET] : Element size must be greater than 0");

  TREE->root        = NULL;
  TREE->size        = 0;
  TREE->elementSize = ELEMENT_SIZE;
  TREE->compare     = COMPARATOR ? COMPARATOR : defaultMemcmp;
  TREE->allocator   = ALLOCATOR;

  return true;
}

void forgeOrderedSetDestroy(ForgeAVLTree* TREE) 
{
  if (!TREE) return;
  destroySubtree(TREE, TREE->root);
  TREE->root = NULL;
  TREE->size = 0;
}

bool forgeOrderedSetInsert(ForgeAVLTree* TREE, const void* VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Cannot insert into NULL tree");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[ORDERED SET] : Value pointer cannot be NULL");

  bool inserted = false;
  TREE->root    = insertRecursive(TREE, TREE->root, VALUE_PTR, &inserted);
  if (inserted) TREE->size++;

  return inserted;
}

bool forgeOrderedSetRemove(ForgeAVLTree* TREE, const void* VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] :  Cannot remove from NULL tree");
  if (!TREE->root) return false;

  bool removed  = false;
  TREE->root    = removeRecursive(TREE, TREE->root, VALUE_PTR, &removed);
  if (removed) TREE->size--;

  return removed;
}

void* forgeOrderedSetFind(const ForgeAVLTree* TREE, const void* VALUE_PTR) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Cannot search NULL tree");
  FORGE_ASSERT_DEBUG_MESSAGE(VALUE_PTR != NULL, "[ORDERED SET] : Cannot find a NULL VALUE_PTR");

  ForgeAVLNode* curr = TREE->root;
  while (curr) 
  {
    int32_t cmp = TREE->compare(VALUE_PTR, curr->data);

    if (cmp == 0)     return (void*)curr->data;
    else if (cmp < 0) curr = curr->left;
    else              curr = curr->right;
  }

  return NULL;
}

static void inorderRecursive(ForgeAVLNode* NODE, ForgeVisitorFunc VISITOR, void* USER_DATA)
{
  if (!NODE) return;
  inorderRecursive(NODE->left, VISITOR, USER_DATA);
  VISITOR((const void*)NODE->data, USER_DATA);
  inorderRecursive(NODE->right, VISITOR, USER_DATA);
}

void forgeOrderedSetTraverseInorder(
  const ForgeAVLTree* TREE,
  ForgeVisitorFunc    VISITOR,
  void*               USER_DATA)
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Cannot traverse NULL tree");
  FORGE_ASSERT_DEBUG_MESSAGE(VISITOR != NULL, "[ORDERED SET] : Visitor callback cannot be NULL");

  inorderRecursive(TREE->root, VISITOR, USER_DATA);
}

void forgeOrderedSetClear(ForgeAVLTree* TREE) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(TREE != NULL, "[ORDERED SET] : Cannot clear a NULL TREE");

  destroySubtree(TREE, TREE->root);
  TREE->root = NULL;
  TREE->size = 0;
}
