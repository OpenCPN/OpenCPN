/* mdns_util.h  -  mDNS/DNS-SD library  -  Public Domain  -  2017 Mattias Jansson
 *
 * This library provides a cross-platform mDNS and DNS-SD library in C.
 * The implementation is based on RFC 6762 and RFC 6763.
 *
 * The latest source code is always available at
 *
 * https://github.com/mjansson/mdns
 *
 * This library is put in the public domain; you can redistribute it and/or modify it without any
 * restrictions.
 *
 * mdns_util.cpp/h was adapted in part from mdns.c
 */

#ifndef __MDNSUTIL_H__
#define __MDNSUTIL_H__

#include "mdns.h"

extern mdns_string_t
ipv4_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in* addr,
                       size_t addrlen);
extern mdns_string_t
ipv6_address_to_string(char* buffer, size_t capacity, const struct sockaddr_in6* addr,
                       size_t addrlen);
extern mdns_string_t
ip_address_to_string(char* buffer, size_t capacity, const struct sockaddr* addr, size_t addrlen);

extern int
open_client_sockets(int* sockets, int max_sockets, int port);

extern int
open_service_sockets(int* sockets, int max_sockets);

#endif
