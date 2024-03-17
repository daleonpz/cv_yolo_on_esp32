# Makefile

# Compiler
CXX := g++

# Compiler flags
CXXFLAGS := -std=c++17 -I main -I /home/dnl/Documents/git/googletest/googletest/include -DUNIT_TESTING

# Linker flags
LDFLAGS := -pthread

# Source files
SRC_DIR := main
TEST_DIR := unit_tests
BUILD_DIR := build_test

# SRC_FILES := $(wildcard $(SRC_DIR)/*.cc)
# only add model_utils.cc from main
SRC_FILES += $(SRC_DIR)/model_utils.cc
TEST_FILES := $(wildcard $(TEST_DIR)/*.cc)

# Object files
OBJ := $(SRC_FILES:$(SRC_DIR)/%.cc=$(BUILD_DIR)/%.o)
TEST_OBJ := $(TEST_FILES:$(TEST_DIR)/%.cc=$(BUILD_DIR)/%.o)

# Executable
TEST_EXEC := $(BUILD_DIR)/tests

.PHONY: all clean

all: $(TEST_EXEC)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o: $(TEST_DIR)/%.cc
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_EXEC): $(OBJ) $(TEST_OBJ)
	$(CXX) $(OBJ) $(TEST_OBJ) /home/dnl/Documents/git/googletest/build/lib/libgtest.a $(LDFLAGS) -o $@

clean:
	@rm -rf $(BUILD_DIR)


