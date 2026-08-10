#include <stdint.h>
#include <system/filesystem.h>
#include <core/asserts.h>
#include <memory/tracker.h>
#include <core/logger.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/// @brief: Path Manipulation Implementation
bool forgePathNormalize(
  const char* PATH,
  char*       OUT_BUFFER,
  size_t      BUFFER_SIZE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(PATH != NULL, "[FILE SYSTEM] : Cannot normalize a NULL PATH");
  FORGE_ASSERT_DEBUG_MESSAGE(OUT_BUFFER != NULL, "[FILE SYSTEM] : Cannot normalize to a NULL OUT_BUFFER");
  FORGE_ASSERT_DEBUG_MESSAGE(BUFFER_SIZE > 0, "[FILE SYSTEM] : Cannot noramlize with 0 BUFFER_SIZE");

  if (PATH[0] == '\0') 
  {
    if (BUFFER_SIZE < 2) return false;
    OUT_BUFFER[0] = '.';
    OUT_BUFFER[1] = '\0';
    return true;
  }

  size_t len = strlen(PATH);
  if (len >= BUFFER_SIZE) return false;

  char temp[4096];
  if (len >= sizeof(temp)) return false;

  // - - - 1. Copy and collapse multiple consecutive slashes
  size_t  r = 0, w = 0;
  bool    isAbsolute = (PATH[0] == '/');

  if (isAbsolute) 
  {
    temp[w++] = '/';
    r++;
  }

  while (r < len) 
  {
    if (PATH[r] == '/') 
    {
      while (r < len && PATH[r] == '/') r++;
      if (w > 0 && temp[w - 1] != '/') 
      { temp[w++] = '/'; }
    }
    else 
    { temp[w++] = PATH[r++]; }
  }
  temp[w] = '\0';

  // - - - 2. Tokenize segments and resolve . and ..
  char*   segments[256];
  size_t  segCount = 0;

  char* token = strtok(temp, "/");
  while (token) 
  {
    if (strcmp(token, ".") == 0) 
    { 
      // Ignore current dir dot 
    }
    else if (strcmp(token, "..") == 0) 
    {
      if (segCount > 0 && strcmp(segments[segCount - 1], "..") != 0) 
      { segCount--; } 
      else if (!isAbsolute) 
      { segments[segCount++] = token; }
    } 
    else 
    { segments[segCount++] = token; }
    token = strtok(NULL, "/");
  }

  // - - - 3. Reconstruct final normalized path
  OUT_BUFFER[0] = '\0';
  size_t outLen = 0;

  if (isAbsolute) 
  {
    OUT_BUFFER[outLen++]  = '/';
    OUT_BUFFER[outLen]    = '\0';
  }

  if (segCount == 0 && !isAbsolute) 
  {
    if (BUFFER_SIZE < 2) return false;
    OUT_BUFFER[0] = '.';
    OUT_BUFFER[1] = '\0';
    return true;
  }

  for (size_t i = 0; i < segCount; ++i) 
  {
    size_t seg_len = strlen(segments[i]);
    if (outLen + seg_len + (i > 0 || isAbsolute ? 1 : 0) >= BUFFER_SIZE) 
    { return false; }

    if (i > 0 || (isAbsolute && outLen > 1)) 
    {
      strcat(OUT_BUFFER, "/");
      outLen++;
    }
    strcat(OUT_BUFFER, segments[i]);
    outLen += seg_len;
  }

  return true;
}

bool forgePathJoin(
  const char* PART_1, 
  const char* PART_2, 
  char*       OUT_BUFFER, 
  size_t      BUFFER_SIZE) 
{
  FORGE_ASSERT(PART_1 != NULL);
  FORGE_ASSERT(PART_2 != NULL);
  FORGE_ASSERT(OUT_BUFFER != NULL);

  size_t len1 = strlen(PART_1);
  size_t len2 = strlen(PART_2);

  if (len1 == 0) 
  {
    return forgePathNormalize(PART_2, OUT_BUFFER, BUFFER_SIZE);
  }
  if (len2 == 0) 
  {
    return forgePathNormalize(PART_1, OUT_BUFFER, BUFFER_SIZE);
  }

  char rawCombined[8192];
  bool p1HasSlash = (PART_1[len1 - 1] == '/');
  bool p2HasSlash = (PART_2[0] == '/');

  if (p1HasSlash && p2HasSlash) 
  {
    snprintf(rawCombined, sizeof(rawCombined), "%s%s", PART_1, PART_2 + 1);
  } 
  else if (!p1HasSlash && !p2HasSlash) 
  {
    snprintf(rawCombined, sizeof(rawCombined), "%s/%s", PART_1, PART_2);
  } 
  else 
  {
    snprintf(rawCombined, sizeof(rawCombined), "%s%s", PART_1, PART_2);
  }

  return forgePathNormalize(rawCombined, OUT_BUFFER, BUFFER_SIZE);
}

