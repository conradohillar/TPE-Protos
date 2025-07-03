#ifndef AUTH_WRAPPER_H
#define AUTH_WRAPPER_H

#include "../include/auth_parser.h"
#include "../include/socks5.h"
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

// --- Autenticación ---

// Leer del socket: VER, ULEN, UNAME, PLEN, PASSWD
// Verificar VER == 0x01
// Extraer usuario y contraseña
// Validar credenciales (llamar a tu función de auth)
// Guardar resultado (éxito/fallo) y pasar a AUTH_WRITE
void auth_read(struct selector_key *key);

// Escribir al socket: VER, STATUS (0x00 éxito, 0x01 error)
// Si éxito, pasar a CONNECTION_REQ_READ
// Si error, cerrar conexión (pasar a ERROR)
void auth_write(struct selector_key *key);

#endif