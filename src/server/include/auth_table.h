#ifndef AUTH_TABLE_H
#define AUTH_TABLE_H

// Gestión de usuarios y autenticación usuario/contraseña (RFC1929)
// Funciones para validar usuarios, agregar/eliminar usuarios, etc.

#include <stdbool.h>
#include <stddef.h>

#define MAX_USERNAME_LEN 256
#define MAX_PASSWORD_LEN 256

void auth_init();
void auth_destroy();

bool auth_add_user(const char *username, const char *password);
bool auth_remove_user(const char *username);
bool auth_check_credentials(const char *username, const char *password);
size_t auth_list_users(char *response, size_t response_size);

#endif 
