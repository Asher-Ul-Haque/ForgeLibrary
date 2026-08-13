/**
 * @file : filesystem.h 
 * @brief : UNIX only file handling utilities
 */

#pragma once

#include <memory/linearAlloc.h>
#include <dataStructures/dynamicArray.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


// - - - File Handling Enums - - - 

///@brief : File modes
typedef enum ForgeFileMode 
{
  FORGE_FILE_READ,        ///< Open existing file for reading
  FORGE_FILE_WRITE,       ///< Truncate or create file for writing
  FORGE_FILE_APPEND,      ///< Append to end or create file for writing
  FORGE_FILE_READ_WRITE   ///< Open for both reading and writing (no truncation)
} ForgeFileMode;

///@brief : Cursor positions
typedef enum ForgeSeekOrigin 
{
  FORGE_SEEK_SET,  ///< From beginning of file
  FORGE_SEEK_CUR,  ///< From current cursor position
  FORGE_SEEK_END   ///< From end of file
} ForgeSeekOrigin;

///@brief : What a file is
typedef struct ForgeFile 
{
  int32_t fd;     ///< POSIX file descriptor
  bool    isOpen; ///< True if handle is currently open
} ForgeFile;

/// @brief : Callback type for zero-allocation directory iteration
typedef void (*ForgeDirIterCallback)(const char* ENTRY_NAME, bool IS_DIRECTORY, void* USER_DATA);


// - - - Path Manipulation - - - 

/**
 * @brief : Normalizes a path by collapsing duplicate slashes and resolving . / .. tokens.
 * @param PATH : Raw input path.
 * @param OUT_BUFFER : Output buffer to write normalized string to.
 * @param BUFFER_SIZE : Capacity of out_buffer.
 * @return : true if successful, false if buffer is too small.
 */
bool forgePathNormalize(
  const char* PATH, 
  char*       OUT_BUFFER, 
  size_t      BUFFER_SIZE);

/**
 * @brief : Joins two path components with a single path separator.
 * @param PART_1: First path 
 * @param PART_2 : Second part 
 * @param OUT_BUFFER : A string buffer big enough to store the join
 * @param BUFFER_SIZE : The size of the buffer
 * @return : true if successful, false if buffer is too small.
 */
bool forgePathJoin(
  const char* PART_1, 
  const char* PART_2, 
  char*       OUT_BUFFER, 
  size_t      BUFFER_SIZE);

/**
 * @brief : Extracts the parent directory component from a path.
 * @param PATH : Child path
 * @param OUT_BUFFER : Buffer that stores the parent path 
 * @param BUFFER_SIZE : The size of the buffer
 * @return : true if successful, false if buffer is too small.
 */
bool forgePathParent(
  const char* PATH, 
  char*       OUT_BUFFER, 
  size_t      BUFFER_SIZE);

/**
 * @brief : Returns a pointer to the filename component within the path string (no copy).
 * @param PATH : The path 
 * @return : pointer to the filename within the path
 */
const char* forgePathFilename(const char* PATH);

/**
 * @brief : Returns a pointer to the extension within the path string (e.g. "txt" or "png", no dot).
 * @param PATH : The file path 
 * @return : pointer to the extension back in the PATH
 */
const char* forgePathExtension(const char* PATH);


// - - - Handle-Based File I/O - - - 

/**
 * @brief : opens a file 
 * @param FILE : Pointer to a file 
 * @param PATH : File path 
 * @param MODE : Open mode 
 * @return : true if opened, false if not
 */
bool forgeFileOpen(
  ForgeFile*    FILE,
  const char*   PATH,
  ForgeFileMode MODE);

/**
 * @brief : closes a file 
 * @param FILE : pointer to the file to be closed
 */
void forgeFileClose(ForgeFile* FILE);

/**
 * @brief : reads a file 
 * @param FILE : Pointer to the file to be read 
 * @param OUT_BUFFER : Where to write file contents 
 * @param BYTES_TO_READ : How many bytes at max to read 
 * @param OUT_BYTES_READ : The function will fill this with how many bytes were read 
 * @return : true is successful, false if not
 */
bool forgeFileRead(
  ForgeFile*  FILE,
  void*       OUT_BUFFER,
  size_t      BYTES_TO_READ,
  size_t*     OUT_BYTES_READ);

/**
 * @brief : writes a file 
 * @param FILE : Pointer to the file to be written 
 * @param BUFFER : What to write 
 * @param BYTES_TO_WRITE : How many bytes to write 
 * @param OUT_BYTES_WRITTEN : Function will fill this with how many bytes were written 
 * @return : true if successful, false if not
 */
bool forgeFileWrite(
  ForgeFile*  FILE, 
  const void* BUFFER, 
  size_t      BYTES_TO_WRITE, 
  size_t*     OUT_BYTES_WRITTEN);

/**
 * @brief : sets cursor position in file 
 * @param FILE : Which file to set cursor in 
 * @param OFFSET : How much to move the cursor 
 * @param ORIGIN : Where to move the cursor from
 * @return : true if successful, false if not
 */
bool forgeFileSeek(
  ForgeFile*      FILE, 
  int64_t         OFFSET, 
  ForgeSeekOrigin ORIGIN);

/**
 * @brief : Tells where the cursor is 
 * @param FILE : Which file's cursor to check 
 * @return : position of the cursor 
 */
uint64_t forgeFileTell(ForgeFile* FILE);

/**
 * @brief : Tells the size of the file 
 * @param FILE : The file whose size is to be checked
 * @return : size of the file in bytes
 */
uint64_t forgeFileSize(ForgeFile* FILE);

/**
 * @brief : Commits all changes to the file now 
 * @param FILE : Which file to commit to disk 
 * @return : true if successful, false otherwise
 */
bool forgeFileFlush(ForgeFile* FILE);


// - - - File System Checks & Operations - - - 

/**
 * @brief : Find whether a file at a path exists 
 * @param PATH : The file path 
 * @return : true if it exists, false otherwise
 */
bool forgeFileExists(const char* PATH);

/**
 * @brief : Find whether a path is a file or not 
 * @param PATH : The file path 
 * @return : true if it is a file, false otherwise
 */
bool forgeIsFile(const char* PATH);

/**
 * @brief : Find whether a path is a folder or not 
 * @param PATH : The folder path 
 * @return : true if it is a folder, false otherwise
 */
bool forgeIsDirectory(const char* PATH);

/**
 * @brief : Provides size of the file at the given path 
 * @param PATH : The file path 
 * @return : size of the file in bytes
 */
uint64_t forgeGetFileSize(const char* PATH);

/**
 * @brief : Make directory 
 * @param PATH : Directory path 
 * @return : true if successful, false otherwise
 */
bool forgeMkdir(const char* PATH);

/**
 * @brief : removes a file at the given path 
 * @param PATH : File path 
 * @return : true if delted, false otherwise
 */
bool forgeFileRemove(const char* PATH);

/**
 * @brief : Renames a file at the given path 
 * @param OLD_PATH : current path 
 * @param NEW_PATH : path to move to 
 * @return : true if successful, false otherwise
 */
bool forgeFileRename(const char* OLD_PATH, const char* NEW_PATH);

/**
 * @brief : Zero-allocation directory iteration calling callback for each entry.
 * @param PATH : Directory path 
 * @param CALLBACK : Directory iteration callback function 
 * @param USER_DATA : Optional user data to hold around 
 */
bool forgeListDir(
  const char*           PATH,
  ForgeDirIterCallback  CALLBACK,
  void*                 USER_DATA);

#ifdef __cplusplus
}
#endif
