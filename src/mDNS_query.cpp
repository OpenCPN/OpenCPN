/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement mDNS Query, and friends.
 * Author:   David Register, Alec Leamas
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David Register, Alec Leamas                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include <algorithm>
#include <memory>
#include <thread>

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS 1
#endif

#include <stdio.h>

#include <errno.h>
#include <signal.h>

#ifdef _WIN32
#include <winsock2.h>
#include <iphlpapi.h>
#define sleep(x) Sleep(x * 1000)
#else
#include <netdb.h>
#include <ifaddrs.h>
#include <net/if.h>
#endif

#include <wx/datetime.h>

#include "mdns_util.h"
#include "mDNS_query.h"

extern bool g_bportable;

// Static data structs
std::vector<std::shared_ptr<ocpn_DNS_record_t>> g_DNS_cache;
wxDateTime g_DNS_cache_time;


static char addrbuffer[64];
static char entrybuffer[256];
static char namebuffer[256];
static char sendbuffer[1024];
static mdns_record_txt_t txtbuffer[128];

static struct sockaddr_in service_address_ipv4;
static struct sockaddr_in6 service_address_ipv6;

static int has_ipv4;
static int has_ipv6;


static int
ocpn_query_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
               uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
               size_t size, size_t name_offset, size_t name_length, size_t record_offset,
               size_t record_length, void* user_data) {
	(void)sizeof(sock);
	(void)sizeof(query_id);
	(void)sizeof(name_length);
	(void)sizeof(user_data);
	mdns_string_t fromaddrstr = ip_address_to_string(addrbuffer, sizeof(addrbuffer), from, addrlen);
	const char* entrytype = (entry == MDNS_ENTRYTYPE_ANSWER) ?
                                "answer" :
                                ((entry == MDNS_ENTRYTYPE_AUTHORITY) ? "authority" : "additional");
	mdns_string_t entrystr =
	    mdns_string_extract(data, size, &name_offset, entrybuffer, sizeof(entrybuffer));
      bool is_ipv4 = from->sa_family == AF_INET;    // Only ipv4 responses are to be used.

	if ((rtype == MDNS_RECORDTYPE_PTR) && is_ipv4) {
		mdns_string_t namestr = mdns_record_parse_ptr(data, size, record_offset, record_length,
		                                              namebuffer, sizeof(namebuffer));
		printf("%.*s : %s %.*s PTR %.*s rclass 0x%x ttl %u length %d\n",
		       MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr),
		       MDNS_STRING_FORMAT(namestr), rclass, ttl, (int)record_length);

    std::string srv(namestr.str, namestr.length);
    size_t rh = srv.find("opencpn-object");
    if(rh > 1) rh--;
    std::string hostname = srv.substr(0, rh);

    std::string from(fromaddrstr.str, fromaddrstr.length);
    size_t r = from.find(':');
    std::string ip = from.substr(0, r);

    //  Search for this record in the cache
    auto func = [srv](const std::shared_ptr<ocpn_DNS_record_t> record) {
      return !record->service_instance.compare(srv);
      };
    auto found = std::find_if(g_DNS_cache.begin(), g_DNS_cache.end(), func);

    std::shared_ptr<ocpn_DNS_record_t>entry;

    if (found == g_DNS_cache.end()){
      // Add a record
      entry = std::make_shared<ocpn_DNS_record_t>();
      g_DNS_cache.push_back(entry);
    }
    else
      entry = *found;

    // Update the cache entry
    entry->service_instance = srv;
    entry->hostname = hostname;
    entry->ip = ip;
    entry->port = "8000";
    // Is the destination a portable?  Detect by string inspection.
    std::string p ("Portable");
    std::size_t port = hostname.find(p);;
    if (port != std::string::npos)
      entry->port = "8001";
  }


