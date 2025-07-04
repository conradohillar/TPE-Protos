#ifndef CONN_REQ_H
#define CONN_REQ_H

#include <selector.h>

// --- Solicitud de conexión ---

// Inicializa el parser de solicitud de conexión y los campos necesarios en la conexión
void connection_req_on_arrival(unsigned state, struct selector_key *key);

// Leer del socket: VER, CMD, RSV, ATYP, DST.ADDR, DST.PORT
// Verificar VER == 0x05, CMD == 0x01 (CONNECT)
// Parsear dirección y puerto destino
// Intentar conectar al destino (no bloqueante)

// Guardar respuesta para el cliente: VER, REP, RSV, ATYP, BND.ADDR, BND.PORT
// REP: 0x00 (succeeded) o 0x01 (general failure)
// BND.ADDR y BND.PORT: dirección y puerto del proxy (0.0.0.0:0)
unsigned int connection_req_read(struct selector_key *key);

void connection_req_on_departure(unsigned state, struct selector_key *key);

#endif