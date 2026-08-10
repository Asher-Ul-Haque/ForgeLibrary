cmake_minimum_required(VERSION 3.16)

# - - - Force build artificats into .build/ 
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
  message(FATAL_ERROR "In-source builds are forbidden. Run `cmake -B .build' from the project root.")
endif()

project(forgeUtilsrary VERSION 0.1.0 LANGUAGES C CXX)


# - - - Compiler Standards - - - 

set(CMAKE_C_STANDARD          99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS        OFF)

set(CMAKE_CXX_STANDARD          17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS        OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug Release RelWithDebInfo MinSizeRel" FORCE)
endif()

if (NOT MSVC)
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -O3")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# - - - Build Options 
option(forgeUtils_BUILD_TESTS     "Build test suite" ON)
option(forgeUtils_BUILD_EXAMPLES  "Build example progras" ON)


# - - - Source Files - - - 

set(forgeUtils_CORE_SOURCES
  src/core/logger.c
)


set(forgeUtils_ALL_SOURCES
  ${forgeUtils_CORE_SOURCES}
)


# - - - Target - - - 

add_library(forgeUtils STATIC ${forgeUtils_ALL_SOURCES})

target_include_directories(forgeUtils
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# - - - Definitions 
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(forgeUtils PUBLIC DEBUG=1)
endif()


# - - - Warnings and Errors - - - 

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
  # Strict Clang warning profile
  set(CLANG_STRICT_WARNINGS
    -Wall
    -Wextra
    -Wpedantic
    -Werror
  )

  target_compile_options(forgeUtils PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${CLANG_STRICT_WARNINGS} -Wstrict-prototypes -Wold-style-definition>
    $<$<COMPILE_LANGUAGE:CXX>:${CLANG_STRICT_WARNINGS} -Wold-style-cast -Wnon-virtual-dtor>
  )

elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  # Strict GCC warning profile
  target_compile_options(forgeUtils PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    $<$<COMPILE_LANGUAGE:C>:-Wstrict-prototypes -Wold-style-definition>
    $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast -Wnon-virtual-dtor>
  )

elseif(MSVC)
  # Strict MSVC warning profile
  target_compile_options(forgeUtils PRIVATE /W4 /WX)
endif()

# - - - Subdirectories

if(forgeUtils_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

if(forgeUtils_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