#if 0

	else if (rtype == MDNS_RECORDTYPE_SRV) {
		mdns_record_srv_t srv = mdns_record_parse_srv(data, size, record_offset, record_length,
		                                              namebuffer, sizeof(namebuffer));
		printf("%.*s : %s %.*s SRV %.*s priority %d weight %d port %d\n",
		       MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr),
		       MDNS_STRING_FORMAT(srv.name), srv.priority, srv.weight, srv.port);
	} else if (rtype == MDNS_RECORDTYPE_A) {
		struct sockaddr_in addr;
		mdns_record_parse_a(data, size, record_offset, record_length, &addr);
		mdns_string_t addrstr =
		    ipv4_address_to_string(namebuffer, sizeof(namebuffer), &addr, sizeof(addr));
		printf("%.*s : %s %.*s A %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
		       MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(addrstr));
	} else if (rtype == MDNS_RECORDTYPE_AAAA) {
		struct sockaddr_in6 addr;
		mdns_record_parse_aaaa(data, size, record_offset, record_length, &addr);
		mdns_string_t addrstr =
		    ipv6_address_to_string(namebuffer, sizeof(namebuffer), &addr, sizeof(addr));
		printf("%.*s : %s %.*s AAAA %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
		       MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(addrstr));
	} else if (rtype == MDNS_RECORDTYPE_TXT) {
		size_t parsed = mdns_record_parse_txt(data, size, record_offset, record_length, txtbuffer,
		                                      sizeof(txtbuffer) / sizeof(mdns_record_txt_t));
		for (size_t itxt = 0; itxt < parsed; ++itxt) {
			if (txtbuffer[itxt].value.length) {
				printf("%.*s : %s %.*s TXT %.*s = %.*s\n", MDNS_STRING_FORMAT(fromaddrstr),
				       entrytype, MDNS_STRING_FORMAT(entrystr),
				       MDNS_STRING_FORMAT(txtbuffer[itxt].key),
				       MDNS_STRING_FORMAT(txtbuffer[itxt].value));
			} else {
				printf("%.*s : %s %.*s TXT %.*s\n", MDNS_STRING_FORMAT(fromaddrstr), entrytype,
				       MDNS_STRING_FORMAT(entrystr), MDNS_STRING_FORMAT(txtbuffer[itxt].key));
			}
		}
	} else {
		printf("%.*s : %s %.*s type %u rclass 0x%x ttl %u length %d\n",
		       MDNS_STRING_FORMAT(fromaddrstr), entrytype, MDNS_STRING_FORMAT(entrystr), rtype,
		       rclass, ttl, (int)record_length);
	}
#endif
	return 0;
}




// Send a mDNS query
int
send_mdns_query(mdns_query_t* query, size_t count, size_t timeout_secs) {
	int sockets[32];
	int query_id[32];
	int num_sockets = open_client_sockets(sockets, sizeof(sockets) / sizeof(sockets[0]), 0);
	if (num_sockets <= 0) {
		printf("Failed to open any client sockets\n");
		return -1;
	}
	printf("Opened %d socket%s for mDNS query\n", num_sockets, num_sockets ? "s" : "");

	size_t capacity = 2048;
	void* buffer = malloc(capacity);
	void* user_data = 0;

	printf("Sending mDNS query");
	for (size_t iq = 0; iq < count; ++iq) {
		const char* record_name = "PTR";
		if (query[iq].type == MDNS_RECORDTYPE_SRV)
			record_name = "SRV";
		else if (query[iq].type == MDNS_RECORDTYPE_A)
			record_name = "A";
		else if (query[iq].type == MDNS_RECORDTYPE_AAAA)
			record_name = "AAAA";
		else
			query[iq].type = MDNS_RECORDTYPE_PTR;
		printf(" : %s %s", query[iq].name, record_name);
	}
	printf("\n");
	for (int isock = 0; isock < num_sockets; ++isock) {
		query_id[isock] =
		    mdns_multiquery_send(sockets[isock], query, count, buffer, capacity, 0);
		if (query_id[isock] < 0)
			printf("Failed to send mDNS query: %s\n", strerror(errno));
	}

	// This is a simple implementation that loops for timeout_secs or as long as we get replies
	int res;
	printf("Reading mDNS query replies\n");
	int records = 0;
	do {
		struct timeval timeout;
		timeout.tv_sec = timeout_secs;
		timeout.tv_usec = 0;

		int nfds = 0;
		fd_set readfs;
		FD_ZERO(&readfs);
		for (int isock = 0; isock < num_sockets; ++isock) {
			if (sockets[isock] >= nfds)
				nfds = sockets[isock] + 1;
			FD_SET(sockets[isock], &readfs);
		}

		res = select(nfds, &readfs, 0, 0, &timeout);
		if (res > 0) {
			for (int isock = 0; isock < num_sockets; ++isock) {
				if (FD_ISSET(sockets[isock], &readfs)) {
					int rec = mdns_query_recv(sockets[isock], buffer, capacity, ocpn_query_callback,
					                          user_data, query_id[isock]);
					if (rec > 0)
						records += rec;
				}
				FD_SET(sockets[isock], &readfs);
			}
		}
	} while (res > 0);

	printf("Read %d records\n", records);

	free(buffer);

	for (int isock = 0; isock < num_sockets; ++isock)
		mdns_socket_close(sockets[isock]);
	printf("Closed socket%s\n", num_sockets ? "s" : "");

	return 0;
}


