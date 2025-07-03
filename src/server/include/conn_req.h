#ifndef CONN_REQ_H
#define CONN_REQ_H

#include <selector.h>
#include <socks5.h>
#include <conn_req_parser.h>

// --- Solicitud de conexión ---

// Inicializa el parser de solicitud de conexión y los campos necesarios en la conexión
void connection_req_init(struct selector_key *key);

// Leer del socket: VER, CMD, RSV, ATYP, DST.ADDR, DST.PORT
// Verificar VER == 0x05, CMD == 0x01 (CONNECT)
// Parsear dirección y puerto destino
// Intentar conectar al destino (no bloqueante)
// Guardar resultado y pasar a CONNECTION_REQ_WRITE
unsigned int connection_req_read(struct selector_key *key);

// Escribir al socket: VER, REP, RSV, ATYP, BND.ADDR, BND.PORT
// REP: 0x00 éxito, otro valor según error
// Si éxito, pasar a DONE (relay de datos)
// Si error, pasar a ERROR
unsigned int connection_req_write(struct selector_key *key);

#endif