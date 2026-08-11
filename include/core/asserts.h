/**
  * @file asserts.hpp
  * @brief Assertion and TODO helper macros used throughout the utils module.
  *
  * Provides compile-time assertions, runtime assertion checks, and TODO
  * markers that log an error before terminating the program.
*/

#pragma once
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif


// - - - Assert Methods - - -

/**
 * @brief Internal forge library function to report an assert based on where it happened 
 * @param EXPRESSION what was the expression being asserted 
 * @param MESSAGE the message to be logged in case assert failed 
 * @param FILE which file was the expression in 
 * @param FUNCTION which function was the expression in, if any 
 * @param LINE what line was the expression on
 */
void reportAssertionFailure(const char* EXPRESSION, 
                            const char* MESSAGE, 
                            const char* FILE, 
                            const char* FUNCTION, 
                            size_t      LINE);

/**
 * @brief Internal forge library function to report a TODO 
 * @param COMMENT the message to be logged in case TODO was executed
 * @param FILE which file was the TODO in
 * @param FUNCTION which function was the TODO in, if any 
 * @param LINE what line was the TODO on
 */
void reportTODO(const char* COMMENT, 
                const char* FILE, 
                const char* FUNCTION, 
                size_t      LINE);

/**
  * @brief Compile-time assertion macro.
  *
  * Evaluates a condition during compilation and produces a compiler error
  * if the condition is false. Used to enforce invariants that must hold
  * at compile time.
  *
  * @see Usage: 
  ```C
    FORGE_COMPILE_TIME_ASSERT(sizeof(myStruct) < 8);
  ```
*/
#if defined(__cplusplus)
  #define FORGE_COMPILE_TIME_ASSERT(EXPRESSION) static_assert(EXPRESSION, "Compile-time assertion failed: " #EXPRESSION)
#elif defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
  #include <assert.h>
  #define FORGE_COMPILE_TIME_ASSERT(EXPRESSION) static_assert(EXPRESSION, "Compile-time assertion failed: " #EXPRESSION)
#else
  // - - - C99 fallback static assert using array size trick, basically if assert is false, make an array of size -1
  #define FORGE_CONCAT_IMPL(a, b) a##b
  #define FORGE_CONCAT(a, b) FORGE_CONCAT_IMPL(a, b)
  #define FORGE_COMPILE_TIME_ASSERT(EXPRESSION) \
    typedef char FORGE_CONCAT(forge_static_assert_at_line_, __LINE__)[(EXPRESSION) ? 1 : -1]
#endif

/**
  * @brief Runtime assertion check.
  *
  * Evaluates the expression at runtime. If the expression evaluates to
  * `false`, a fatal log message is emitted and the program aborts.
  *
  * @param EXPRESSION Boolean expression that must evaluate to true.
  *
  * @see Usage: 
  ```C 
  FORGE_ASSERT(value < 32 && myStruct.isValid == true);
  ```
*/
#define FORGE_ASSERT(EXPRESSION)                                                                  \
{                                                                                                 \
  if (EXPRESSION){}                                                                               \
  else                                                                                            \
  {                                                                                               \
    reportAssertionFailure(#EXPRESSION, "Assert Fail\t", __FILE__, __func__, __LINE__);           \
  }                                                                                               \
}                               

/**
  * @brief Runtime assertion check with custom message.
  *
  * Same as `RUNTIME_ASSERT` but allows attaching additional diagnostic
  * information to the log output.
  *
  * @param EXPRESSION Boolean expression that must evaluate to true.
  * @param MESSAGE Custom message describing the failure.
  *
  * @see Usage:
  ```C 
  FORGE_ASSERT_MESSAGE(value < 32, "Value must be less than 32");
  ```
*/
#define FORGE_ASSERT_MESSAGE(EXPRESSION, MESSAGE)                                                 \
{                                                                                                 \
  if (EXPRESSION){}                                                                               \
  else                                                                                            \
  {                                                                                               \
    reportAssertionFailure(#EXPRESSION, MESSAGE, __FILE__, __func__, __LINE__);                   \
  }                                                                                               \
}

#ifdef DEBUG

/**
 * @brief TODO, haven't written your code? just write TODO 
 * @warning crashes the program on execution in debug mode, 
 * @warning wont allow the program to compile in release mode, fix todos before you make a release
 *
 * @see DEBUG 
 * @see Usage: 
 ```C
 TODO;
 ```
*/
  #define TODO                                            reportTODO(NULL,    __FILE__, __func__, __LINE__);  

/**
 * @brief TODO, haven't written your code? just write TODO, but with a comment
 *
 * @param COMMENT, the comment to print if execution reaches TODO
 * @warning crashes the program on execution in debug mode, 
 * @warning wont allow the program to compile in release mode, fix todos before you make a release
 *
 * @see DEBUG 
 * @see Usage: 
 ```C
 TODO_COMMENT("Finish the forge library before going too ther projects";
 ```
*/
  #define TODO_COMMENT(COMMENT)                           reportTODO(COMMENT, __FILE__, __func__, __LINE__); 

/**
 * @brief FORGE_ASSERT but in debug mode 
 * @see FORGE_ASSERT 
*/
  #define FORGE_ASSERT_DEBUG(EXPRESSION)                  FORGE_ASSERT(EXPRESSION)

/**
 * @brief FORGE_ASSERT_MESSAGE but in debug mode 
 * @see FORGE_ASSERT_MESSAGE
*/
  #define FORGE_ASSERT_DEBUG_MESSAGE(EXPRESSION, MESSAGE) FORGE_ASSERT_MESSAGE(EXPRESSION, MESSAGE)
#else 
  #define TODO compulsary compile fail here; fix your TODO 
  #define TODO_COMMENT(COMMENT) compulsary compile fail here; fix your TODO
  #define FORGE_ASSERT_DEBUG(EXPRESSION)                        // - - - Does nothing at all
  #define FORGE_ASSERT_DEBUG_MESSAGE(EXPRESSION, MESSAGE) // - - - Does nothing at all
#endif 


#ifdef __cplusplus
}
#endif
