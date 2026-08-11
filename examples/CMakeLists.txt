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
