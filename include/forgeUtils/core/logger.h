/**
 * @file logger.h 
 * @brief provides logging utilities to any program, have colored logging output, severity based coloring etc
 * 
 * @warning use stdout and not thread safe, if you want logging in a file, pipe stdout to a file 
 * @see logger.c 
*/

#pragma once 
#ifdef __cplusplus
extern "C" {
#endif


// - - - | Log macros and defs | - - - 


// - - - Enables - - -

/// @brief The DEBUG macro turns thorough debugging on by default
#ifdef DEBUG 
  #define LOG_WARNING_ENABLED 1 
  #define LOG_DEBUG_ENABLED   1
  #define LOG_TRACE_ENABLED   1 
  #define LOG_INFO_ENABLED    1
#endif 

#ifndef LOG_WARNING_ENABLED
  #define LOG_WARNING_ENABLED   1
#endif

#ifndef LOG_INFO_ENABLED
  #define LOG_INFO_ENABLED      1
#endif

#ifndef LOG_DEBUG_ENABLED
  #define LOG_DEBUG_ENABLED     1
#endif

#ifndef LOG_TRACE_ENABLED
  #define LOG_TRACE_ENABLED     1
#endif

#ifndef PRINT_LOG_TYPES
  #define PRINT_LOG_TYPES       1
#endif

#ifndef PRINT_LOG_COLORS
  #define PRINT_LOG_COLORS      1
#endif


// - - - Log Levels - - -

/// @brief Represents log level, internally
typedef enum LogLevel 
{
  LOG_LEVEL_FATAL   =   0,      /// @brief Just give up and die 
  LOG_LEVEL_ERROR   =   1,      /// @brief Something went wrong badly
  LOG_LEVEL_WARNING =   2,      /// @brief Are you sure about that
  LOG_LEVEL_INFO    =   3,      /// @brief Just some information
  LOG_LEVEL_DEBUG   =   4,      /// @brief Debugging information
  LOG_LEVEL_TRACE   =   5       /// @brief Trace every step, be verbose
} LogLevel;


// - - - API Controls - - -

void logOutput(LogLevel LEVEL, const char* MESSAGE, ...); // - - - Multivariate, takes any number of arguments greater than 1


// - - - Fatal log
// - - - Always define FATAL and ERROR logs.
#ifndef FORGE_LOG_FATAL
  #define FORGE_LOG_FATAL(...) logOutput(LOG_LEVEL_FATAL, __VA_ARGS__, "%s", "");
#endif

#ifndef FORGE_LOG_ERROR
  #define FORGE_LOG_ERROR(...) logOutput(LOG_LEVEL_ERROR, __VA_ARGS__, "%s", "");
#endif

// - - - For the rest, define only when enabled, else define to nothingness
#if LOG_WARNING_ENABLED == 1
  #define FORGE_LOG_WARNING(...) logOutput(LOG_LEVEL_WARNING, __VA_ARGS__, "%s", "");
#else
  #define FORGE_LOG_WARNING(...)
#endif

#if LOG_INFO_ENABLED == 1
  #define FORGE_LOG_INFO(...) logOutput(LOG_LEVEL_INFO, __VA_ARGS__, "%s", "");
#else
  #define FORGE_LOG_INFO(...)
#endif

#if LOG_DEBUG_ENABLED == 1
  #define FORGE_LOG_DEBUG(...) logOutput(LOG_LEVEL_DEBUG, __VA_ARGS__, "%s", "");
#else
  #define FORGE_LOG_DEBUG(...)
#endif

#if LOG_TRACE_ENABLED == 1
  #define FORGE_LOG_TRACE(...) logOutput(LOG_LEVEL_TRACE, __VA_ARGS__, "%s", "");
#else
  #define FORGE_LOG_TRACE(...)
#endif
  
#define FORGE_LOG_CLEAR() printf("\033[H\033[J")


#ifdef __cplusplus
}
#endif
