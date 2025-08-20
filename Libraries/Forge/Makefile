# Directories
SRC_DIR := implement
INC_DIR := include
BUILD_DIR := build
LIBRARY_DIR := bin

# Library output
LIB_NAME := libForge.so
LIB_OUTPUT := $(LIBRARY_DIR)/$(LIB_NAME)

# Compiler and flags
CC := clang
CXX := clang++
CFLAGS := -I$(INC_DIR) -fPIC
CXXFLAGS := -I$(INC_DIR) -fPIC
LDFLAGS := -shared

# Colors
GREEN := \033[0;32m
RED := \033[0;31m
YELLOW := \033[1;33m
RESET := \033[0m

# Source files (recursive search)
C_SRC_FILES := $(shell find $(SRC_DIR) -name "*.c")
CPP_SRC_FILES := $(shell find $(SRC_DIR) -name "*.cpp")

# Object files
C_OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRC_FILES))
CPP_OBJ_FILES := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(CPP_SRC_FILES))
OBJ_FILES := $(C_OBJ_FILES) $(CPP_OBJ_FILES)

# Default target
all: banner $(LIB_OUTPUT) success

banner:
	@echo ""
	@echo "$(YELLOW)- - - Building Library : $(LIB_NAME) - - -$(RESET)"
	@echo ""

success:
	@echo ""
	@echo "$(GREEN)- - - Library : $(LIB_NAME) compiled successfully - - -$(RESET)"
	@echo ""

# Create the shared library
$(LIB_OUTPUT): $(OBJ_FILES)
	@mkdir -p $(LIBRARY_DIR)
	@echo "$(YELLOW)> Linking objects into $(LIB_OUTPUT)$(RESET)"
	@$(CXX) $(LDFLAGS) $^ -o $@ \
		&& echo "$(GREEN)  > Linking successful$(RESET)" \
		|| (echo "$(RED)  > Linking failed$(RESET)"; exit 1)

# Compile .c files to .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)> Compiling file : $<$(RESET)"
	@$(CC) $(CFLAGS) -c $< -o $@ >/dev/null 2>&1 \
		&& echo "   $(GREEN)✓ Success$(RESET)" \
		|| (echo "   $(RED)✗ Error compiling $<$(RESET)"; $(CC) $(CFLAGS) -c $< -o $@; exit 1)

# Compile .cpp files to .o files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo "$(YELLOW)> Compiling file : $<$(RESET)"
	@$(CXX) $(CXXFLAGS) -c $< -o $@ >/dev/null 2>&1 \
		&& echo "   $(GREEN)✓ Success$(RESET)" \
		|| (echo "   $(RED)✗ Error compiling $<$(RESET)"; $(CXX) $(CXXFLAGS) -c $< -o $@; exit 1)

# Clean build files
clean:
	rm -rf $(BUILD_DIR) $(LIBRARY_DIR)

.PHONY: all clean banner success
