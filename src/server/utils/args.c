#include <errno.h>
#include <getopt.h>
#include <limits.h> /* LONG_MIN et al */
// #include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <string.h> /* memset */

#include "args.h"
#include "logger.h"
log_level_t log_level_from_string(const char *level);

static unsigned short port(const char *s) {
  char *end = 0;
  const long sl = strtol(s, &end, 10);

  if (end == s || '\0' != *end ||
      ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) || sl < 0 ||
      sl > USHRT_MAX) {
    LOG(ERROR, "Port should in in the range of 1-65536: %s", s);
    exit(1);
    return 1;
  }
  return (unsigned short)sl;
}

static void user(char *s, struct users *user) {
  char *p = strchr(s, ':');
  if (p == NULL) {
    LOG_MSG(ERROR, "Password not found");
    exit(1);
  } else {
    *p = 0;
    p++;
    user->name = s;
    user->pass = p;
  }
}

static void version(void) {
  printf("socks5v version 0.0\n"
                "ITBA Protocolos de Comunicación 2025/1 -- Grupo 2\n"
                "AQUI VA LA LICENCIA\n");
}

static void usage(const char *progname) {
  printf(
      "Usage: %s [OPTION]...\n"
      "\n"
      "   -h               Imprime la ayuda y termina.\n"
      "   -l <SOCKS addr>  Dirección donde servirá el proxy SOCKS.\n"
      "   -L <conf  addr>  Dirección donde servirá el servicio de management.\n"
      "   -p <SOCKS port>  Puerto entrante conexiones SOCKS.\n"
      "   -P <conf port>   Puerto entrante conexiones configuracion\n"
      "   -u <name>:<pass> Usuario y contraseña de usuario que puede usar el "
      "proxy. Hasta 10.\n"
      "   -v               Imprime información sobre la versión versión y "
      "termina.\n"
      "   -e <LOGLEVEL>    Establece el nivel de logueo. Valores: DEBUG, INFO, "
      "WARNING, ERROR.\n"
      "\n",
      progname);
  exit(1);
}

void parse_args(const int argc, char **argv, struct server_args *args) {
  memset(args, 0,
         sizeof(*args)); // sobre todo para setear en null los punteros de users

  args->socks_addr = "0.0.0.0";
  args->socks_port = 1080;

  args->mng_addr = "127.0.0.1";
  args->mng_port = 8080;

  args->disectors_enabled = true;
  args->users_count = 0;
  args->log_level = INFO;

  int c;

  while (true) {
    int option_index = 0;
    static struct option long_options[] = {{0, 0, 0, 0}};

    c = getopt_long(argc, argv, "hl:L:Np:P:u:ve:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'h':
      usage(argv[0]);
      break;
    case 'l':
      args->socks_addr = optarg;
      break;
    case 'L':
      args->mng_addr = optarg;
      break;
    case 'N':
      args->disectors_enabled = false;
      break;
    case 'p':
      args->socks_port = port(optarg);
      break;
    case 'P':
      args->mng_port = port(optarg);
      break;
    case 'u':
      if (args->users_count >= MAX_USERS) {
        LOG(ERROR, "Maximun number of command line users reached: %d.",
            MAX_USERS);
        exit(1);
      } else {
        user(optarg, args->users + args->users_count);
        args->users_count++;
      }
      break;
    case 'v':
      version();
      exit(0);
    case 'e': {
      args->log_level = log_level_from_string(optarg);
      if (args->log_level < 0) {
        exit(1);
      }
      break;
    }
    default:
      LOG(ERROR, "Unknown argument %d.", c);
      exit(1);
    }
  }
  if (optind < argc) {
    LOG_MSG(ERROR, "Argument not accepted: ");
    while (optind < argc) {
      LOG(ERROR, "%s ", argv[optind++]);
    }
    LOG_MSG(ERROR, "\n");
    exit(1);
  }
}

log_level_t log_level_from_string(const char *level) {
  if (strcmp(level, "DEBUG") == 0) {
    return DEBUG;
  } else if (strcmp(level, "INFO") == 0) {
    return INFO;
  } else if (strcmp(level, "WARNING") == 0) {
    return WARNING;
  } else if (strcmp(level, "ERROR") == 0) {
    return ERROR;
  } else {
    LOG(ERROR, "Invalid log level: %s", level);
    return -1;
  }
}
