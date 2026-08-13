/**
 * @file stackExample.c
 * @brief Simple guide showing how to use ForgeStack (LIFO data structure).
 */

#include <core/logger.h>
#include <dataStructures/stack.h>

#include <stdbool.h>
#include <stddef.h>

typedef struct StateFrame {
    int state_id;
    float delta_time;
} StateFrame;

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("           FORGE STACK EXAMPLE              ");
    FORGE_LOG_INFO("============================================");

    // 1. Create Stack instance
    ForgeStack stack = {0};
    if (!forgeStackCreate(&stack, 4, sizeof(StateFrame), NULL)) {
        FORGE_LOG_ERROR("Failed to create stack!");
        return 1;
    }

    FORGE_LOG_INFO("Created Stack (Element Size: %zu B).", sizeof(StateFrame));

    // 2. Push elements onto the stack (LIFO)
    StateFrame f1 = {.state_id = 1, .delta_time = 0.016f};
    StateFrame f2 = {.state_id = 2, .delta_time = 0.033f};
    StateFrame f3 = {.state_id = 3, .delta_time = 0.008f};

    forgeStackPush(&stack, &f1);
    forgeStackPush(&stack, &f2);
    forgeStackPush(&stack, &f3);

    FORGE_LOG_INFO("Pushed 3 frames onto stack. Current size: %zu", forgeStackSize(&stack));

    // 3. Peek top element without popping
    StateFrame* top_frame = (StateFrame*)forgeStackPeek(&stack);
    if (top_frame) {
        FORGE_LOG_INFO("Peek top element -> State ID: %d (dt: %.3fs)", 
                       top_frame->state_id, top_frame->delta_time);
    }

    // 4. Pop elements in Last-In, First-Out order
    FORGE_LOG_INFO("--- Popping Elements (LIFO Order) ---");
    StateFrame popped = {0};
    while (!forgeStackIsEmpty(&stack)) {
        if (forgeStackPop(&stack, &popped)) {
            FORGE_LOG_INFO("Popped -> State ID: %d (Remaining size: %zu)", 
                           popped.state_id, forgeStackSize(&stack));
        }
    }

    FORGE_LOG_INFO("Is stack empty? %s", forgeStackIsEmpty(&stack) ? "YES" : "NO");

    // 5. Clean up backing array
    forgeStackDestroy(&stack);
    FORGE_LOG_INFO("Stack destroyed cleanly.");

    return 0;
}
