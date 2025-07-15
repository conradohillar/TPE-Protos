#ifndef ACCESS_REGISTER_H
#define ACCESS_REGISTER_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <logger.h>
#include <defines.h>

#define DEST_MAX_LEN 256
#define REGISTER_MAX_ENTRIES 100
#define IP_MAX_LEN 64
#define STATUS_MAX_LEN 8
#define ISO_DATE_LEN 32   //ISO-8601 (ej: 2025-06-10T19:56:34Z)
#define DEFAULT_REG_TYPE 'A' // Tipo de acceso por defecto

// Estructura para una entrada del registro de accesos
typedef struct {
    char iso_date[ISO_DATE_LEN]; 
    char username[MAX_USERNAME_LEN];
    char register_type; 
    char src_ip[IP_MAX_LEN];
    uint16_t src_port;
    char destination[DEST_MAX_LEN]; // nombre o dirección IP
    uint16_t dest_port;
    int status_code; 
} access_entry_t;

// Estructura para el registro completo (cola circular)
typedef struct {
    access_entry_t entries[REGISTER_MAX_ENTRIES];
    size_t head;  // índice del elemento más antiguo
    size_t tail;  // índice donde se insertará el próximo elemento
    size_t count; // cantidad de elementos actuales
} access_register_t;

access_register_t* access_register_init(void);

void access_register_add_entry(access_register_t* reg, const char* username, const char* src_ip, uint16_t src_port, const char* destination, uint16_t dest_port, int status_code, time_t timestamp);

size_t access_register_print(const access_register_t* reg, char* response, size_t response_size);

#endif // ACCESS_REGISTER_H