bool forgePathParent(
  const char* PATH,
  char*       OUT_BUFFER,
  size_t      BUFFER_SIZE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(PATH != NULL, "[FILE SYSTEM] : Cannot find parent of NULL PATH");
  FORGE_ASSERT_DEBUG_MESSAGE(OUT_BUFFER != NULL, "[FILE SYSTEM] : ");

  char norm[4096];
  if (!forgePathNormalize(PATH, norm, sizeof(norm))) return false;

  char* lastSlash = strrchr(norm, '/');
  if (!lastSlash) 
  {
    if (BUFFER_SIZE < 2) return false;
    OUT_BUFFER[0] = '.';
    OUT_BUFFER[1] = '\0';
    return true;
  }

  // - - - Root dir `/`
  if (lastSlash == norm) 
  {
    if (BUFFER_SIZE < 2) return false;
    OUT_BUFFER[0] = '/';
    OUT_BUFFER[1] = '\0';
    return true;
  }

  size_t parentLen = (size_t)(lastSlash - norm);
  if (parentLen >= BUFFER_SIZE) return false;

  strncpy(OUT_BUFFER, norm, parentLen);
  OUT_BUFFER[parentLen] = '\0';
  return true;
}

const char* forgePathFilename(const char* PATH) 
{
  if (!PATH) return "";
  const char* lastSlash = strrchr(PATH, '/');
  return lastSlash ? (lastSlash + 1) : PATH;
}

const char* forgePathExtension(const char* PATH)
{
  const char* filename = forgePathFilename(PATH);
  const char* lastDot  = strrchr(filename, '.');

  if (!lastDot || lastDot == filename) return ""; 
  return lastDot + 1;
}


// - - - Handle-Based File I/O Implementation - - - 

bool forgeFileOpen(
  ForgeFile*    FILE,
  const char*   PATH,
  ForgeFileMode MODE)
{
  FORGE_ASSERT_DEBUG_MESSAGE(FILE != NULL, "[FILE SYSTEM] : Cannot open a NULL FILE");
  FORGE_ASSERT_DEBUG_MESSAGE(PATH != NULL, "[FILE SYSTEM] : Cannot open a file from a NULL PATH");

  int32_t flags       = 0;
  mode_t  permissions = 0644;

  switch (MODE) 
  {
    case FORGE_FILE_READ:
      flags = O_RDONLY;
      break;
    case FORGE_FILE_WRITE:
      flags = O_WRONLY | O_CREAT | O_TRUNC;
      break;
    case FORGE_FILE_APPEND:
      flags = O_WRONLY | O_CREAT | O_APPEND;
      break;
    case FORGE_FILE_READ_WRITE:
      flags = O_RDWR | O_CREAT;
      break;
  }

  int32_t fd = open(PATH, flags, permissions);
  if (fd < 0) 
  {
    FORGE_LOG_ERROR("[FILE SYSTEM] : Failed to open FILE '%s': %s", PATH, strerror(errno));
    FILE->fd      = -1;
    FILE->isOpen  = false;
    return false;
  }

  FILE->fd = fd;
  FILE->isOpen = true;
  return true;
}

void forgeFileClose(ForgeFile* FILE)
{
  if (!FILE || !FILE->isOpen) return;
  close(FILE->fd);
  FILE->fd      = -1;
  FILE->isOpen  = false;
}

bool forgeFileRead(
  ForgeFile*  FILE, 
  void*       OUT_BUFFER,
  size_t      BYTES_TO_READ, 
  size_t*     OUT_BYTES_READ) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(FILE != NULL, "[FILE SYSTEM] : Cannot set cursor of a NULL FILE");
  FORGE_ASSERT_DEBUG_MESSAGE(FILE->isOpen, "[FILE SYSTEM] : Cannot set cursor of a FILE that is not open")
  FORGE_ASSERT_DEBUG_MESSAGE(OUT_BUFFER != NULL, "[FILE SYSTEM] : Cannot read into a NULL OUT_BUFFER");

  ssize_t res = read(FILE->fd, OUT_BUFFER, BYTES_TO_READ);
  if (res < 0) 
  {
    FORGE_LOG_ERROR("[FILESYSTEM] Read error: %s", strerror(errno));
    if (OUT_BYTES_READ) *OUT_BYTES_READ = 0;
    return false;
  }

  if (OUT_BYTES_READ) *OUT_BYTES_READ = (size_t)res;
  return true;
}

bool forgeFileWrite(
  ForgeFile*  FILE,
  const void* BUFFER, 
  size_t      BYTES_TO_WRITE, 
  size_t*     OUT_BYTES_WRITTEN) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(FILE != NULL, "[FILE SYSTEM] : Cannot set cursor of a NULL FILE");
  FORGE_ASSERT_DEBUG_MESSAGE(FILE->isOpen, "[FILE SYSTEM] : Cannot set cursor of a FILE that is not open")
  FORGE_ASSERT_DEBUG_MESSAGE(BUFFER != NULL, "[FILE SYSTEM] : Cannot write a NULL BUFFER");

  ssize_t res = write(FILE->fd, BUFFER, BYTES_TO_WRITE);
  if (res < 0) 
  {
    FORGE_LOG_ERROR("[FILE SYSTEM] : Write error: %s", strerror(errno));
    if (OUT_BYTES_WRITTEN) *OUT_BYTES_WRITTEN = 0;
    return false;
  }

  if (OUT_BYTES_WRITTEN) *OUT_BYTES_WRITTEN = (size_t)res;
  return true;
}

