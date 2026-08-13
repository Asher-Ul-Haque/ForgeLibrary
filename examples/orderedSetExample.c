/**
 * @file orderedSetExample.c
 * @brief Simple guide showing how to use ForgeAVLTree (Ordered Set) for sorted, unique elements.
 */

#include <core/logger.h>
#include <dataStructures/orderedSet.h>

#include <stdbool.h>
#include <stddef.h>

// Custom comparator for integers
static int32_t compare_ints(const void* a, const void* b) {
    int val_a = *(const int*)a;
    int val_b = *(const int*)b;
    return (val_a > val_b) - (val_a < val_b);
}

// Visitor callback for in-order traversal (prints elements in sorted order)
static void print_visitor(const void* value, void* user_data) {
    int val = *(const int*)value;
    int* count = (int*)user_data;
    FORGE_LOG_INFO("  [%d] Item value: %d", (*count)++, val);
}

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("        FORGE ORDERED SET EXAMPLE           ");
    FORGE_LOG_INFO("============================================");

    // 1. Create Ordered Set (AVL Tree) for integers
    ForgeAVLTree set = {0};
    if (!forgeOrderedSetCreate(&set, sizeof(int), compare_ints, NULL)) {
        FORGE_LOG_ERROR("Failed to create ordered set!");
        return 1;
    }

    FORGE_LOG_INFO("Created Ordered Set (Element Size: %zu B).", set.elementSize);

    // 2. Insert unsorted values
    int values[] = {50, 20, 70, 10, 30, 60, 80};
    size_t num_values = sizeof(values) / sizeof(values[0]);

    FORGE_LOG_INFO("Inserting unsorted elements: 50, 20, 70, 10, 30, 60, 80...");
    for (size_t i = 0; i < num_values; ++i) {
        forgeOrderedSetInsert(&set, &values[i]);
    }

    FORGE_LOG_INFO("Set size: %zu", set.size);

    // 3. Attempt duplicate insertion (Ordered Set enforces uniqueness)
    int duplicate = 30;
    bool inserted_dup = forgeOrderedSetInsert(&set, &duplicate);
    FORGE_LOG_INFO("Attempted duplicate insert of 30: %s (Set size remains: %zu)", 
                   inserted_dup ? "SUCCESS" : "REJECTED (as expected)", set.size);

    // 4. In-order traversal (prints elements strictly sorted)
    FORGE_LOG_INFO("--- Sorted In-Order Traversal ---");
    int counter = 0;
    forgeOrderedSetTraverseInorder(&set, print_visitor, &counter);

    // 5. Lookup elements
    int search_val = 60;
    int missing_val = 99;
    FORGE_LOG_INFO("Contains 60? %s", forgeOrderedSetContains(&set, &search_val) ? "YES" : "NO");
    FORGE_LOG_INFO("Contains 99? %s", forgeOrderedSetContains(&set, &missing_val) ? "YES" : "NO");

    // 6. Remove node (triggers internal AVL tree rotation/rebalance)
    int remove_val = 20;
    FORGE_LOG_INFO("Removing element 20...");
    if (forgeOrderedSetRemove(&set, &remove_val)) {
        FORGE_LOG_INFO("Removed 20. New set size: %zu", set.size);
    }

    // Verify sorted order after removal
    FORGE_LOG_INFO("--- Traversal After Removal ---");
    counter = 0;
    forgeOrderedSetTraverseInorder(&set, print_visitor, &counter);

    // 7. Clear set
    forgeOrderedSetClear(&set);
    FORGE_LOG_INFO("Cleared set. Size: %zu", set.size);

    // 8. Clean up backing memory
    forgeOrderedSetDestroy(&set);
    FORGE_LOG_INFO("Ordered set destroyed cleanly.");

    return 0;
}
