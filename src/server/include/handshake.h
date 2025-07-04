#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include <selector.h>

// --- Handshake - WRAPPER FUNCTIONS ---

// Inicializar buffers/parsers para handshake
// Ej: resetear índices, limpiar buffers, etc.
void handshake_on_arrival(unsigned state, struct selector_key *key);

// Leer del socket: VER, NMETHODS, METHODS[]
// Verificar que VER == 0x05
// Buscar si METHODS incluye 0x02 (username/password)
// Guardar método elegido en el estado de la conexión
// Si no soporta, preparar respuesta con método 0xFF y pasar a HANDSHAKE_WRITE
// Si soporta, preparar respuesta con método 0x02 y pasar a HANDSHAKE_WRITE
unsigned int handshake_read(struct selector_key *key);

// Cerrar el parser de handshake y limpiar recursos
void handshake_on_departure(unsigned state, struct selector_key *key);

#endif