bool forgeFileSeek(
  ForgeFile*      FILE, 
  int64_t         OFFSET, 
  ForgeSeekOrigin ORIGIN) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(FILE != NULL, "[FILE SYSTEM] : Cannot set cursor of a NULL FILE");
  FORGE_ASSERT_DEBUG_MESSAGE(FILE->isOpen, "[FILE SYSTEM] : Cannot set cursor of a FILE that is not open")

  int32_t whence = SEEK_SET;
  if      (ORIGIN == FORGE_SEEK_CUR) whence = SEEK_CUR;
  else if (ORIGIN == FORGE_SEEK_END) whence = SEEK_END;

  off_t res = lseek(FILE->fd, (off_t)OFFSET, whence);
  return (res != (off_t)-1);
}

uint64_t forgeFileTell(ForgeFile* FILE) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(FILE != NULL, "[FILE SYSTEM] : Cannot get cursor of a NULL FILE");
  FORGE_ASSERT_DEBUG_MESSAGE(FILE->isOpen, "[FILE SYSTEM] : Cannot check cursor of a FILE that is not open")

  return (uint64_t)lseek(FILE->fd, 0, SEEK_CUR);
}

uint64_t forgeFileSize(ForgeFile* FILE) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(FILE != NULL, "[FILE SYSTEM] : Cannot get size of a NULL FILE");
  FORGE_ASSERT_DEBUG_MESSAGE(FILE->isOpen, "[FILE SYSTEM] : Cannot check size of a FILE that is not open")

  struct stat st;
  if (fstat(FILE->fd, &st) != 0) return -1;
  return (uint64_t)st.st_size;
}

bool forgeFileFlush(ForgeFile* FILE) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(FILE != NULL, "[FILE SYSTEM] : Cannot flush a NULL FILE");
  return (fsync(FILE->fd) == 0);
}


// - - - File System Checks & Directory Operations Implementation - - - 

bool forgeFileExists(const char* PATH) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(PATH != NULL, "[FILE SYSTEM] : Cannot check if a NULL PATH exists or not. The answer is no btw")
  return (access(PATH, F_OK) == 0);
}

bool forgeIsFile(const char* PATH) 
{
  struct stat st;
  if (stat(PATH, &st) != 0) return false;
  return S_ISREG(st.st_mode);
}

bool forgeIsDirectory(const char* PATH) 
{
  struct stat st;
  if (stat(PATH, &st) != 0) return false;
  return S_ISDIR(st.st_mode);
}

uint64_t forgeGetFileSize(const char* PATH)
{
  struct stat st;
  if (stat(PATH, &st) != 0) return -1;
  return (uint64_t)st.st_size;
}

bool forgeMkdir(const char* PATH) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(PATH != NULL, "[FILE SYSTEM] : Cannot call make a directory at NULL PATH");
  if (mkdir(PATH, 0755) == 0) return true;
  return (errno == EEXIST);
}

bool forgeFileRemove(const char* PATH) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(PATH != NULL, "[FILE SYSTEM] : Cannot call remove with a NULL PATH");
  return (remove(PATH) == 0);
}

bool forgeFileRename(const char* OLD_PATH, const char* NEW_PATH) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(OLD_PATH != NULL, "[FILE SYSTEM] : Cannot call rename with a NULL OLD_PATH");
  FORGE_ASSERT_DEBUG_MESSAGE(NEW_PATH != NULL, "[FILE SYSTEM] : Cannot call rename with a NULL NEW_PATH");
  return (rename(OLD_PATH, NEW_PATH) == 0);
}

bool forgeListDir(
  const char*           PATH, 
  ForgeDirIterCallback  CALLBACK, 
  void*                 USER_DATA) 
{
  FORGE_ASSERT_DEBUG_MESSAGE(PATH != NULL, "[FILE SYSTEM] : Cannot list a NULL directory PATH");
  FORGE_ASSERT_DEBUG_MESSAGE(CALLBACK != NULL, "[FILE SYSTEM] : Cannot list with a NULL CALLBACK");

  DIR* dir = opendir(PATH);
  if (!dir) 
  {
    FORGE_LOG_ERROR("[FILE SYSTEM] : Cannot open directory '%s': %s", PATH, strerror(errno));
    return false;
  }

  struct dirent* entry = NULL;
  while ((entry = readdir(dir)) != NULL) 
  {
    // - - - Skip . and ..
    if (
      strcmp(entry->d_name, ".") == 0 || 
      strcmp(entry->d_name, "..") == 0) 
    { continue; }

    bool isDir = false;
    char fullPath[4096];
    forgePathJoin(PATH, entry->d_name, fullPath, sizeof(fullPath));
    isDir = forgeIsDirectory(fullPath);

    CALLBACK(entry->d_name, isDir, USER_DATA);
  }

  closedir(dir);
  return true;
}
