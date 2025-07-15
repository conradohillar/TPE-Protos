# SOCKSv5 Proxy con Interfaz de Administración

Este proyecto implementa un servidor proxy que utiliza el protocolo SOCKS versión 5 (RFC 1928) para intermediar conexiones entre clientes y servidores finales. Además, incluye una interfaz de administración que permite configurar y monitorear el servidor en tiempo real. Se provee un cliente de administración para interactuar con dicha interfaz.

## 🛠️ Compilación

El código fuente incluye un `Makefile` que facilita la compilación del proyecto. Luego de descomprimir el directorio, desde la carpeta raíz se puede compilar utilizando el siguiente comando:

```bash
make <target> [MODE=release]
```

* `<target>` puede ser: `all`, `server` o `client`, dependiendo de los binarios que se desee generar.
* El parámetro `MODE=release` es opcional:

  * Si **se omite**, se compila con herramientas de análisis como `-fsanitize=address`, útiles para debugging.
  * Si **se incluye**, se compila en modo producción (sin sanitizadores).

> ⚠️ Es necesario tener el compilador GCC instalado para compilar el proyecto.

Los binarios generados (`socks5d` y/o `admin_client`) se ubicarán en el directorio `bin/`, en la raíz del proyecto.

---

## ⚙️ Configuración

La configuración se realiza por línea de comandos. No se utilizan archivos de configuración.

### 🧹 Configuración del Servidor

El servidor se configura al momento de su ejecución. Opciones disponibles:

| Opcion           | Descripción                                                                 |
| ---------------- | --------------------------------------------------------------------------- |
| `-h`             | Muestra un mensaje de ayuda y termina la ejecución.                         |
| `-l <addr>`      | Dirección IP en la que el servidor SOCKS escuchará (por defecto `0.0.0.0`). |
| `-L <addr>`      | Dirección IP para el servidor de administración (por defecto `127.0.0.1`).  |
| `-p <port>`      | Puerto para el servicio SOCKSv5 (por defecto `1080`).                       |
| `-P <port>`      | Puerto para la interfaz de administración (por defecto `8080`).             |
| `-u <user:pass>` | Agrega un usuario con contraseña. Se puede repetir hasta 10 veces.          |
| `-v`             | Muestra la versión del servidor y termina la ejecución.                     |
| `-g <nivel>`     | Nivel de log (`DEBUG`, `INFO`, `WARNING`, `ERROR`). Por defecto `INFO`.     |
| `-f <archivo>`   | Archivo de log. Si se omite, los logs se imprimen en la salida estándar.    |

#### 📌 Ejemplo de ejecución del servidor

```bash
./bin/socks5d -p 1080 -P 8080 -u user1:pass1 -u user2:pass2
```

Esto ejecuta el servidor SOCKSv5 en el puerto `1080`, la interfaz de administración en el puerto `8080`, y crea dos usuarios.

---

### 💻 Configuración del Cliente de Administración

El cliente de administración se conecta al servidor para consultar y modificar su estado. Opciones disponibles:

| Opcion         | Descripción                                                              |
| -------------- | ------------------------------------------------------------------------ |
| `-h`           | Muestra un mensaje de ayuda y termina la ejecución.                      |
| `-L <addr>`    | Dirección IP del servidor de administración (por defecto `127.0.0.1`).   |
| `-P <port>`    | Puerto del servidor de administración (por defecto `8080`).              |
| `-v`           | Muestra la versión del cliente.                                          |
| `-g <nivel>`   | Nivel de log (`ERROR`, `WARNING`, `INFO`, `DEBUG`). Por defecto `INFO`.  |
| `-f <archivo>` | Archivo de log. Si se omite, los logs se imprimen en la salida estándar. |

#### 📌 Ejemplo de ejecución del cliente

```bash
./bin/admin_client -L 127.0.0.1 -P 8080
```

Esto conecta el cliente al servidor de administración corriendo en `localhost` y puerto `8080`.

---

## 📂 Estructura del Proyecto

```
.
├── src/                # Código fuente
│   ├── socks5d/        # Código del servidor SOCKSv5
│   ├── admin_client/   # Código del cliente de administración
│   └── common/         # Código compartido (logger, parser, etc.)
├── bin/                # Binarios generados
├── Makefile            # Archivo de compilación
└── README.md           # Este archivo
```

---

## 📋 Requisitos

* GCC
* Make
