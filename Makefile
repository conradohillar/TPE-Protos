# Makefile para el proyecto SOCKSv5 Proxy
# Compila el servidor y el cliente de administración
# Uso: make [all|server|admin_client|clean]
# El parámetro -p <puerto> se debe pasar al ejecutar los binarios, no en la compilación

CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g
SRC_DIR = src
OBJ_DIR = obj
BIN_DIR = bin

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

SERVER_OBJS = $(OBJ_DIR)/main.o $(OBJ_DIR)/auth.o $(OBJ_DIR)/config.o \
	$(OBJ_DIR)/metrics.o $(OBJ_DIR)/logger.o $(OBJ_DIR)/buffer.o \
	$(OBJ_DIR)/netutils.o $(OBJ_DIR)/parser.o $(OBJ_DIR)/parser_utils.o \
	$(OBJ_DIR)/selector.o $(OBJ_DIR)/stm.o

ADMIN_CLIENT_OBJS = $(OBJ_DIR)/admin_client.o $(OBJ_DIR)/netutils.o $(OBJ_DIR)/buffer.o

SERVER_BIN = $(BIN_DIR)/server
ADMIN_CLIENT_BIN = $(BIN_DIR)/admin_client

.PHONY: all server admin_client clean dirs

all: dirs $(SERVER_BIN) $(ADMIN_CLIENT_BIN)

server: dirs $(SERVER_BIN)

admin_client: dirs $(ADMIN_CLIENT_BIN)

dirs:
	@mkdir -p $(OBJ_DIR) $(BIN_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(SERVER_BIN): $(SERVER_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

$(ADMIN_CLIENT_BIN): $(ADMIN_CLIENT_OBJS)
	$(CC) $(CFLAGS) $^ -o $@

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

# Ejemplo de ejecución:
# ./bin/server -p 1080
# ./bin/admin_client -p 9090
