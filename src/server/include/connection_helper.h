#ifndef __CONNECTION_HELPER_H__
#define __CONNECTION_HELPER_H__
#define _GNU_SOURCE

#include <stddef.h>
#include <netutils.h>
#include <sys/socket.h>
#include <unistd.h>
#include <selector.h>

int passive_accept(struct selector_key* key, void* data, const fd_handler* callback_functions);

int create_and_register_passive_socket(fd_selector* selector, char* address, unsigned short port, const fd_handler* callback_functions, selector_status* ss, const char** error_msg, char* protocol);


#endif