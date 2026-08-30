/**
 * @file testManager.h 
 * @brief test Management framework in C
*/

#include <stddef.h>
#include <stdint.h>

/// @brief How many tests in a group are allowed 
#define MAX_TESTS_PER_GROUP 256

/// @brief How many groups are allowed
#define MAX_GROUPS 32

/** 
 * @brief Function pointer type for test functions 
 * @return uint8_t the exit value when a function is executed 
 * @see FORGE_TEST_FAIL
 * @see FORGE_TEST_SKIP
 * @see FORGE_TEST_PASS
*/
typedef uint8_t (*ForgeTestFunc)(void);

/**
 * @brief Registers a test to be executed sequentially.
 * 
 * @param TEST_FUNC Pointer to test function returning FORGE_TEST_PASS, FAIL, or SKIP.
 * @param DESCRIPTION Short human-readable description of what the test verifies.
 * @param GROUP_ID to cluster tests together (0 by default). Tests in the same group run sequentially
*/
void forgeTestRegister(ForgeTestFunc TEST_FUNC, const char* DESCRIPTION, uint8_t GROUP_ID);

/**
 * @brief : Runs all registered tests and prints a formatted diagnostic summary 
 * @return : Returns total number of failed + crashed tests (0 if all passed / skipped)
*/
size_t forgeTestRunAll(void);
