#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include "handshake_parser.h"
#include "socks5_stm.h"
#include "socks5.h"
#include "selector.h" // <-- Agregado para struct selector_key
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>


// --- Handshake - WRAPPER FUNCTIONS ---

// Inicializar buffers/parsers para handshake
// Ej: resetear índices, limpiar buffers, etc.
void handshake_parser_init(struct selector_key *key);

// Leer del socket: VER, NMETHODS, METHODS[]
// Verificar que VER == 0x05
// Buscar si METHODS incluye 0x02 (username/password)
// Guardar método elegido en el estado de la conexión
// Si no soporta, preparar respuesta con método 0xFF y pasar a HANDSHAKE_WRITE
// Si soporta, preparar respuesta con método 0x02 y pasar a HANDSHAKE_WRITE
void handshake_read(struct selector_key *key);

// Escribir al socket: VER, METHOD (0x02 o 0xFF)
// Si METHOD == 0xFF, cerrar conexión (pasar a ERROR)
// Si METHOD == 0x02, pasar a AUTH_READ
void handshake_write(struct selector_key *key);

#endif