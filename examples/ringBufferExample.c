/**
 * @file ringBufferExample.c
 * @brief Simple guide showing how to use ForgeRingBuffer for circular buffering.
 */

#include <core/logger.h>
#include <dataStructures/ringBuffer.h>

#include <stdbool.h>
#include <stddef.h>

typedef struct AudioSample {
    float left;
    float right;
} AudioSample;

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("         FORGE RING BUFFER EXAMPLE          ");
    FORGE_LOG_INFO("============================================");

    // 1. Create Ring Buffer with capacity of 4 (ALLOW_OVERWRITE = true)
    ForgeRingBuffer ring = {0};
    size_t capacity = 4;
    
    if (!forgeRingBufferCreate(&ring, capacity, sizeof(AudioSample), true, NULL)) {
        FORGE_LOG_ERROR("Failed to create ring buffer!");
        return 1;
    }

    FORGE_LOG_INFO("Created Ring Buffer (Capacity: %zu, Element Size: %zu B, Overwrite: YES).", 
                   ring.capacity, ring.elementSize);

    // 2. Fill the buffer to capacity
    AudioSample s1 = {1.0f, 1.0f};
    AudioSample s2 = {2.0f, 2.0f};
    AudioSample s3 = {3.0f, 3.0f};
    AudioSample s4 = {4.0f, 4.0f};

    forgeRingBufferPush(&ring, &s1);
    forgeRingBufferPush(&ring, &s2);
    forgeRingBufferPush(&ring, &s3);
    forgeRingBufferPush(&ring, &s4);

    FORGE_LOG_INFO("Pushed 4 samples. Count: %zu/%zu, Is Full? %s", 
                   forgeRingBufferSize(&ring), ring.capacity, 
                   forgeRingBufferIsFull(&ring) ? "YES" : "NO");

    // 3. Peek at the oldest item (tail)
    AudioSample* oldest = (AudioSample*)forgeRingBufferPeek(&ring);
    if (oldest) {
        FORGE_LOG_INFO("Peek oldest sample -> L: %.1f, R: %.1f", oldest->left, oldest->right);
    }

    // 4. Overwrite test: Push a 5th sample into a full buffer
    // Since allowOverwrite = true, sample 1 (s1) is dropped and tail advances!
    AudioSample s5 = {5.0f, 5.0f};
    FORGE_LOG_INFO("Pushing 5th sample into full buffer (triggers overwrite)...");
    forgeRingBufferPush(&ring, &s5);

    // Peek should now reveal s2 (since s1 was overwritten)
    oldest = (AudioSample*)forgeRingBufferPeek(&ring);
    if (oldest) {
        FORGE_LOG_INFO("Peek oldest sample after overwrite -> L: %.1f, R: %.1f", oldest->left, oldest->right);
    }

    // 5. Pop all elements (reads circularly until empty)
    FORGE_LOG_INFO("--- Reading All Samples ---");
    AudioSample popped = {0};
    while (!forgeRingBufferIsEmpty(&ring)) {
        if (forgeRingBufferPop(&ring, &popped)) {
            FORGE_LOG_INFO("Popped sample -> L: %.1f, R: %.1f (Remaining: %zu)", 
                           popped.left, popped.right, forgeRingBufferSize(&ring));
        }
    }

    FORGE_LOG_INFO("Buffer is empty? %s", forgeRingBufferIsEmpty(&ring) ? "YES" : "NO");

    // 6. Test wraparound pushing/popping
    FORGE_LOG_INFO("Testing wraparound indexing...");
    AudioSample s6 = {6.0f, 6.0f};
    AudioSample s7 = {7.0f, 7.0f};

    forgeRingBufferPush(&ring, &s6);
    forgeRingBufferPush(&ring, &s7);

    forgeRingBufferPop(&ring, &popped);
    FORGE_LOG_INFO("Popped after wrap -> L: %.1f, R: %.1f", popped.left, popped.right);

    // 7. Clear buffer
    forgeRingBufferClear(&ring);
    FORGE_LOG_INFO("Cleared ring buffer. Count: %zu", forgeRingBufferSize(&ring));

    // 8. Clean up memory
    forgeRingBufferDestroy(&ring);
    FORGE_LOG_INFO("Ring buffer destroyed cleanly.");

    return 0;
}
