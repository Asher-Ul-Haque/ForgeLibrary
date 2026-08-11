# Add logger example executable
add_executable(loggerExample loggerExample.c)

# Link against the forgeUtils static library
target_link_libraries(loggerExample PRIVATE forgeUtils)

# Set target properties
set_target_properties(loggerExample PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
)
