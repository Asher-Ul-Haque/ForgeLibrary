/**
 * @file threadPoolExample.c
 * @brief Simple guide showing how to use ForgeThreadPool for multi-threaded work dispatch.
 */

#include <forgeUtils/core/logger.h>
#include <stdlib.h>
#include <forgeUtils/system/threadPool.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>

typedef struct TaskData {
    int task_id;
    int input_value;
    int result;
} TaskData;

// Task worker callback executed on worker threads
static void process_task(void* arg) {
    TaskData* data = (TaskData*)arg;
    
    FORGE_LOG_INFO("  [Worker Thread] Starting Task #%d (Input: %d)...", 
                   data->task_id, data->input_value);
    
    // Simulate computational work
    sleep(rand() % 5); // 50ms sleep
    data->result = data->input_value * 2;

    FORGE_LOG_INFO("  [Worker Thread] Completed Task #%d -> Result: %d", 
                   data->task_id, data->result);
}

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("        FORGE THREAD POOL EXAMPLE           ");
    FORGE_LOG_INFO("============================================");

    // 1. Create Thread Pool with 4 worker threads and queue capacity of 16
    ForgeThreadPool pool = {0};
    size_t worker_threads = 4;
    size_t queue_capacity = 16;

    if (!forgeThreadpoolCreate(&pool, worker_threads, queue_capacity, NULL)) {
        FORGE_LOG_ERROR("Failed to create thread pool!");
        return 1;
    }

    FORGE_LOG_INFO("Created Thread Pool with %zu worker threads (Queue capacity: %zu).", 
                   pool.threadCount, pool.queueCapacity);

    // 2. Dispatch 8 tasks to the pool
    TaskData tasks[8];
    for (int i = 0; i < 8; ++i) {
        tasks[i].task_id = i + 1;
        tasks[i].input_value = (i + 1) * 10;
        tasks[i].result = 0;

        bool enqueued = forgeThreadpoolAddTask(&pool, process_task, &tasks[i]);
        if (!enqueued) {
            FORGE_LOG_ERROR("Failed to enqueue task #%d", i + 1);
        }
    }

    FORGE_LOG_INFO("Dispatched 8 tasks to pool. Pending tasks: %zu", 
                   forgeThreadpoolPendingTasks(&pool));

    // 3. Wait for all tasks in the pool to complete processing
    FORGE_LOG_INFO("Waiting for worker threads to finish...");
    forgeThreadpoolWait(&pool);
    FORGE_LOG_INFO("All tasks finished execution! Pending tasks: %zu", 
                   forgeThreadpoolPendingTasks(&pool));

    // 4. Verify results
    FORGE_LOG_INFO("--- Task Results Verification ---");
    for (int i = 0; i < 8; ++i) {
        FORGE_LOG_INFO("  Task #%d: Input %d -> Output %d", 
                       tasks[i].task_id, tasks[i].input_value, tasks[i].result);
    }

    // 5. Clean up thread pool and worker handles
    forgeThreadpoolDestroy(&pool);
    FORGE_LOG_INFO("Thread pool destroyed cleanly.");

    return 0;
}
