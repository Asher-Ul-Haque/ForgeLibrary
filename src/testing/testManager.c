#include <core/logger.h>
#include <stdbool.h>
#include <stdint.h>
#include <testing/testManager.h>
#include <testing/expect.h>
#include <core/asserts.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
  #include <unistd.h>
  #include <sys/wait.h>
#endif

typedef struct ForgeTestEntry 
{
  ForgeTestFunc func;
  const char*   description;
} ForgeTestEntry;

typedef struct ForgeTestGroup 
{
  ForgeTestEntry  tests[MAX_TESTS_PER_GROUP];
  size_t          count;
} ForgeTestGroup;

static ForgeTestGroup testGroups[MAX_GROUPS];
static size_t         maxGroupID = 0;

void forgeTestRegister(ForgeTestFunc TEST_FUNC, const char* DESCRIPTION, uint8_t GROUP_ID) 
{
  FORGE_ASSERT_MESSAGE(TEST_FUNC != NULL, "[TEST MANAGER] : Test function cannot be NULL");
  FORGE_ASSERT_MESSAGE(GROUP_ID < MAX_GROUPS, "[TEST MANAGER] : Exceeded maximum group ID limit");

  if (GROUP_ID > maxGroupID) maxGroupID = GROUP_ID;

  ForgeTestGroup* group = &testGroups[GROUP_ID];
  FORGE_ASSERT_MESSAGE(group->count < MAX_TESTS_PER_GROUP, "[TEST MANAGER] Group test capacity exceeded!");

  group->tests[group->count].func         = TEST_FUNC;
  group->tests[group->count].description  = DESCRIPTION;
  group->count++;
}

#if !defined(_WIN32)
static bool runTestForked(ForgeTestEntry TEST, uint8_t* OUT_RESULT) 
{
  int32_t pipefd[2];
  if (pipe(pipefd) != 0) 
  {
    FORGE_LOG_ERROR("[TEST MANAGER] : Failed to create process pipe");
    return false;
  }

  pid_t pid = fork();
  if (pid == 0) 
  {
    // - - - Child Process: Redirect stdout/stderr to pipe and run test
    dup2(pipefd[1], STDOUT_FILENO);
    dup2(pipefd[1], STDERR_FILENO);
    close(pipefd[0]);
    close(pipefd[1]);

    uint8_t res = TEST.func();
    exit((int32_t)res);
  }

  // - - - Parent Process
  close(pipefd[1]);

  char buffer[1024];
  ssize_t bytesRead;

  // - - - Flush child logs to current terminal stream
  while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) 
  {
    buffer[bytesRead] = '\0';
    fputs(buffer, stdout);   
  }
  close(pipefd[0]);

  int32_t status = 0;
  waitpid(pid, &status, 0);

  // - - - Detect signal crashes (SIGSEGV, SIGABRT, etc.)
  if (WIFSIGNALED(status)) return false;

  *OUT_RESULT = (uint8_t)WEXITSTATUS(status);
  return true;
}
#endif

size_t forge_test_run_all(void) 
{
  size_t totalTests   = 0;
  size_t passedTests  = 0;
  size_t skippedTests = 0;
  size_t failedTests  = 0;
  size_t crashedTests = 0;

  FORGE_LOG_INFO("==================================================");
  FORGE_LOG_INFO("               RUNNING UNIT TESTS                ");
  FORGE_LOG_INFO("==================================================");

  for (size_t g = 0; g <= maxGroupID; ++g) 
  {
    ForgeTestGroup* group = &testGroups[g];
    if (group->count == 0) continue;

    FORGE_LOG_INFO("\n--- [TEST GROUP %zu] ---", g);

    for (size_t i = 0; i < group->count; ++i) 
    {
      ForgeTestEntry test = group->tests[i];
      totalTests++;

      uint8_t result = FORGE_TEST_FAIL;
      bool executed_safely = true;

    #if !defined(_WIN32)
      executed_safely = runTestForked(test, &result);
    #else
      result = test.func();
    #endif

      if (!executed_safely) 
      {
        crashedTests++;
        FORGE_LOG_ERROR("  [CRASHED] %s", test.description);
      } 
      else if (result == FORGE_TEST_PASS) 
      {
        passedTests++;
        FORGE_LOG_INFO("  [PASS]    %s", test.description);
      } 
      else if (result == FORGE_TEST_SKIP) 
      {
        skippedTests++;
        FORGE_LOG_WARNING("  [SKIP]    %s", test.description);
      } 
      else 
      {
        failedTests++;
        FORGE_LOG_ERROR("  [FAIL]    %s", test.description);
      }
    }
  }

  FORGE_LOG_INFO("\n==================================================");
  FORGE_LOG_INFO("                  TEST SUMMARY                    ");
  FORGE_LOG_INFO("==================================================");
  FORGE_LOG_INFO(" Total Executed : %zu", totalTests);
  FORGE_LOG_INFO(" Passed         : %zu", passedTests);
  
  if (skippedTests > 0) FORGE_LOG_WARNING(" Skipped        : %zu", skippedTests);
  if (failedTests  > 0) FORGE_LOG_ERROR(" Failed         : %zu", failedTests);
  if (crashedTests > 0) FORGE_LOG_ERROR(" Crashed        : %zu", crashedTests);

  FORGE_LOG_INFO("==================================================\n");

  return (int32_t)(failedTests + crashedTests);
}
