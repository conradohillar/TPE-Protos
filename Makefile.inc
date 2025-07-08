COMPILER=gcc
COMPILERFLAGS=-Wall -Wextra -pedantic -std=c11 -g3 -pthread -D_POSIX_C_SOURCE=200112L -fsanitize=address -Wno-unused-parameter
LDFLAGS= -fsanitize=address
