/**
 * @file trackerExample.c
 * @brief Demonstrates memory tracking, calloc zeroing, bounds validation, and leak reporting.
 */

#include <forgeUtils/core/logger.h>
#include <forgeUtils/memory/tracker.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("        FORGE MEMORY TRACKER EXAMPLE        ");
    FORGE_LOG_INFO("============================================");

    // 1. Initial tracking state
    FORGE_LOG_INFO("Active allocated bytes at start: %zu bytes", forgeMemoryGetActiveBytes());

    // 2. Allocating via FORGE_MALLOC and FORGE_CALLOC
    int* numbers = (int*)FORGE_MALLOC(10 * sizeof(int));
    float* zeros = (float*)FORGE_CALLOC(5, sizeof(float));

    FORGE_LOG_INFO("Allocated int array (10 items) and float array (5 items).");
    FORGE_LOG_INFO("Active allocated bytes: %zu bytes", forgeMemoryGetActiveBytes());

    // 3. Resize memory with FORGE_REALLOC
    numbers = (int*)FORGE_REALLOC(numbers, 20 * sizeof(int));
    FORGE_LOG_INFO("Resized int array to 20 items.");
    FORGE_LOG_INFO("Active allocated bytes: %zu bytes", forgeMemoryGetActiveBytes());

    // 4. Validate memory bounds against canary corruption
    if (forgeMemoryCheckBounds()) {
        FORGE_LOG_INFO("Memory bounds check PASSED: All canaries are intact.");
    } else {
        FORGE_LOG_ERROR("Memory bounds check FAILED: Buffer overflow detected!");
    }

    // 5. Intentionally create a small leak for demonstration
    void* leaked_block = FORGE_MALLOC(64);
    (void)leaked_block; // Unused, intentionally not calling FORGE_FREE(leaked_block)

    // 6. Clean up legitimate allocations
    FORGE_FREE(numbers);
    FORGE_FREE(zeros);

    FORGE_LOG_INFO("Cleaned up valid blocks.");
    FORGE_LOG_INFO("Active allocated bytes remaining: %zu bytes", forgeMemoryGetActiveBytes());

    // 7. Report any memory leaks before exiting
    FORGE_LOG_INFO("--- Reporting Memory Leaks ---");
    forgeMemoryReportLeaks();

    // Clean up the intentional leak so the test process finishes cleanly
    FORGE_FREE(leaked_block);

    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("Memory tracker example completed.");

    return 0;
}
