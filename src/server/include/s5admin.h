#ifndef __S5ADMIN__H__
#define __S5ADMIN__H__

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <selector.h>
#include <config.h>
#include <connection_helper.h>


void s5admin_passive_accept(struct selector_key *key);



#endif