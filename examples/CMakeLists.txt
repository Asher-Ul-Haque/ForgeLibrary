if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(forgeUtils PUBLIC DEBUG=1 FORGE_ENABLE_MEMORY_TRACKING=1)
endif()

# Add logger example executable
add_executable(loggerExample loggerExample.c)
target_link_libraries(loggerExample PRIVATE forgeUtils)
set_target_properties(loggerExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add assert example executable
add_executable(assertExample assertExample.c)
target_link_libraries(assertExample PRIVATE forgeUtils)
set_target_properties(assertExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Linear alloc example executable
add_executable(linearAllocExample linearAllocExample.c)
target_link_libraries(linearAllocExample PRIVATE forgeUtils)
set_target_properties(linearAllocExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Object Pool example executable
add_executable(objectPoolExample objectPoolExample.c)
target_link_libraries(objectPoolExample PRIVATE forgeUtils)
set_target_properties(objectPoolExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Tracker example executable
add_executable(trackerExample trackerExample.c)
target_link_libraries(trackerExample PRIVATE forgeUtils)
set_target_properties(trackerExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Dynamic Array example executable
add_executable(dynamicArrayExample dynamicArrayExample.c)
target_link_libraries(dynamicArrayExample PRIVATE forgeUtils)
set_target_properties(dynamicArrayExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Hash map example executable
add_executable(hashmapExample hashmapExample.c)
target_link_libraries(hashmapExample PRIVATE forgeUtils)
set_target_properties(hashmapExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Ordered Set example executable
add_executable(orderedSetExample orderedSetExample.c)
target_link_libraries(orderedSetExample PRIVATE forgeUtils)
set_target_properties(orderedSetExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Queue example executable
add_executable(queueExample queueExample.c)
target_link_libraries(queueExample PRIVATE forgeUtils)
set_target_properties(queueExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Ring buffer example executable
add_executable(ringBufferExample ringBufferExample.c)
target_link_libraries(ringBufferExample PRIVATE forgeUtils)
set_target_properties(ringBufferExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Stack example executable
add_executable(stackExample stackExample.c)
target_link_libraries(stackExample PRIVATE forgeUtils)
set_target_properties(stackExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Filesystem example executable
add_executable(filesystemExample filesystemExample.c)
target_link_libraries(filesystemExample PRIVATE forgeUtils)
set_target_properties(filesystemExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

# Add Threadpool example executable
add_executable(threadPoolExample threadPoolExample.c)
target_link_libraries(threadPoolExample PRIVATE forgeUtils)
set_target_properties(threadPoolExample PROPERTIES    C_STANDARD 99    C_STANDARD_REQUIRED ON)

add_executable(dynamicArrayBenchmark dynamicArrayBenchmark.cpp)
target_link_libraries(dynamicArrayBenchmark PRIVATE forgeUtils)
set_target_properties(dynamicArrayBenchmark PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

add_executable(stackBenchmark stackBenchmark.cpp)
target_link_libraries(stackBenchmark PRIVATE forgeUtils)
set_target_properties(stackBenchmark PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

add_executable(queueBenchmark queueBenchmark.cpp)
target_link_libraries(queueBenchmark PRIVATE forgeUtils)
set_target_properties(queueBenchmark PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

add_executable(orderedSetBenchmark orderedSetBenchmark.cpp)
target_link_libraries(orderedSetBenchmark PRIVATE forgeUtils)
set_target_properties(orderedSetBenchmark PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)

add_executable(hashMapBenchmark hashMapBenchmark.cpp)
target_link_libraries(hashMapBenchmark PRIVATE forgeUtils)
set_target_properties(hashMapBenchmark PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
