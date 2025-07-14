MODE ?= debug
COMPILER=gcc
ifeq ($(MODE),debug)
COMPILERFLAGS=-Wall -Wextra -pedantic -std=c11 -g3 -pthread -D_POSIX_C_SOURCE=200112L -Wno-unused-parameter -fsanitize=address
LDFLAGS=-fsanitize=address
else ifeq ($(MODE),release)
COMPILERFLAGS=-Wall -Wextra -pedantic -std=c11 -O2 -pthread -D_POSIX_C_SOURCE=200112L -Wno-unused-parameter
LDFLAGS=
else
$(error Unknown MODE "$(MODE)". Use MODE=debug or MODE=release)
endif

