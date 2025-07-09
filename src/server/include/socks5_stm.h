#ifndef SOCKS5_PARSER_H
#define SOCKS5_PARSER_H

#include <stm.h>
#include <logger.h>
/*
Segun RFC 1928:
Cuando el cliente se conecta al servidor, le envía un mensaje de identificación
de versión y selección de metodo de autenticación.

+----+----------+----------+
|VER | NMETHODS | METHODS  |
+----+----------+----------+
| 1  |    1     | 1 to 255 |
+----+----------+----------+


Tenemos que chequear que la versión del cliente sea la 5. (El byte VER debe
valer 0x05) y que alguno de los métodos de autenticación soportados sea el
método 0x02 (Username/Password).

Debemos responder con un mensaje de la forma, especificando el método que
elegimos (en nuestro caso, solo aceptamos el método 0x02):

+----+--------+
|VER | METHOD |
+----+--------+
| 1  |   1    |
+----+--------+

En el caso de que el cliente no soporte el método de autenticación que queremos,
debemos responder con el método 0xFF, y el cliente debe cerrar la conexión.

En el caso de que el cliente soporte el método de autenticación que queremos,
entramos en la subnegociación de autenticación especifica para el metodo de
Username/Password.

Ahora, segun RFC 1929:
El cliente envía un mensaje request de autenticación por Username/Password, de
la forma:

+----+------+----------+------+----------+
|VER | ULEN |  UNAME   | PLEN |  PASSWD  |
+----+------+----------+------+----------+
| 1  |  1   | 1 to 255 |  1   | 1 to 255 |
+----+------+----------+------+----------+

Donde VER es la versión actual de la subnegociación (0x01), ULEN es la longitud
del nombre de usuario, PLEN es la longitud de la contraseña, UNAME es el nombre
de usuario y PASSWD es la contraseña.

Nosotros debemos verificar el usuario y contraseña, y responder con un mensaje
de respuesta, de la forma:

+----+--------+
|VER | STATUS |
+----+--------+
| 1  |   1    |
+----+--------+

Un STATUS de 0x00 significa que la autenticación fue exitosa, y el cliente puede
continuar con la conexión.

Un STATUS distinto a 0x00 significa que la autenticación falló, y el cliente
debe cerrar la conexión.

Ahora, nuevamente según RFC 1928:
El cliente envía un mensaje de solicitud de conexión, de la forma:

+----+-----+-------+------+----------+----------+
|VER | CMD |  RSV  | ATYP | DST.ADDR | DST.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | 0x00  |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+

Donde:
o  VER es la versión del protocolo (0x05)
o  CMD es el comando:
   o  CONNECT (0x01)
   o  BIND (0x02)
   o  UDP ASSOCIATE (0x03)
o  RSV es reservado (0x00)
o  ATYP es el tipo de dirección de destino:
   o  IPv4: (0x01)
   o  DOMAINNAME: (0x03)
   o  IPv6: (0x04)
o  DST.ADDR: dirección de destino
o  DST.PORT: puerto de destino en orden de octetos de red

Luego de evaluar el pedido, debemos responder con un mensaje de respuesta, de
la forma:

+----+-----+-------+------+----------+----------+
|VER | REP |  RSV  | ATYP | BND.ADDR | BND.PORT |
+----+-----+-------+------+----------+----------+
| 1  |  1  | 0x00  |  1   | Variable |    2     |
+----+-----+-------+------+----------+----------+

Donde:
o  VER es la versión del protocolo (0x05)
o  REP es el campo de respuesta:
   o  0x00: éxito
   o  0x01: falla general del servidor SOCKS
   o  0x02: conexión no permitida por las reglas
   o  0x03: red inalcanzable
   o  0x04: host inalcanzable
   o  0x05: conexión rechazada
   o  0x06: TTL expirado
   o  0x07: comando no soportado
   o  0x08: tipo de dirección no soportado
   o  0x09 a 0xFF: no asignado
o  RSV es reservado (0x00)
o  ATYP es el tipo de dirección de destino:
   o  IPv4: (0x01)
   o  DOMAINNAME: (0x03)
   o  IPv6: (0x04)
o  BND.ADDR es la dirección de enlace del servidor
o  BND.PORT es el puerto de enlace del servidor en orden de octetos de red

*/

// Estados de la stm para el protocolo SOCKS5

typedef enum socks5_state {
    SOCKS5_HANDSHAKE,
    SOCKS5_AUTH,
    SOCKS5_CONNECTION_REQ,
    SOCKS5_CONNECTING,
    SOCKS5_COPY,
    SOCKS5_DONE,
    SOCKS5_ERROR,
} socks5_state;

struct state_machine* socks5_stm_init();
void socks5_stm_free(struct state_machine* stm);

#endif