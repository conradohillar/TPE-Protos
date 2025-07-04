#ifndef AUTH_H
#define AUTH_H


#include <auth_table.h>
#include <selector.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>

// --- Autenticación ---

// Inicializa el parser de autenticación y los campos necesarios en la conexión
void auth_on_arrival(unsigned state, struct selector_key *key);

// Leer del socket: VER, ULEN, UNAME, PLEN, PASSWD
// Verificar VER == 0x01
// Extraer usuario y contraseña
// Validar credenciales (llamar a tu función de auth)
// Guardar resultado (éxito/fallo) y pasar a AUTH_WRITE
unsigned int auth_read(struct selector_key *key);

// Cerrar el parser de autenticación y limpiar recursos
void auth_on_departure(unsigned state, struct selector_key *key);

#endif