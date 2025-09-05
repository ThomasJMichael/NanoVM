CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SRC_DIR = src
OBJ_DIR = obj

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

TARGET = nanovm

# Test setup
TEST_DIR = test
UNITY_DIR = $(TEST_DIR)/unity
TEST_SOURCES := $(wildcard $(TEST_DIR)/*.c)
TEST_RUNNERS := $(patsubst $(TEST_DIR)/%.c,$(TEST_DIR)/%.runner,$(TEST_SOURCES))
UNITY_SOURCES := $(UNITY_DIR)/unity.c
TEST_OBJECTS := $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.test.o,$(TEST_SOURCES))
UNITY_OBJECT := $(OBJ_DIR)/unity.o
LIB_OBJECTS = $(filter-out $(OBJ_DIR)/NanoVM.o, $(OBJECTS))

# Trace build
TRACE_CFLAGS = -DLOG_LEVEL=LOG_TRACE -DLOG_USE_COLOR -g

# Debug build
DEBUG_CFLAGS = -DLOG_LEVEL=LOG_DEBUG -DLOG_USE_COLOR -g

# Release build
RELEASE_CFLAGS = -DLOG_LEVEL=LOG_WARN -O3 -DNDEBUG

.PHONY: all trace debug release clean test

all: debug

trace: CFLAGS += $(TRACE_CFLAGS)
trace: $(TARGET)

debug: CFLAGS += $(DEBUG_CFLAGS)
debug: $(TARGET)

release: CFLAGS += $(RELEASE_CFLAGS)
release: $(TARGET)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(TARGET)

# Unity object
$(OBJ_DIR)/unity.o: $(UNITY_SOURCES)
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Test objects
$(OBJ_DIR)/%.test.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) -c $< -o $@

$(TEST_DIR)/%.runner: $(TEST_DIR)/%.c $(UNITY_OBJECT) $(LIB_OBJECTS)
	$(CC) $(CFLAGS) -I$(UNITY_DIR) $< $(UNITY_OBJECT) $(LIB_OBJECTS) -o $@

# Run all test runners
test: CFLAGS += $(DEBUG_CFLAGS) -Isrc -Iinclude
test: $(TEST_RUNNERS)
	@for runner in $(TEST_RUNNERS); do ./$$runner; done

clean:
	rm -rf $(OBJ_DIR) $(TARGET) $(TEST_RUNNERS)
