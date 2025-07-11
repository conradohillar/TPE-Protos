#ifndef AUTH_H
#define AUTH_H

#include <auth_table.h>
#include <selector.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>
#include <defines.h>
#include <logger.h>
#include <socks5_responses.h>


void auth_on_arrival(unsigned state, struct selector_key* key);

unsigned int auth_read(struct selector_key* key);

unsigned int auth_write(struct selector_key* key);

void auth_on_departure(unsigned state, struct selector_key* key);

#endif
