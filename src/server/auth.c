#include "auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASHMAP_SIZE 1031

typedef struct user_entry {
  char username[MAX_USERNAME_LEN];
  char password[MAX_PASSWORD_LEN];
  struct user_entry *next;
} user_entry_t;

static user_entry_t *hashmap[HASHMAP_SIZE];

static unsigned long hash(const char *str) {
  // djb2 hash
  unsigned long hash = 5381;
  int c;
  while ((c = *str++))
    hash = ((hash << 5) + hash) + c; // hash * 33 + c
  return hash % HASHMAP_SIZE;
}

void auth_init() { memset(hashmap, 0, sizeof(hashmap)); }

void auth_destroy(void) {
  for (int i = 0; i < HASHMAP_SIZE; i++) {
    user_entry_t *curr = hashmap[i];
    while (curr) {
      user_entry_t *next = curr->next;
      free(curr);
      curr = next;
    }
    hashmap[i] = NULL;
  }
}

bool auth_add_user(const char *username, const char *password) {
  unsigned long h = hash(username);
  user_entry_t *curr = hashmap[h];
  while (curr) {
    if (strcmp(curr->username, username) == 0)
      return false; // caso el username ya existe
    curr = curr->next;
  }

  user_entry_t *new_user = malloc(sizeof(user_entry_t));
  if (!new_user)
    return false;

  strncpy(new_user->username, username, MAX_USERNAME_LEN - 1);
  new_user->username[MAX_USERNAME_LEN - 1] = '\0';
  strncpy(new_user->password, password, MAX_PASSWORD_LEN - 1);
  new_user->password[MAX_PASSWORD_LEN - 1] = '\0';

  new_user->next = hashmap[h];
  hashmap[h] = new_user;

  return true;
}

bool auth_remove_user(const char *username) {
  unsigned long h = hash(username);
  user_entry_t **indirect = &hashmap[h];
  while (*indirect) {
    user_entry_t *entry = *indirect;
    if (strcmp(entry->username, username) == 0) {
      *indirect = entry->next;
      free(entry);
      return true;
    }
    indirect = &entry->next;
  }
  return false;
}

bool auth_check_credentials(const char *username, const char *password) {
  unsigned long h = hash(username);
  user_entry_t *curr = hashmap[h];
  while (curr) {
    if (strcmp(curr->username, username) == 0 &&
        strcmp(curr->password, password) == 0)
      return true;
    curr = curr->next;
  }
  return false;
}
