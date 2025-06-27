#include "main.h"

#include "auth.h"

int main(int argc, char *argv[]) {
  // parse args...
  auth_init();

  // ejemplo de agregar un usuario desde argumentos
  auth_add_user("pepe", "1234");

  // continuar setup de selector, etc.
}
