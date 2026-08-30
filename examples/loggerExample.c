/**
 * @file logger_example.c
 * @brief Demonstrates usage of the forgeUtils logging system.
 */

#include <forgeUtils/core/logger.h>

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("          FORGE LOGGER EXAMPLE              ");
    FORGE_LOG_INFO("============================================");

    // Logging formatted strings with arguments
    const char* app_name = "ForgeEngine";
    int version_major = 1;
    int version_minor = 0;
    FORGE_LOG_INFO("Initializing %s version %d.%d...", app_name, version_major, version_minor);

    // Severity levels
    FORGE_LOG_TRACE("This is a TRACE message (used for granular step-by-step logging).");
    FORGE_LOG_DEBUG("This is a DEBUG message (Variable x = %d, Pointer = %p).", 42, (void*)app_name);
    FORGE_LOG_INFO("This is an INFO message (Normal operational logs).");
    FORGE_LOG_WARNING("This is a WARNING message (e.g. FPS dropped below %d).", 60);
    FORGE_LOG_ERROR("This is an ERROR message (e.g. Failed to load texture '%s').", "missing_hero.png");
    FORGE_LOG_FATAL("This is a FATAL message (Unrecoverable system state!).");

    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("Logging demonstration complete!");

    return 0;
}
