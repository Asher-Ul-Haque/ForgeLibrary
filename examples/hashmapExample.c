/**
 * @file hashMapExample.c
 * @brief Simple guide showing how to use ForgeHashMap for key-value storage.
 */

#include <forgeUtils/core/logger.h>
#include <forgeUtils/dataStructures/hashMap.h>

#include <stdbool.h>
#include <stddef.h>
#include <string.h>

typedef struct UserProfile {
    int user_id;
    float score;
    bool is_active;
} UserProfile;

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("         FORGE HASH MAP EXAMPLE             ");
    FORGE_LOG_INFO("============================================");

    // 1. Create HashMap mapping integer keys (user_id) to UserProfile structs
    ForgeHashMap user_map = {0};
    if (!forgeHashmapCreate(&user_map, sizeof(int), sizeof(UserProfile), MAP_DEFAULT_CAPACITY, NULL, NULL, NULL)) {
        FORGE_LOG_ERROR("Failed to create user hash map!");
        return 1;
    }

    FORGE_LOG_INFO("Created HashMap (Key: int [%zu B], Value: UserProfile [%zu B]).", 
                   user_map.keySize, user_map.valueSize);

    // 2. Insert entries into hashmap (forgeHashmapSet)
    int key1 = 1001;
    UserProfile profile1 = {.user_id = 1001, .score = 88.5f, .is_active = true};

    int key2 = 1002;
    UserProfile profile2 = {.user_id = 1002, .score = 94.0f, .is_active = true};

    int key3 = 1003;
    UserProfile profile3 = {.user_id = 1003, .score = 72.3f, .is_active = false};

    forgeHashmapSet(&user_map, &key1, &profile1);
    forgeHashmapSet(&user_map, &key2, &profile2);
    forgeHashmapSet(&user_map, &key3, &profile3);

    FORGE_LOG_INFO("Inserted 3 profiles. Map count: %zu", forgeHashmapSize(&user_map));

    // 3. Lookup entry (forgeHashmapGet)
    UserProfile* retrieved = (UserProfile*)forgeHashmapGet(&user_map, &key2);
    if (retrieved) {
        FORGE_LOG_INFO("Retrieved Key 1002 -> Score: %.1f, Active: %s", 
                       retrieved->score, retrieved->is_active ? "true" : "false");
    }

    // 4. Update/Overwrite existing key
    UserProfile profile2_updated = {.user_id = 1002, .score = 99.9f, .is_active = true};
    forgeHashmapSet(&user_map, &key2, &profile2_updated);

    retrieved = (UserProfile*)forgeHashmapGet(&user_map, &key2);
    if (retrieved) {
        FORGE_LOG_INFO("Updated Key 1002 -> New Score: %.1f", retrieved->score);
    }

    // 5. Check existence (forgeHashmapContains)
    int missing_key = 9999;
    FORGE_LOG_INFO("Contains key 1001? %s", forgeHashmapContains(&user_map, &key1) ? "YES" : "NO");
    FORGE_LOG_INFO("Contains key 9999? %s", forgeHashmapContains(&user_map, &missing_key) ? "YES" : "NO");

    // 6. Remove an entry (forgeHashmapRemove)
    FORGE_LOG_INFO("Removing key 1003...");
    if (forgeHashmapRemove(&user_map, &key3)) {
        FORGE_LOG_INFO("Key 1003 removed. New map count: %zu", forgeHashmapSize(&user_map));
    }

    // Verify key 1003 is gone
    FORGE_LOG_INFO("Contains key 1003 after removal? %s", 
                   forgeHashmapContains(&user_map, &key3) ? "YES" : "NO");

    // 7. Clear the hashmap
    forgeHashmapClear(&user_map);
    FORGE_LOG_INFO("Cleared HashMap. Count: %zu, Capacity retained: %zu", 
                   forgeHashmapSize(&user_map), user_map.capacity);

    // 8. Clean up backing buffers
    forgeHashmapDestroy(&user_map);
    FORGE_LOG_INFO("HashMap destroyed cleanly.");

    return 0;
}
