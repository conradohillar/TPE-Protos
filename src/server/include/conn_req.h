#ifndef CONN_REQ_H
#define CONN_REQ_H

#include <selector.h>
#include <logger.h>
#include <socks5.h>
#include <socks5_stm.h>
#include <stdint.h>
#include <defines.h>
#include <conn_req_parser.h>
#include <netutils.h>
#include <pthread.h>
#include <socks5_responses.h>


void connection_req_on_arrival(unsigned state, struct selector_key* key);

unsigned int connection_req_read(struct selector_key* key);

unsigned int connection_req_write(struct selector_key* key);

void connection_req_on_departure(unsigned state, struct selector_key* key);


#endif