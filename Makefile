include ./Makefile.inc

SRC_DIRS := src/server src/admin_client src/shared
OBJ_DIR := obj
BIN_DIR := bin

# Recursively find all .c files in each source directory
SERVER_SOURCES := $(shell find src/server -name '*.c')
CLIENT_SOURCES := $(shell find src/admin_client -name '*.c')
SHARED_SOURCES := $(shell find src/shared -name '*.c')

# All sources and objects
SOURCES := $(SERVER_SOURCES) $(CLIENT_SOURCES) $(SHARED_SOURCES)
OBJECTS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SOURCES))

SERVER_OBJECTS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SERVER_SOURCES)) $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SHARED_SOURCES))
CLIENT_OBJECTS := $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(CLIENT_SOURCES)) $(patsubst src/%.c,$(OBJ_DIR)/%.o,$(SHARED_SOURCES))

SERVER_OUTPUT_FILE := $(BIN_DIR)/server
CLIENT_OUTPUT_FILE := $(BIN_DIR)/admin_client

all: server client

server: $(SERVER_OUTPUT_FILE)
client: $(CLIENT_OUTPUT_FILE)

$(SERVER_OUTPUT_FILE): $(SERVER_OBJECTS)
	mkdir -p $(BIN_DIR)
	$(COMPILER) $(LDFLAGS) $(SERVER_OBJECTS) -o $@

$(CLIENT_OUTPUT_FILE): $(CLIENT_OBJECTS)
	mkdir -p $(BIN_DIR)
	$(COMPILER) $(LDFLAGS) $(CLIENT_OBJECTS) -o $@

# Pattern rule for all object files
$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(COMPILER) $(COMPILERFLAGS) -I./src/$(firstword $(subst /, ,$(dir $*)))/include -I./src/shared/include -c $< -o $@

clean:
	rm -rf $(BIN_DIR)
	rm -rf $(OBJ_DIR)

.PHONY: all server client clean