// Static query definition,
// be careful with thread sync if multiple querries used simultaneously
mdns_query_t s_query;

void FindAllOCPNServers(size_t timeout_secs) {
  s_query.name = "opencpn-object-control-service";
  s_query.type = MDNS_RECORDTYPE_PTR;
  s_query.length = strlen(s_query.name);

  std::thread{ send_mdns_query, &s_query, 1, timeout_secs}.detach();
  //send_mdns_query(&query, 1, timeout_secs);
}

std::vector<std::string> get_local_ipv4_addresses() {
  std::vector<std::string> ret_vec;

	// When sending, each socket can only send to one network interface
	// Thus we need to open one socket for each interface and address family
	int num_sockets = 0;
#ifndef ANDROID

#ifdef _WIN32

	IP_ADAPTER_ADDRESSES* adapter_address = 0;
	ULONG address_size = 8000;
	unsigned int ret;
	unsigned int num_retries = 4;
	do {
		adapter_address = (IP_ADAPTER_ADDRESSES*)malloc(address_size);
		ret = GetAdaptersAddresses(AF_UNSPEC, GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_ANYCAST, 0,
		                           adapter_address, &address_size);
		if (ret == ERROR_BUFFER_OVERFLOW) {
			free(adapter_address);
			adapter_address = 0;
			address_size *= 2;
		} else {
			break;
		}
	} while (num_retries-- > 0);

	if (!adapter_address || (ret != NO_ERROR)) {
		free(adapter_address);
		printf("Failed to get network adapter addresses\n");
		return ret_vec;
	}

	int first_ipv4 = 1;
	int first_ipv6 = 1;
	for (PIP_ADAPTER_ADDRESSES adapter = adapter_address; adapter; adapter = adapter->Next) {
		if (adapter->TunnelType == TUNNEL_TYPE_TEREDO)
			continue;
		if (adapter->OperStatus != IfOperStatusUp)
			continue;

		for (IP_ADAPTER_UNICAST_ADDRESS* unicast = adapter->FirstUnicastAddress; unicast;
		     unicast = unicast->Next) {
			if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
				struct sockaddr_in* saddr = (struct sockaddr_in*)unicast->Address.lpSockaddr;
				if ((saddr->sin_addr.S_un.S_un_b.s_b1 != 127) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b2 != 0) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b3 != 0) ||
				    (saddr->sin_addr.S_un.S_un_b.s_b4 != 1)) {
					int log_addr = 0;
					if (first_ipv4) {
						service_address_ipv4 = *saddr;
						first_ipv4 = 0;
						log_addr = 1;
					}
					has_ipv4 = 1;


					char buffer[128];
					mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,
						                                            sizeof(struct sockaddr_in));
          std::string addr_string(addr.str, addr.length);
          ret_vec.push_back(addr_string);

          }
				}
			}
  }
#if 0
			else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
				struct sockaddr_in6* saddr = (struct sockaddr_in6*)unicast->Address.lpSockaddr;
				// Ignore link-local addresses
				if (saddr->sin6_scope_id)
					continue;
				static const unsigned char localhost[] = {0, 0, 0, 0, 0, 0, 0, 0,
				                                          0, 0, 0, 0, 0, 0, 0, 1};
				static const unsigned char localhost_mapped[] = {0, 0, 0,    0,    0,    0, 0, 0,
				                                                 0, 0, 0xff, 0xff, 0x7f, 0, 0, 1};
				if ((unicast->DadState == NldsPreferred) &&
				    memcmp(saddr->sin6_addr.s6_addr, localhost, 16) &&
				    memcmp(saddr->sin6_addr.s6_addr, localhost_mapped, 16)) {
					int log_addr = 0;
					if (first_ipv6) {
						service_address_ipv6 = *saddr;
						first_ipv6 = 0;
						log_addr = 1;
					}
					has_ipv6 = 1;
					if (num_sockets < max_sockets) {
						saddr->sin6_port = htons((unsigned short)port);
						int sock = mdns_socket_open_ipv6(saddr);
						if (sock >= 0) {
							sockets[num_sockets++] = sock;
							log_addr = 1;
						} else {
							log_addr = 0;
						}
					}
					if (log_addr) {
						char buffer[128];
						mdns_string_t addr = ipv6_address_to_string(buffer, sizeof(buffer), saddr,
						                                            sizeof(struct sockaddr_in6));
						printf("Local IPv6 address: %.*s\n", MDNS_STRING_FORMAT(addr));
					}
				}
			}
		}
	}

