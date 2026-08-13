/**
 * @file assertExample.c
 * @brief Simple guide showing how to use compile-time asserts, runtime checks, and TODOs.
 */

#include <core/asserts.h>
#include <core/logger.h>

#include <stddef.h>
#include <stdbool.h>

// Example data structure
typedef struct Player {
    int health;
    float position[3];
    bool is_alive;
} Player;

// Global compile-time assertions
FORGE_COMPILE_TIME_ASSERT(sizeof(Player) == 20);
FORGE_COMPILE_TIME_ASSERT(sizeof(int) >= 4);

void update_player_health(Player* player, int damage) {
    // Basic invariant check: player pointer must never be NULL
    FORGE_ASSERT(player != NULL);

    // Custom message assertion
    FORGE_ASSERT_MESSAGE(player->is_alive, "Cannot update health of a dead player!");

    player->health -= damage;
    if (player->health <= 0) {
        player->health = 0;
        player->is_alive = false;
    }
}

void load_player_save(const char* filepath) {
    // Debug-only assertion
    FORGE_ASSERT_DEBUG(filepath != NULL);

    // TODO macro demo (uncomment to test debug abort)
    TODO_COMMENT("Implement binary save file parser");
}

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("          FORGE ASSERTS EXAMPLE             ");
    FORGE_LOG_INFO("============================================");

    // Compile-time assertion inside function scope
    FORGE_COMPILE_TIME_ASSERT(sizeof(float) == 4);

    Player hero = {
        .health = 100,
        .position = {0.0f, 0.0f, 0.0f},
        .is_alive = true
    };

    FORGE_LOG_INFO("Player initialized with %d health.", hero.health);

    FORGE_LOG_INFO("Applying 30 damage to player...");
    update_player_health(&hero, 30);
    FORGE_LOG_INFO("Player health is now: %d", hero.health);

    FORGE_LOG_INFO("Applying 80 damage to player...");
    update_player_health(&hero, 80);
    FORGE_LOG_INFO("Player is alive: %s", hero.is_alive ? "true" : "false");

    load_player_save("save.dat");
    // Uncomment to test assertion failure:
    // update_player_health(NULL, 10);

    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("All passing assertion checks executed cleanly!");

    return 0;
}
