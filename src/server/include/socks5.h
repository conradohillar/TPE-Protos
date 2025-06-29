#ifndef SOCKS5NIO_H
#define SOCKS5NIO_H

#include <sys/socket.h>
#include <selector.h>

void socksv5_passive_accept(struct selector_key *key);


#endif