#endif
	free(adapter_address);

#else

	struct ifaddrs* ifaddr = 0;
	struct ifaddrs* ifa = 0;

	if (getifaddrs(&ifaddr) < 0)
		printf("Unable to get interface addresses\n");

	int first_ipv4 = 1;
	int first_ipv6 = 1;
	for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
		if (!ifa->ifa_addr)
			continue;
		if (!(ifa->ifa_flags & IFF_UP) || !(ifa->ifa_flags & IFF_MULTICAST))
			continue;
		if ((ifa->ifa_flags & IFF_LOOPBACK) || (ifa->ifa_flags & IFF_POINTOPOINT))
			continue;

		if (ifa->ifa_addr->sa_family == AF_INET) {
			struct sockaddr_in* saddr = (struct sockaddr_in*)ifa->ifa_addr;
			if (saddr->sin_addr.s_addr != htonl(INADDR_LOOPBACK)) {
				int log_addr = 0;
				if (first_ipv4) {
					service_address_ipv4 = *saddr;
					first_ipv4 = 0;
					log_addr = 1;
				}
				has_ipv4 = 1;

				char buffer[128];
				mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,
					                                            sizeof(struct sockaddr_in));
        std::string addr_string(addr.str, addr.length);
        ret_vec.push_back(addr_string);
#if 0
				if (num_sockets < max_sockets) {
					saddr->sin_port = htons(port);
					int sock = mdns_socket_open_ipv4(saddr);
					if (sock >= 0) {
						sockets[num_sockets++] = sock;
						log_addr = 1;
					} else {
						log_addr = 0;
					}
				}
				if (log_addr) {
					char buffer[128];
					mdns_string_t addr = ipv4_address_to_string(buffer, sizeof(buffer), saddr,
					                                            sizeof(struct sockaddr_in));
					printf("Local IPv4 address: %.*s\n", MDNS_STRING_FORMAT(addr));
				}
#endif
			}
		}
#if 0
		else if (ifa->ifa_addr->sa_family == AF_INET6) {
			struct sockaddr_in6* saddr = (struct sockaddr_in6*)ifa->ifa_addr;
			// Ignore link-local addresses
			if (saddr->sin6_scope_id)
				continue;
			static const unsigned char localhost[] = {0, 0, 0, 0, 0, 0, 0, 0,
			                                          0, 0, 0, 0, 0, 0, 0, 1};
			static const unsigned char localhost_mapped[] = {0, 0, 0,    0,    0,    0, 0, 0,
			                                                 0, 0, 0xff, 0xff, 0x7f, 0, 0, 1};
			if (memcmp(saddr->sin6_addr.s6_addr, localhost, 16) &&
			    memcmp(saddr->sin6_addr.s6_addr, localhost_mapped, 16)) {
				int log_addr = 0;
				if (first_ipv6) {
					service_address_ipv6 = *saddr;
					first_ipv6 = 0;
					log_addr = 1;
				}
				has_ipv6 = 1;
				if (num_sockets < max_sockets) {
					saddr->sin6_port = htons(port);
					int sock = mdns_socket_open_ipv6(saddr);
					if (sock >= 0) {
						sockets[num_sockets++] = sock;
						log_addr = 1;
					} else {
						log_addr = 0;
					}
				}
				if (log_addr) {
					char buffer[128];
					mdns_string_t addr = ipv6_address_to_string(buffer, sizeof(buffer), saddr,
					                                            sizeof(struct sockaddr_in6));
					printf("Local IPv6 address: %.*s\n", MDNS_STRING_FORMAT(addr));
				}
			}
		}
#endif
	}

	freeifaddrs(ifaddr);

#endif

#endif //ANDROID

	return ret_vec;
}


