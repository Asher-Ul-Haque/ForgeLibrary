/**
 * @file objectPoolExample.c
 * @brief Simple guide showing how to use the Object Pool for fixed-size allocations.
 */

#include <forgeUtils/core/logger.h>
#include <forgeUtils/memory/objectPool.h>

#include <stdint.h>
#include <stdbool.h>

// Particle struct used in game engine / particle system
typedef struct Particle {
    float position[3];
    float velocity[3];
    float lifetime;
    uint32_t id;
} Particle;

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("        FORGE OBJECT POOL EXAMPLE           ");
    FORGE_LOG_INFO("============================================");

    // 1. Create an Object Pool for 10 Particles
    ForgeObjectPool pool = {0};
    size_t pool_capacity = 10;
    
    if (!forgeObjectPoolCreate(&pool, pool_capacity, sizeof(Particle), NULL)) {
        FORGE_LOG_ERROR("Failed to create object pool!");
        return 1;
    }

    FORGE_LOG_INFO("Created Object Pool for %zu Particles (%zu bytes each).", 
                   pool_capacity, sizeof(Particle));
    forgeObjectPoolDebugPrint(&pool);

    // 2. Allocate 8 particles from the pool
    Particle* particles[8];
    for (int i = 0; i < 8; ++i) {
        particles[i] = (Particle*)forgeObjectPoolTakeObject(&pool);
        if (particles[i]) {
            particles[i]->id = (uint32_t)(i + 1);
            particles[i]->lifetime = 5.0f;
        }
    }

    FORGE_LOG_INFO("Allocated 8 particles from pool.");
    forgeObjectPoolDebugPrint(&pool);

    // 3. Return 3 particles back to the pool (recycling memory)
    FORGE_LOG_INFO("Returning particles #2, #4, and #5 back to pool...");
    forgeObjectPoolReturnObject(&pool, particles[1]);
    forgeObjectPoolReturnObject(&pool, particles[3]);
    forgeObjectPoolReturnObject(&pool, particles[4]);

    forgeObjectPoolDebugPrint(&pool);

    // 4. Allocate 2 new particles (takes recycled slots from free list)
    Particle* new_p1 = (Particle*)forgeObjectPoolTakeObject(&pool);
    Particle* new_p2 = (Particle*)forgeObjectPoolTakeObject(&pool);
    
    if (new_p1) new_p1->id = 99;
    if (new_p2) new_p2->id = 100;

    FORGE_LOG_INFO("Allocated 2 new particles into recycled slots.");
    forgeObjectPoolDebugPrint(&pool);

    // 5. Clean up the object pool
    forgeObjectPoolDestroy(&pool);
    FORGE_LOG_INFO("Object pool destroyed cleanly.");

    return 0;
}
