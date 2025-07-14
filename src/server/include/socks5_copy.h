#include <selector.h>
#include <socks5.h>
#include <logger.h>
#include <errno.h>
#include <string.h>

void copy_on_arrival(unsigned state, struct selector_key* key);

unsigned int copy_read(struct selector_key* key);

void copy_on_departure(unsigned state, struct selector_key* key);
