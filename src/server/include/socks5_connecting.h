#include <selector.h>
#include <socks5.h>
#include <logger.h>
#include <socks5_responses.h>

unsigned int connecting_on_block_ready(struct selector_key* key);

unsigned int connecting_read(struct selector_key* key);

unsigned int connecting_write(struct selector_key* key);
