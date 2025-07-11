# S5Admin - Protocolo de Administración y Monitoreo para Proxy SOCKSv5

## 1. Introducción
Este documento describe el protocolo de texto utilizado para la administración y monitoreo en tiempo real del servidor proxy SOCKSv5. Permite a un cliente administrador conectarse al servidor y ejecutar comandos para consultar métricas, modificar la configuración y gestionar usuarios, sin necesidad de reiniciar el servidor.

## 2. Conexión
- El cliente se conecta al puerto de administración especificado por línea de comandos (`-p <puerto>`).

### 2.1 Handshake
- Negociación de versión realizada de la siguiente forma:
  - El server envía un paquete indicando las versiones del protocolo que soporta.
  - El cliente responde con otro paquete especificando la versión que desea utilizar.

### 2.2 Reglas generales
- Cada comando y respuesta se transmite como una línea de texto terminada en `\n`.
- El servidor responde a cada comando con una línea de texto.

## 3. Comandos Soportados

### 3.1 Gestión de Usuarios
- `ADD_USER <usuario> <password>`
  - Agrega un nuevo usuario.
  - Respuesta: `OK` o `ERROR <motivo>`
- `REMOVE_USER <usuario>`
  - Elimina un usuario existente.
  - Respuesta: `OK` o `ERROR <motivo>`
- `LIST_USERS`
  - Lista todos los usuarios registrados.
  - Respuesta: una línea por usuario, luego `END`.

### 3.2 Métricas y Monitoreo
- `GET_METRICS`
  - Devuelve métricas actuales del servidor.
  - Respuesta ejemplo: `CONNECTIONS:5 HISTORICAL:20 BYTES:10240`
- `GET_ACCESS_REGISTER`
  - Devuelve las entradas del registro de accesos.
  - Respuesta: una línea por entrada, indicando nombre de usuario, destino y tiempo de acceso. Luego, `END`.

### 3.3 Configuración Dinámica
- `SET_LOGLEVEL <nivel>`
  - Cambia el nivel de logging del servidor. Valores posibles: `DEBUG`, `INFO`, `WARNING`, `ERROR`.
  - Respuesta: `OK` o `ERROR <motivo>`
- `SET_MAX_CONN <cantidad>`
  - Cambia el máximo de conexiones simultáneas permitidas en el servidor.
  - Si el máximo es menor que la cantidad actual de conexiones, se notifica y no se permiten nuevas conexiones hasta que la cantidad de conexiones activas sea menor al nuevo máximo.
  - Respuesta: `OK` o `ERROR <motivo>`
- `SET_BUFF <bytes>`
  - Cambia el tamaño de los buffers de entrada/salida para todas las conexiones (actuales y futuras).
  - Respuesta: `OK` o `ERROR <motivo>`
- `GET_CONFIG`
  - Devuelve la configuración actual del servidor (tamaño del buffer, cantidad de usuarios registrados, cantidad de usuarios máximos, nivel de log, etc.)
  - Respuesta: una línea por parámetro, luego `END`.

### 3.4 Información y Ayuda
- `HELP`
  - Lista los comandos disponibles.
  - Respuesta: una línea por comando, luego `END`.
- `PING`
  - Prueba de conectividad.
  - Respuesta: `PONG`

### 3.5 Cerrar conexión
- `EXIT`
  - Respuesta: `BYE`

## 4. Ejemplo de Sesión
```
> ADD_USER juan 1234
OK
> GET_METRICS
CONNECTIONS:3 HISTORICAL:10 BYTES:20480
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
loglevel:DEBUG
max_conn:100
buffer_size:8192
port:9090
END
> GET_LOG
2025-06-25 10:00:01 admin login OK
2025-06-25 10:01:12 juan login FAIL
END
> HELP
ADD_USER <usuario> <password>
REMOVE_USER <usuario>
LIST_USERS
GET_METRICS
GET_LOG
SET_LOGLEVEL <DEBUG|INFO|WARNING|ERROR>
SET_MAX_CONN <cantidad>
SET_BUFF <bytes>
GET_CONFIG
HELP
PING
END
```

## 5. Consideraciones
- Los comandos y respuestas no distinguen mayúsculas/minúsculas.
- Los errores se reportan con `ERROR <motivo>`.
- El protocolo puede extenderse agregando nuevos comandos.

