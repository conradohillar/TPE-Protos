# S5Admin - Protocolo de Administración y Monitoreo para Proxy SOCKSv5

## 1. Introducción
Este documento describe el protocolo de texto utilizado para la administración y monitoreo en tiempo real del servidor proxy SOCKSv5. Permite a un cliente administrador conectarse al servidor y ejecutar comandos para consultar métricas, modificar la configuración y gestionar usuarios, sin necesidad de reiniciar el servidor.

## 2. Conexión
- El cliente se conecta al puerto de administración especificado por línea de comandos (`-p <puerto>`).

### 3. Reglas generales
- Cada comando y respuesta se transmite como una línea de texto terminada en `\n`.
- El servidor responde a cada comando con una o más líneas de texto.
- Ante un comando inválido, la respuesta será: `ERROR: invalid command`. **Los comandos son case sensitive**.
- Si la cantidad de argumentos es menor que la esperada, la respuesta será: `ERROR: missing argument`.
- Si la cantidad de argumentos es mayor que la esperada, la respuesta será: `ERROR: too many arguments`.

## 4. Comandos Soportados

### 4.1 Gestión de Usuarios
- `ADD_USER <usuario> <password>`
  - Agrega un nuevo usuario.
  - Respuesta: `OK` o `ERROR: <motivo>`
  - **Posibles errores específicos:**
    - `ERROR: user already exists`
    - `ERROR: invalid username or password`
    - `ERROR: max users reached`
- `REMOVE_USER <usuario>`
  - Elimina un usuario existente.
  - Respuesta: `OK` o `ERROR: <motivo>`
  - **Posibles errores específicos:**
    - `ERROR: user not found`
- `LIST_USERS`
  - Lista todos los usuarios registrados.
  - Respuesta: una línea por usuario, luego `END`.

### 4.2 Métricas y Monitoreo
- `GET_METRICS`
  - Devuelve métricas actuales del servidor (conexiones actuales, conexiones históricas, bytes transferidos y errores).
  - Respuesta: una línea por métrica, luego `END`.
- `GET_ACCESS_REGISTER`
  - Devuelve las entradas del registro de accesos.
  - Respuesta: una línea por entrada, indicando fecha de acceso, nombre de usuario, tipo de registro, IP/puerto de origen y destino y status de SOCKS5. Luego, `END`.

### 4.3 Configuración Dinámica
- `SET_LOGLEVEL <nivel>`
  - Cambia el nivel de logging del servidor. Valores posibles: `DEBUG`, `INFO`, `WARNING`, `ERROR`.
  - Respuesta: `OK` o `ERROR: <motivo>`
  - **Posibles errores específicos:**
    - `ERROR: invalid log level`
- `SET_MAX_CONN <cantidad>`
  - Cambia el máximo de conexiones simultáneas permitidas en el servidor.
  - Si el máximo es menor que la cantidad actual de conexiones, se notifica y no se permiten nuevas conexiones hasta que la cantidad de conexiones activas sea menor al nuevo máximo.
  - Respuesta: `OK` o `ERROR: <motivo>`
  - **Posibles errores específicos:**
    - `ERROR: max connections must be at least 1`
- `SET_BUFF <bytes>`
  - Cambia el tamaño de los buffers de entrada/salida para las conexiones futuras.
  - Respuesta: `OK` o `ERROR: <motivo>`
  - **Posibles errores específicos:**
    - `ERROR: buffer size must be at least 256 bytes`
- `GET_CONFIG`
  - Devuelve la configuración actual del servidor (tamaño del buffer, cantidad de usuarios máximos y nivel de log).
  - Respuesta: una línea por variable de configuración, luego `END`.

### 4.4 Información y Ayuda
- `HELP`
  - Lista los comandos disponibles.
  - Respuesta: una línea por comando, luego `END`.
- `PING`
  - Prueba de conectividad.
  - Respuesta: `PONG`

### 4.5 Cerrar conexión
- `EXIT`
  - Respuesta: `BYE`

## 5. Ejemplo de Sesión
```
> ADD_USER juan 1234
OK
> GET_METRICS
CONNECTED: 5
HISTORICAL: 20
BYTES: 10240
ERRORS: 0
END
> LIST_USERS
admin
user
juan
END
> REMOVE_USER user
OK
> SET_LOGLEVEL DEBUG
OK
> SET_MAX_CONN 100
OK
> SET_BUFF 8192
OK
> GET_CONFIG
LOG_LEVEL: INFO
BUFFER_SIZE: 512B
MAX_CONN: 500
END
> GET_ACCESS_REGISTER
Fecha                     Usuario         Tipo       IP_origen       Pto_org     Destino              Pto_dst     Status    
2025-07-13T18:20:00Z      user1           A          192.168.0.2     54786       www.example.com      443         0         
2025-07-13T18:21:15Z      user2           A          ::1             12345       www.google.com       443         0         
END
> HELP
ADD_USER <usuario> <password>
REMOVE_USER <usuario>
LIST_USERS
GET_METRICS
GET_ACCESS_REGISTER
SET_LOGLEVEL <DEBUG|INFO|WARNING|ERROR>
SET_MAX_CONN <cantidad>
SET_BUFF <bytes>
GET_CONFIG
HELP
PING
EXIT
END
```
