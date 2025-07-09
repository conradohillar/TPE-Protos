#include <errno.h>
#include <getopt.h>
#include <limits.h> /* LONG_MIN et al */
// #include <stdio.h>  /* for printf */
#include <stdlib.h> /* for exit */
#include <string.h> /* memset */

#include "args.h"
#include "logger.h"

static unsigned short port(const char* s) {
    char* end = 0;
    const long sl = strtol(s, &end, 10);

    if (end == s || '\0' != *end ||
        ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno) || sl < 0 ||
        sl > USHRT_MAX) {
        LOG(ERROR, "Port should in in the range of 1-65536: %s", s);
        exit(1);
        return 1;
    }
    return (unsigned short) sl;
}

static void user(char* s, struct users* user) {
    char* p = strchr(s, ':');
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
    LOG_MSG(INFO, "socks5v version 0.0\n"
                  "ITBA Protocolos de Comunicación 2025/1 -- Grupo 2\n"
                  "AQUI VA LA LICENCIA\n");
}

static void usage(const char* progname) {
    LOG(INFO,
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
        "\n",
        progname);
    exit(1);
}

void parse_args(const int argc, char** argv, struct server_args* args) {
    memset(args, 0,
           sizeof(*args)); // sobre todo para setear en null los punteros de users

    args->socks_addr = "0.0.0.0";
    args->socks_port = 1080;

    args->mng_addr = "127.0.0.1";
    args->mng_port = 8080;

    args->disectors_enabled = true;
    args->users_count = 0;

    int c;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {{0, 0, 0, 0}};

        c = getopt_long(argc, argv, "hl:L:Np:P:u:v", long_options, &option_index);
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
                LOG(ERROR, "Maximun number of command line users reached: %d.", MAX_USERS);
                exit(1);
            } else {
                user(optarg, args->users + args->users_count);
                args->users_count++;
            }
            break;
        case 'v':
            version();
            exit(0);
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
