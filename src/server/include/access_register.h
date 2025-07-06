#ifndef ACCESS_REGISTER_H
#define ACCESS_REGISTER_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <logger.h>

#define USERNAME_MAX_LEN 64
#define DEST_MAX_LEN 256
#define REGISTER_MAX_ENTRIES 100

// Estructura para una entrada del registro de accesos
typedef struct {
    char username[USERNAME_MAX_LEN];
    char destination[DEST_MAX_LEN]; // host:puerto o IP:puerto
    time_t timestamp;
} access_entry_t;

// Estructura para el registro completo (cola circular)
typedef struct {
    access_entry_t entries[REGISTER_MAX_ENTRIES];
    size_t head; // índice del elemento más antiguo
    size_t tail; // índice donde se insertará el próximo elemento
    size_t count; // cantidad de elementos actuales
} access_register_t;

// Inicializa el registro de accesos
access_register_t *access_register_init();

// Agrega una nueva entrada al registro (FIFO, borra la más antigua si está lleno)
void access_register_add_entry(access_register_t *reg, const char *username, const char *destination, time_t timestamp);

// Imprime todas las entradas del registro en el stream dado
size_t access_register_print(const access_register_t *reg, char *response, size_t response_size);

#endif // ACCESS_REGISTER_H
