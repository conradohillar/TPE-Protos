#ifndef NETUTILS_H_CTCyWGhkVt1pazNytqIRptmAi5U
#define NETUTILS_H_CTCyWGhkVt1pazNytqIRptmAi5U

#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdbool.h>
#include <buffer.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <logger.h>


#define SOCKADDR_TO_HUMAN_MIN (INET6_ADDRSTRLEN + 5 + 1)
/**
 * Describe de forma humana un sockaddr:
 *
 * @param buff     el buffer de escritura
 * @param buffsize el tamaño del buffer  de escritura
 *
 * @param af    address family
 * @param addr  la dirección en si
 * @param nport puerto en network byte order
 *
 */
const char *
sockaddr_to_human(char *buff, const size_t buffsize,
                  const struct sockaddr *addr);



/**
 * Escribe n bytes de buff en fd de forma bloqueante
 *
 * Retorna 0 si se realizó sin problema y errno si hubo problemas
 */
int
sock_blocking_write(const int fd, buffer *b);


/**
 * copia todo el contenido de source a dest de forma bloqueante.
 *
 * Retorna 0 si se realizó sin problema y errno si hubo problemas
 */
int
sock_blocking_copy(const int source, const int dest);

int set_non_blocking_fd(const int fd);

int connect_to_host(const char * dst_addr, const uint16_t dst_port);

#endif
