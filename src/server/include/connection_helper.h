#ifndef __CONNECTION_HELPER_H__
#define __CONNECTION_HELPER_H__

#include <stddef.h>
#include <selector.h>
#include <sys/socket.h>
 #include <unistd.h>


int passive_accept(struct selector_key *key, void * data, const fd_handler * callback_functions);


#endif