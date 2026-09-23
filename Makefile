# =========================================================================
# COMPILER AND SECURITY FLAGS CONFIGURATION
# =========================================================================
CC = gcc

# -Wall -Wextra: Enforce strict compiler warnings.
# -g3 -O0: Maximum debug information with zero optimization for accurate 
#          Valgrind memory profiling and GDB analysis.
# -I./include: Directs the preprocessor to the public API headers.
CFLAGS = -Wall -Wextra -g3 -O0 -I./include

# Memory Sanitizer flags for catching Use-After-Free or Buffer Overflows natively.
# Use 'make sanitize' to build with these active.
SAN_FLAGS = -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer

# Detect Operating System to link Windows-specific cryptographic libraries
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lbcrypt
else
    LDFLAGS = 
endif

# =========================================================================
# DIRECTORY MAPPING
# =========================================================================
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
BIN_DIR = bin
TEST_DIR = tests

# Automatically resolve all source files and map them to object files
SRC_FILES = $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC_FILES))

# =========================================================================
# BUILD TARGETS
# =========================================================================
.PHONY: all clean prepare fuzzer test_cw test_dh memory_tests sanitize

all: prepare fuzzer test_cw test_dh memory_tests

prepare:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

# Compile object files (API implementation)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Build Fuzzer Target (Links CW Hash Table and Carter-Wegman Hasher)
fuzzer: prepare $(OBJ_DIR)/hash_table_cw.o $(OBJ_DIR)/carter_wegman.o
	$(CC) $(CFLAGS) $(OBJ_DIR)/hash_table_cw.o $(OBJ_DIR)/carter_wegman.o $(TEST_DIR)/fuzzer_target.c -o $(BIN_DIR)/$@ $(LDFLAGS)

# Build Unit Test for Hash Table with Double Hashing (Baseline)
test_ht_dh: prepare $(OBJ_DIR)/hash_table_dh.o
	$(CC) $(CFLAGS) $(OBJ_DIR)/hash_table_dh.o $(TEST_DIR)/unit/test_hash_table_dh.c -o $(BIN_DIR)/$@

# Build Unit Test for Carter-Wegman
test_cw: prepare $(OBJ_DIR)/carter_wegman.o
	$(CC) $(CFLAGS) $(OBJ_DIR)/carter_wegman.o $(TEST_DIR)/unit/test_carter_wegman.c -o $(BIN_DIR)/$@ $(LDFLAGS)

# Build Unit Test for Hash Table with Carter Wegman
test_ht_dh: prepare $(OBJ_DIR)/hash_table_cw.o
	$(CC) $(CFLAGS) $(OBJ_DIR)/hash_table_cw.o $(TEST_DIR)/unit/test_hash_table_cw.c -o $(BIN_DIR)/$@ $(LDFLAGS)

# Build Memory Harnesses for Valgrind
memory_tests: prepare $(OBJ_DIR)/hash_table_dh.o $(OBJ_DIR)/hash_table_cw.o $(OBJ_DIR)/carter_wegman.o
	$(CC) $(CFLAGS) $(OBJ_DIR)/hash_table_cw.o $(OBJ_DIR)/carter_wegman.o $(TEST_DIR)/memory/hash_table_cw.c -o $(BIN_DIR)/valgrind_cw
	$(CC) $(CFLAGS) $(OBJ_DIR)/hash_table_dh.o $(TEST_DIR)/memory/hash_table_dh.c -o $(BIN_DIR)/valgrind_dh

# Sanitize build for aggressive dynamic vulnerability detection
sanitize: CFLAGS += $(SAN_FLAGS)
sanitize: clean all

clean:
	@rm -rf $(OBJ_DIR) $(BIN_DIR)