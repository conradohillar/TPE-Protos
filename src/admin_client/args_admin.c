#include <errno.h>
#include <getopt.h>
#include <limits.h> /* LONG_MIN et al */
#include <stdbool.h>
#include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <string.h> /* memset */

#include <args_admin.h>
#include <logger.h>
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

static void usage(const char *progname) {
  printf(
      "Usage: %s [OPTION]...\n"
      "\n"
      "   -h                   Imprime la ayuda y termina.\n"
      "   -l <addr>            Dirección donde se escucha el proxy SOCKS (IPv4, IPv6, dominio).\n"
      "   -p <port>            Puerto donde se escucha el proxy SOCKS.\n"
      "   -g <logging level>   Nivel de logging. Valores: DEBUG, INFO, WARNING, ERROR.\n"
      "   -f <file>            Archivo donde se escriben los logs.\n"
      "\n",
      progname);
  exit(1);
}

void parse_args(const int argc, char **argv, admin_args *args) {
  memset(args, 0,
         sizeof(*args)); 

  args->host = DEFAULT_SERVER_HOST;
  args->port = DEFAULT_SERVER_PORT;

  args->log_file = NULL;

  args->log_level = INFO;

  int c;

  while (true) {
    int option_index = 0;
    static struct option long_options[] = {{0, 0, 0, 0}};

    c = getopt_long(argc, argv, "hl:p:g:f:", long_options, &option_index);
    if (c == -1)
      break;

    switch (c) {
    case 'h':
      usage(argv[0]);
      break;
    case 'l':
      args->host = optarg;
      break;
    case 'p':
      args->port = port(optarg);
      break;
    case 'g':
      args->log_level = log_level_from_string(optarg);
      break;
    case 'f':
      args->log_file = optarg;
      break;
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
