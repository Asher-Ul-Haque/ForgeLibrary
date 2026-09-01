/**
 * @file logger.c 
 * Implementation of logging utilities for forgelib 
 *
 * @see logger.h for usage 
 * @warning This file is internal implementation
 */

#include <forgeUtils/core/logger.h>
#include <forgeUtils/core/asserts.h>
#include <stdarg.h>
#include <stdio.h>
#include <memory.h>
#include <stdlib.h>


// - - - Writing Controls - - - 


/**
 * @brief Writes a log to the console 
 * @param MESSAGE what to log 
 * @param COLOR the severity represented by a color 
 */
static void writeConsole(const char* MESSAGE, LogLevel COLOR)
{
  FILE*       stream    = NULL;
  const char* colorStr  = NULL;

  switch (COLOR)
  {
    case LOG_LEVEL_FATAL    : colorStr = "1;97;41"; stream = stderr; break;
    case LOG_LEVEL_ERROR    : colorStr = "1;31";    stream = stderr; break;
    case LOG_LEVEL_WARNING  : colorStr = "33";      stream = stdout; break;
    case LOG_LEVEL_INFO     : colorStr = "32";      stream = stdout; break;
    case LOG_LEVEL_DEBUG    : colorStr = "36";      stream = stdout; break;
    case LOG_LEVEL_TRACE    : colorStr = "90";      stream = stdout; break;
    default                 : colorStr = "0";       stream = stdout; break;
  }

  #ifdef PRINT_LOG_COLORS    
    fprintf(stream, "\033[%sm%s\033[0m\n", colorStr, MESSAGE);
  #else 
    fprintf(stream, "%s", MESSAGE); // - - -This looks scary
  #endif
  /*Here is how it works:
  \033[     - This is the escape character
  %s        - This is the color code
  m         - This is the end of the color code
  %s        - This is the message
  \033[0m   - This is the end of the color
  This is how you print colored text in the terminal*/
}


// - - - API Controls - - -


/**
 * @brief constructs a output string and log it
 * @param LEVEL the log severity 
 * @param MESSAGE the main message given by the user 
 * @warning this function is internal to forge, do not use directly 
 * @see FORGE_LOG macros for usage
 */
void logOutput(LogLevel LEVEL, const char* MESSAGE, ...)
{
  const char* levelStrings[6]   = {"[FATAL]: ", "[ERROR]: ", "[WARN]: ", "[INFO]: ", "[DEBUG]: ", "[TRACE]: "};
  const int   messageLength     = 32000;
  char        outputMessage     [messageLength];
  memset(outputMessage, 0, sizeof(outputMessage));

  // - - - Add the rest of the arguments
  __builtin_va_list argumentPointer;
  va_start(argumentPointer, MESSAGE);
  vsnprintf(outputMessage, sizeof(outputMessage),
            MESSAGE, argumentPointer);
  va_end(argumentPointer);

  // - - - Prepend with level header
  char finalMessage[32000];
  #if PRINT_LOG_TYPES == 1
    sprintf(finalMessage, "%s\t%s", levelStrings[LEVEL], outputMessage);
  #else 
    sprintf(finalMessage, "%s", outputMessage);
  #endif

  writeConsole(finalMessage, LEVEL);    
}


// - - - | Assert Functions | - - -


void reportAssertionFailure(const char* EXPRESSION, const char* MESSAGE, const char* FILE, const char* FUNCTION, size_t LINE)
{
  logOutput(LOG_LEVEL_FATAL, "ASSERT FAIL :     %s\nMESSAGE     :     %s\nLOCATION    :     file: %s at line: %d in function: %s\n", EXPRESSION, MESSAGE, FILE, LINE, FUNCTION);
  abort();
}

void reportTODO(const char* COMMENT, const char* FILE, const char* FUNCTION, size_t LINE)
{
  logOutput(LOG_LEVEL_ERROR, "Oopsie!, you have a TODO!!!");
  logOutput(LOG_LEVEL_ERROR, "At: %s:%d -> Function: %s", FILE, LINE, FUNCTION);
  if (COMMENT) logOutput(LOG_LEVEL_WARNING, "COMMENT: %s", COMMENT);
  abort();
}
