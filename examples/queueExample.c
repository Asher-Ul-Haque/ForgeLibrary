/**
 * @file queueExample.c
 * @brief Simple guide showing how to use ForgeQueue (FIFO data structure).
 */

#include <core/logger.h>
#include <dataStructures/queue.h>

#include <stdbool.h>
#include <stddef.h>

typedef struct Command {
    int command_id;
    float priority;
} Command;

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("           FORGE QUEUE EXAMPLE              ");
    FORGE_LOG_INFO("============================================");

    // 1. Create a Queue for Command structs
    ForgeQueue queue = {0};
    if (!forgeQueueCreate(&queue, 4, sizeof(Command), NULL)) {
        FORGE_LOG_ERROR("Failed to create queue!");
        return 1;
    }

    FORGE_LOG_INFO("Created Queue (Element Size: %zu B).", sizeof(Command));

    // 2. Enqueue elements (FIFO order)
    Command cmd1 = {.command_id = 1, .priority = 0.5f};
    Command cmd2 = {.command_id = 2, .priority = 0.9f};
    Command cmd3 = {.command_id = 3, .priority = 0.1f};

    forgeQueueEnqueue(&queue, &cmd1);
    forgeQueueEnqueue(&queue, &cmd2);
    forgeQueueEnqueue(&queue, &cmd3);

    FORGE_LOG_INFO("Enqueued 3 commands. Queue size: %zu", forgeQueueSize(&queue));

    // 3. Peek at the head element (without removing it)
    Command* front_cmd = (Command*)forgeQueuePeek(&queue);
    if (front_cmd) {
        FORGE_LOG_INFO("Peek front -> Command ID: %d (Priority: %.1f)", 
                       front_cmd->command_id, front_cmd->priority);
    }

    // 4. Dequeue elements in FIFO order
    Command dequeued = {0};
    while (forgeQueueSize(&queue) > 0) {
        if (forgeQueueDequeue(&queue, &dequeued)) {
            FORGE_LOG_INFO("Dequeued -> Command ID: %d (Remaining size: %zu)", 
                           dequeued.command_id, forgeQueueSize(&queue));
        }
    }

    // 5. Test enqueueing after complete drain (verifies head cursor reset/compaction)
    Command cmd4 = {.command_id = 4, .priority = 1.0f};
    forgeQueueEnqueue(&queue, &cmd4);

    FORGE_LOG_INFO("Enqueued Command ID: %d after full drain. Queue size: %zu", 
                   cmd4.command_id, forgeQueueSize(&queue));

    // 6. Clean up backing dynamic array
    forgeQueueDestroy(&queue);
    FORGE_LOG_INFO("Queue destroyed cleanly.");

    return 0;
}
