/**
 * @file expect.h 
 * @brief provides test friendly macros for C 
 * The macros are similar to other test frameworks like junit 
*/

#include <stddef.h>
#include <stdint.h>


/// @brief returned when test fails, same as false 
#define FORGE_TEST_FAIL 0

/// @brief returned when test passes, same as true 
#define FORGE_TEST_PASS 1 

/// @brief returned when you want to just skip the test
#define FORGE_TEST_SKIP 2


// - - - Expectation macros for tests - - - 

/**
 * @brief : Verifies two values are equal.
 * @param EXPECTED : The expected value 
 * @param ACTUAL : The real value
*/
#define EXPECT_TO_BE(EXPECTED, ACTUAL)                                                              \
  if ((EXPECTED) != (ACTUAL))                                                                       \
  {                                                                                                 \
    FORGE_LOG_ERROR("  [EXPECT FAILED] %s != %s at %s:%d", #EXPECTED, #ACTUAL, __FILE__, __LINE__); \
    return FORGE_TEST_FAIL;                                                                         \
  }

/**
 * @brief : Verifies two values are not equal.
 * @param NOT_EXPECTED : The expected value 
 * @param ACTUAL : The real value
*/
#define EXPECT_NOT_TO_BE(NOT_EXPECTED, ACTUAL)                                                              \
  if ((NOT_EXPECTED) == (ACTUAL))                                                                           \
  {                                                                                                         \
    FORGE_LOG_ERROR("  [NOT_EXPECT FAILED] %s == %s at %s:%d", #NOT_EXPECTED, #ACTUAL, __FILE__, __LINE__); \
    return FORGE_TEST_FAIL;                                                                                 \
  }

#ifdef __cplusplus
/**
 * @brief : Verifies two std::string values are not equal.
 * @param NOT_EXPECTED : The expected value 
 * @param ACTUAL : The real value
*/
#define EXPECT_STRING_TO_BE(EXPECTED, ACTUAL)                                                               \
  if ((EXPECTED) != (ACTUAL))                                                                               \
  {                                                                                                         \
    FORGE_LOG_ERROR("  [EXPECT STRING FAILED] %s != %s at %s:%d", #EXPECTED, #ACTUAL, __FILE__, __LINE__);  \
    return FORGE_TEST_FAIL;                                                                                 \
  }
#endif

/**
 * @brief : Verifies two std::string values are not equal.
 * @param NOT_EXPECTED : The expected value 
 * @param ACTUAL : The real value
*/
#define EXPECT_C_STRING_TO_BE(EXPECTED, ACTUAL)                                                 \
  const char* _exp = (EXPECTED);                                                                \
  const char* _act = (ACTUAL);                                                                  \
  if (_exp == NULL || _act == NULL || strcmp(_exp, _act) != 0)                                  \
  {                                                                                             \
    FORGE_LOG_ERROR("  [EXPECT FAILED] String mismatch: Expected \"%s\", got \"%s\" at %s:%d",  \
                            _exp ? _exp : "NULL", _act ? _act : "NULL", __FILE__, __LINE__);    \
    return FORGE_TEST_FAIL;                                                                     \
  }

/**
  * @brief Verifies two floating point values are approximately equal.
  *
  * @param EXPECTED Expected value.
  * @param ACTUAL Actual value.
  * @param EPS Allowed tolerance.
*/
#define EXPECT_FLOAT_TO_BE(EXPECTED, ACTUAL, EPS)                                           \
  double _diff = (double)(EXPECTED) - (double)(ACTUAL);                                     \
  if (_diff < 0) _diff = -_diff;                                                            \
  if (_diff > (double)(EPSILON))                                                            \
  {                                                                                         \
    FORGE_LOG_ERROR("  [EXPECT FAILED] Float mismatch: %s vs %s (Diff: %f > %f) at %s:%d",  \
                      #EXPECTED, #ACTUAL, _diff, (double)(EPSILON), __FILE__, __LINE__);    \
    return FORGE_TEST_FAIL;                                                                 \
  }

///@brief Verifies a pointer is null.
#define EXPECT_TO_BE_NULL(PTR)                                                                            \
  if ((PTR) != NULL)                                                                                      \
  {                                                                                                       \
    FORGE_LOG_ERROR("  [EXPECT FAILED] Expected NULL pointer for %s at %s:%d", #PTR, __FILE__, __LINE__); \
    return FORGE_TEST_FAIL;                                                                               \
  }

/// @brief Verifies a pointer is not null.
#define EXPECT_TO_BE_NOT_NULL(PTR)                                                                            \
  if ((PTR) == NULL)                                                                                          \
  {                                                                                                           \
    FORGE_LOG_ERROR("  [EXPECT FAILED] Expected non-NULL pointer for %s at %s:%d", #PTR, __FILE__, __LINE__); \
    return FORGE_TEST_FAIL;                                                                                   \
  }

/// @brief Verifies an expression evaluates to true.
#define EXPECT_TO_BE_TRUE(EXPR)                                                                                 \
  if (!(EXPR))                                                                                                  \
  {                                                                                                             \
    FORGE_LOG_ERROR("  [EXPECT FAILED] Expression evaluated to FALSE: %s at %s:%d", #EXPR, __FILE__, __LINE__); \
    return FORGE_TEST_FAIL;                                                                                     \
  }

/// @brief Verifies an expression evaluates to false.
#define EXPECT_TO_BE_FALSE(EXPR)                                                                                \
  if ((EXPR))                                                                                                   \
  {                                                                                                             \
    FORGE_LOG_ERROR("  [EXPECT FAILED] Expression evaluated to TRUE: %s at %s:%d", #EXPR, __FILE__, __LINE__);  \
    return FORGE_TEST_FAIL;                                                                                     \
  }

