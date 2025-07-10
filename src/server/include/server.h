#ifndef __SERVER__H__
#define __SERVER__H__

// Declaraciones para el entry point del servidor SOCKSv5
// Inicialización, ciclo principal, manejo de argumentos, etc.
#include <args.h>
#include <auth_table.h>
#include <stddef.h>
#include <stdio.h>
#include <selector.h>
#include <signal.h> // Para sigaction, sigemptyset, sigaddset, SIG_BLOCK (senales)
#include <sys/signal.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <socks5.h>
#include <s5admin.h>
#include <server_data.h>
#include <logger.h>
#include <connection_helper.h>

#define FD_STDIN 0
#define INITIAL_QUANTITY_FDS 1024
#define MAX_PENDING_CONNECTIONS 128
#define SEC_TIMEOUT 5
#define NANO_SEC_TIMEOUT 0
#define SOCKS5 "SOCKS5"
#define CONF_PROTOCOL "CONF PROTOCOL"

#endif
