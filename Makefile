# - - - Directories
LIBS_DIR 			:= Libraries
SRC_DIR 			:= Source
TESTS_DIR 		:= Tests
BUILD_DIR 		:= build
BIN_DIR 			:= bin
TEST_BIN_DIR 	:= $(BIN_DIR)/tests

# - - - Compilers
CC 				:= clang
CXX 			:= clang++
CFLAGS 		:= -fPIC -Wall -Werror -O3
CXXFLAGS 	:= -fPIC -Wall -Werror -O3
LDFLAGS 	:=
RPATH 		:=

# - - - Colors
GREEN 	:= \033[0;32m
RED 		:= \033[0;31m
YELLOW 	:= \033[1;33m
PURPLE 	:= \033[1;35m
PINK 		:= \033[95m
BLUE 		:= \033[0;34m
RESET 	:= \033[0m

# - - - Root project name
ROOT_NAME := $(notdir $(CURDIR))

# - - - Discover all libraries
LIB_SUBDIRS := $(wildcard $(LIBS_DIR)/*/)
LIB_NAMES 	:= $(notdir $(patsubst %/,%,$(LIB_SUBDIRS)))
LIB_BINS 		:= $(foreach lib,$(LIB_NAMES),$(LIBS_DIR)/$(lib)/bin)
LIB_SO 			:= $(foreach lib,$(LIB_NAMES),-l$(lib))

# - - - Add include paths, lib paths, and RPATH entries
CFLAGS 			+= $(foreach lib,$(LIB_NAMES),-I$(LIBS_DIR)/$(lib)/include)
CXXFLAGS 		+= $(foreach lib,$(LIB_NAMES),-I$(LIBS_DIR)/$(lib)/include)
LDFLAGS 		+= $(foreach lib,$(LIB_BINS),-L$(lib))
RPATH 			+= $(foreach lib,$(LIB_BINS),-Wl,-rpath,'$$ORIGIN/../$(lib)')
RPATH_TEST 	:= $(foreach lib,$(LIB_BINS),-Wl,-rpath,'$$ORIGIN/../../$(lib)')

# - - - Application main (must exist in root)
APP_MAIN := $(wildcard main.c main.cpp)

# - - - Application sources (excluding main)
SRC_FILES_C 	:= $(shell find $(SRC_DIR) -name "*.c")
SRC_FILES_CPP := $(shell find $(SRC_DIR) -name "*.cpp")
OBJ_FILES_C 	:= $(patsubst $(SRC_DIR)/%.c,  $(BUILD_DIR)/%.o, $(SRC_FILES_C))
OBJ_FILES_CPP := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o, $(SRC_FILES_CPP))
OBJ_FILES 		:= $(OBJ_FILES_C) $(OBJ_FILES_CPP)

# - - - Tests
TEST_FILES_C 		:= $(shell find $(TESTS_DIR) -name "*.c")
TEST_FILES_CPP 	:= $(shell find $(TESTS_DIR) -name "*.cpp")

# - - - Final executable
TARGET := $(BIN_DIR)/$(ROOT_NAME)

# - - - Default target
all: banner libs sources tests $(TARGET) finished

banner:
	@clear
	@echo "$(PURPLE) - - - | Building Project $(ROOT_NAME) | - - -$(RESET)"
	@echo ""
	@echo ""

libs:
	@echo "$(YELLOW)- - - Building All Libraries - - -$(RESET)"
	@for dir in $(LIB_SUBDIRS); do \
		$(MAKE) -s -C $$dir || exit 1; \
	done
	@echo "$(GREEN)- - - Finished building all libraries - - -$(RESET)"
	@echo ""
	@echo ""

sourceBanner:
	@echo "$(PINK)- - - Building Source - - -$(RESET)"
	@echo ""
	@echo ""

sources: sourceBanner $(OBJ_FILES)
	@echo "$(GREEN)- - - Finished Building Source - - -$(RESET)"
	@echo ""
	@echo ""

# - - - Application build (main + objs)
$(TARGET): $(OBJ_FILES) $(APP_MAIN)
	@mkdir -p $(BIN_DIR)
	@echo "$(PINK)> Linking application $(TARGET)$(RESET)"
	@$(CXX) $^ -o $@ $(LDFLAGS) $(LIB_SO) $(RPATH) \
		&& echo "$(GREEN)✓ Application built successfully$(RESET)" \
		|| (echo "$(RED)✗ Linking failed$(RESET)"; exit 1)
	@echo "$(GREEN)- - - Finished Building Application - - -$(RESET)"
	@echo ""

# - - - Compile C sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo " |_ Compiling (C) : $<"
	@$(CC) $(CFLAGS) -c $< -o $@ >/dev/null 2>&1 \
		&& echo "    $(GREEN)✓ Success$(RESET)" \
		|| (echo "    $(RED)✗ Error compiling $<$(RESET)"; $(CC) $(CFLAGS) -c $< -o $@; exit 1)

# - - - Compile C++ sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	@echo " |_ Compiling (C++) : $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@ >/dev/null 2>&1 \
		&& echo "    $(GREEN)✓ Success$(RESET)" \
		|| (echo "    $(RED)✗ Error compiling $<$(RESET)"; $(CXX) $(CXXFLAGS) -c $< -o $@; exit 1)

# - - - Tests build target
tests: libs sources
	@echo "$(BLUE)- - - Building Tests - - -$(RESET)"
	@mkdir -p $(TEST_BIN_DIR)
	@for test in $(TEST_FILES_C) $(TEST_FILES_CPP); do \
		name=$$(basename $$test | sed 's/\.[^.]*$$//'); \
		echo " |_ Linking test : $$test"; \
		if echo $$test | grep -q '\.c$$'; then \
			$(CC) $(CFLAGS) $$test $(OBJ_FILES) -o $(TEST_BIN_DIR)/$$name $(LDFLAGS) $(LIB_SO) $(RPATH_TEST) >/dev/null 2>&1 \
				&& echo "    $(GREEN)✓ Built test: $(TEST_BIN_DIR)/$$name$(RESET)" \
				|| (echo "    $(RED)✗ Error building test $$test$(RESET)"; $(CC) $(CFLAGS) $$test $(OBJ_FILES) -o $(TEST_BIN_DIR)/$$name $(LDFLAGS) $(LIB_SO) $(RPATH_TEST); exit 1); \
		else \
			$(CXX) $(CXXFLAGS) $$test $(OBJ_FILES) -o $(TEST_BIN_DIR)/$$name $(LDFLAGS) $(LIB_SO) $(RPATH) >/dev/null 2>&1 \
				&& echo "    $(GREEN)✓ Built test: $(TEST_BIN_DIR)/$$name$(RESET)" \
				|| (echo "    $(RED)✗ Error building test $$test$(RESET)"; $(CXX) $(CXXFLAGS) $$test $(OBJ_FILES) -o $(TEST_BIN_DIR)/$$name $(LDFLAGS) $(LIB_SO) $(RPATH_TEST); exit 1); \
		fi; \
	done
	@echo "$(GREEN)- - - Finished Building Tests - - -$(RESET)"
	@echo ""
	@echo ""

finished:
	@echo ""
	@echo "$(GREEN)- - - Built Project : $(ROOT_NAME) - - -$(RESET)"
	@echo ""

# - - - Clean everything
clean:
	@for dir in $(LIB_SUBDIRS); do \
		$(MAKE) -s -C $$dir clean; \
	done
	rm -rf $(BUILD_DIR) $(BIN_DIR)


# - - - Remake targets (clean + build) - - -

remake: clean all

remake-libs:
	@echo "$(YELLOW)- - - Remaking Libraries - - -$(RESET)"
	@for dir in $(LIB_SUBDIRS); do \
		$(MAKE) -s -C $$dir clean || exit 1; \
		$(MAKE) -s -C $$dir || exit 1; \
	done
	@echo "$(GREEN)- - - Finished Remaking Libraries - - -$(RESET)"

remake-sources:
	@echo "$(PINK)- - - Remaking Sources - - -$(RESET)"
	@rm -rf $(BUILD_DIR)
	$(MAKE) sources
	@echo "$(GREEN)- - - Finished Remaking Sources - - -$(RESET)"

remake-target:
	@echo "$(PURPLE)- - - Remaking Application - - -$(RESET)"
	@rm -f $(TARGET)
	$(MAKE) $(TARGET)
	@echo "$(GREEN)- - - Finished Remaking Application - - -$(RESET)"

remake-tests:
	@echo "$(BLUE)- - - Remaking Tests - - -$(RESET)"
	@rm -rf $(TEST_BIN_DIR)
	$(MAKE) tests
	@echo "$(GREEN)- - - Finished Remaking Tests - - -$(RESET)"



# - - - Run targets - - -

run:
	@clear
	@$(TARGET)

compileRun: libs $(TARGET)
	@clear
	@$(TARGET)


.PHONY: all clean libs banner finished sources tests remake \
        remake-libs remake-sources remake-target remake-tests
