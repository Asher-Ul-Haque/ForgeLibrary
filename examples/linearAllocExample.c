/**
 * @file linearAllocExample.c
 * @brief Simple guide showing how to use the Linear (Arena) Allocator.
 */

#include <forgeUtils/core/logger.h>
#include <forgeUtils/memory/linearAlloc.h>

#include <stdint.h>
#include <stdbool.h>

typedef struct Transform {
    float position[3];
    float rotation[4];
    float scale[3];
} Transform;

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("       FORGE LINEAR ALLOCATOR EXAMPLE       ");
    FORGE_LOG_INFO("============================================");

    // 1. Create a 4 KB Linear Allocator (Standard 1-page size)
    ForgeLinearAllocator arena = {0};
    size_t arena_size = 4 * 1024; // 4096 bytes
    
    if (!forgeLinearAllocCreate(&arena, arena_size, NULL, false)) {
        FORGE_LOG_ERROR("Failed to create linear allocator!");
        return 1;
    }

    FORGE_LOG_INFO("Created 4 KB Arena Allocator.");
    forgeLinearAllocDebugPrint(&arena);

    // 2. Allocate 500 integers (2000 bytes ~ 48.8% of capacity)
    size_t count = 500;
    int* numbers = (int*)forgeLinearAllocAllocate(&arena, count * sizeof(int), 0);
    for (size_t i = 0; i < count; ++i) {
        numbers[i] = (int)i;
    }
    FORGE_LOG_INFO("Allocated 500 integers.");

    // 3. Allocate 20 Transform components (~800 bytes)
    size_t transform_count = 20;
    Transform* transforms = (Transform*)forgeLinearAllocAllocate(
        &arena, 
        transform_count * sizeof(Transform), 
        16
    );
    (void)transforms;
    FORGE_LOG_INFO("Allocated 20 Transform structs.");

    // 4. Check the visual bar now! (~68% used)
    forgeLinearAllocDebugPrint(&arena);

    // 5. Reset the arena
    FORGE_LOG_INFO("Resetting arena allocations...");
    forgeLinearAllocFree(&arena, arena.allocated);
    forgeLinearAllocDebugPrint(&arena);

    // 6. Clean up
    forgeLinearAllocDestroy(&arena);
    FORGE_LOG_INFO("Linear allocator destroyed cleanly.");

    return 0;
}
