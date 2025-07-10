#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include <selector.h>
#include <logger.h>


void handshake_on_arrival(unsigned state, struct selector_key* key);

unsigned int handshake_read(struct selector_key* key);

unsigned int handshake_write(struct selector_key* key);

void handshake_on_departure(unsigned state, struct selector_key* key);

#endif