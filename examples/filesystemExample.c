/**
 * @file filesystemExample.c
 * @brief Simple guide showing how to use the UNIX-only filesystem utilities.
 */

#include <core/logger.h>
#include <system/filesystem.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

// Callback function for directory listing
static void dir_list_callback(const char* entry_name, bool is_directory, void* user_data) {
    int* count = (int*)user_data;
    (*count)++;
    FORGE_LOG_INFO("  [%d] %s %s", *count, is_directory ? "[DIR] " : "[FILE]", entry_name);
}

int main(void) {
    FORGE_LOG_INFO("============================================");
    FORGE_LOG_INFO("         FORGE FILESYSTEM EXAMPLE           ");
    FORGE_LOG_INFO("============================================");

    // 1. Path Manipulation
    char joined_path[256] = {0};
    char norm_path[256] = {0};
    char parent_path[256] = {0};

    forgePathJoin("./test_dir/..", "demo_file.txt", joined_path, sizeof(joined_path));
    forgePathNormalize(joined_path, norm_path, sizeof(norm_path));
    forgePathParent(norm_path, parent_path, sizeof(parent_path));

    FORGE_LOG_INFO("Raw Joined Path  : %s", joined_path);
    FORGE_LOG_INFO("Normalized Path  : %s", norm_path);
    FORGE_LOG_INFO("Parent Directory : %s", parent_path);
    FORGE_LOG_INFO("Filename Part    : %s", forgePathFilename(norm_path));
    FORGE_LOG_INFO("Extension Part   : %s", forgePathExtension(norm_path));

    // 2. File Creation and Writing
    const char* test_file = "forge_fs_test.txt";
    ForgeFile file = {0};

    if (forgeFileOpen(&file, test_file, FORGE_FILE_WRITE)) {
        const char* content = "Hello, Forge Engine Filesystem!\nLine 2 of data.";
        size_t bytes_written = 0;
        
        forgeFileWrite(&file, content, strlen(content), &bytes_written);
        forgeFileFlush(&file);
        
        FORGE_LOG_INFO("Created and wrote %zu bytes to '%s'.", bytes_written, test_file);
        FORGE_LOG_INFO("File size via handle: %llu bytes", (unsigned long long)forgeFileSize(&file));
        
        forgeFileClose(&file);
    } else {
        FORGE_LOG_ERROR("Failed to open file for writing!");
        return 1;
    }

    // 3. File System Queries
    FORGE_LOG_INFO("File exists? %s", forgeFileExists(test_file) ? "YES" : "NO");
    FORGE_LOG_INFO("Is file? %s", forgeIsFile(test_file) ? "YES" : "NO");
    FORGE_LOG_INFO("Is directory? %s", forgeIsDirectory(test_file) ? "YES" : "NO");
    FORGE_LOG_INFO("File size by path: %llu bytes", (unsigned long long)forgeGetFileSize(test_file));

    // 4. File Reading
    if (forgeFileOpen(&file, test_file, FORGE_FILE_READ)) {
        char read_buffer[128] = {0};
        size_t bytes_read = 0;
        
        forgeFileRead(&file, read_buffer, sizeof(read_buffer) - 1, &bytes_read);
        read_buffer[bytes_read] = '\0';
        
        FORGE_LOG_INFO("Read %zu bytes back from file:\n---\n%s\n---", bytes_read, read_buffer);
        forgeFileClose(&file);
    }

    // 5. Zero-Allocation Directory Iteration
    FORGE_LOG_INFO("--- Current Directory Listing ---");
    int entry_count = 0;
    forgeListDir(".", dir_list_callback, &entry_count);

    // 6. Cleanup Test File
    if (forgeFileRemove(test_file)) {
        FORGE_LOG_INFO("Test file '%s' removed successfully.", test_file);
    }

    FORGE_LOG_INFO("Filesystem example completed cleanly.");

    return 0;
}
