#ifndef AUTH_H
#define AUTH_H

// Gestión de usuarios y autenticación usuario/contraseña (RFC1929)
// Funciones para validar usuarios, agregar/eliminar usuarios, etc.

#include <stdbool.h>

#define MAX_USERNAME_LEN 256
#define MAX_PASSWORD_LEN 256

void auth_init(void);
void auth_destroy(void);

bool auth_add_user(const char *username, const char *password);
bool auth_remove_user(const char *username);
bool auth_check_credentials(const char *username, const char *password);

#endif // AUTH_H
