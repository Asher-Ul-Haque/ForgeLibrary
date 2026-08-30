/**
 * @file dynamicArrayExample.c
 * @brief Simple guide showing how to use ForgeDynamicArray and its ergonomic macros.
 */

#include <forgeUtils/core/logger.h>
#include <forgeUtils/dataStructures/dynamicArray.h>

#include <stdbool.h>
#include <stddef.h>

typedef struct Item {
    int id;
    float value;
} Item;

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("      FORGE DYNAMIC ARRAY EXAMPLE           ");
    FORGE_LOG_INFO("============================================");

    // 1. Initialize Dynamic Array using the ergonomic macro
    ForgeDynamicArray arr = {0};
    if (!FORGE_ARRAY_INIT(&arr, 4, Item, NULL)) {
        FORGE_LOG_ERROR("Failed to initialize dynamic array!");
        return 1;
    }

    FORGE_LOG_INFO("Initialized array with capacity %zu (item size: %zu bytes).", 
                   arr.capacity, arr.elementSize);

    // 2. Push elements using FORGE_ARRAY_PUSH_VAL macro
    FORGE_ARRAY_PUSH_VAL(&arr, Item, ((Item){.id = 101, .value = 12.5f}));
    FORGE_ARRAY_PUSH_VAL(&arr, Item, ((Item){.id = 102, .value = 45.0f}));
    FORGE_ARRAY_PUSH_VAL(&arr, Item, ((Item){.id = 103, .value = 99.9f}));

    // 3. Push element using raw C function (passing pointer)
    Item item4 = {.id = 104, .value = 250.0f};
    forgeDynamicArrayPush(&arr, &item4);

    FORGE_LOG_INFO("Pushed 4 elements. Size: %zu, Capacity: %zu", arr.size, arr.capacity);

    // 4. Access elements using FORGE_ARRAY_GET macro
    FORGE_LOG_INFO("--- Array Contents ---");
    for (size_t i = 0; i < arr.size; ++i) {
        Item current = FORGE_ARRAY_GET(&arr, Item, i);
        FORGE_LOG_INFO("  [%zu] ID: %d, Value: %.1f", i, current.id, current.value);
    }

    // 5. Modify elements directly in-place using forgeDynamicArrayAt
    Item* item_ref = (Item*)forgeDynamicArrayAt(&arr, 1);
    if (item_ref) {
        item_ref->value = 50.0f; // Update value at index 1
        FORGE_LOG_INFO("Updated index 1 value in-place to %.1f", item_ref->value);
    }

    // 6. Pop the last element
    Item popped_item = {0};
    if (forgeDynamicArrayPop(&arr, &popped_item)) {
        FORGE_LOG_INFO("Popped last element -> ID: %d, Value: %.1f", popped_item.id, popped_item.value);
        FORGE_LOG_INFO("Array size after pop: %zu", arr.size);
    }

    // 7. Test capacity expansion / realloc
    FORGE_LOG_INFO("Pushing 5 additional items to trigger auto-reserve...");
    for (int i = 0; i < 5; ++i) {
        FORGE_ARRAY_PUSH_VAL(&arr, Item, ((Item){.id = 200 + i, .value = (float)(i * 10)}));
    }
    FORGE_LOG_INFO("Array size: %zu, New Capacity: %zu", arr.size, arr.capacity);

    // 8. Clear array elements without freeing capacity
    forgeDynamicArrayClear(&arr);
    FORGE_LOG_INFO("Cleared array. Size: %zu, Capacity retained: %zu", arr.size, arr.capacity);

    // 9. Destroy array and release memory
    forgeDynamicArrayDestroy(&arr);
    FORGE_LOG_INFO("Dynamic array destroyed cleanly.");

    return 0;
}
