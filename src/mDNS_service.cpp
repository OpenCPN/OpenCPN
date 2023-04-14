/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Implement RESTful server.
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

#include <string>
#include <mutex>
#include <vector>
#include <thread>

#ifdef _WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#endif
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

#include "mdns_util.h"

static char addrbuffer[64];
static char namebuffer[256];
static char sendbuffer[1024];

static struct sockaddr_in service_address_ipv4;
static struct sockaddr_in6 service_address_ipv6;

volatile sig_atomic_t running_server = 1;

// Data for our service including the mDNS records
typedef struct {
	mdns_string_t service;
	mdns_string_t hostname;
	mdns_string_t service_instance;
	mdns_string_t hostname_qualified;
	struct sockaddr_in address_ipv4;
	struct sockaddr_in6 address_ipv6;
	int port;
	mdns_record_t record_ptr;
	mdns_record_t record_srv;
	mdns_record_t record_a;
	mdns_record_t record_aaaa;
	mdns_record_t txt_record[2];
} service_t;




// Callback handling questions incoming on service sockets
int
ocpn_service_callback(int sock, const struct sockaddr* from, size_t addrlen, mdns_entry_type_t entry,
                 uint16_t query_id, uint16_t rtype, uint16_t rclass, uint32_t ttl, const void* data,
                 size_t size, size_t name_offset, size_t name_length, size_t record_offset,
                 size_t record_length, void* user_data) {
	(void)sizeof(ttl);
	if (entry != MDNS_ENTRYTYPE_QUESTION)
		return 0;

	const char dns_sd[] = "_services._dns-sd._udp.local.";
	const service_t* service = (const service_t*)user_data;

	mdns_string_t fromaddrstr = ip_address_to_string(addrbuffer, sizeof(addrbuffer), from, addrlen);

	size_t offset = name_offset;
	mdns_string_t name = mdns_string_extract(data, size, &offset, namebuffer, sizeof(namebuffer));

	const char* record_name = 0;
	if (rtype == MDNS_RECORDTYPE_PTR)
		record_name = "PTR";
	else if (rtype == MDNS_RECORDTYPE_SRV)
		record_name = "SRV";
	else if (rtype == MDNS_RECORDTYPE_A)
		record_name = "A";
	else if (rtype == MDNS_RECORDTYPE_AAAA)
		record_name = "AAAA";
	else if (rtype == MDNS_RECORDTYPE_TXT)
		record_name = "TXT";
	else if (rtype == MDNS_RECORDTYPE_ANY)
		record_name = "ANY";
	else
		return 0;
	printf("Query %s %.*s\n", record_name, MDNS_STRING_FORMAT(name));

	if ((name.length == (sizeof(dns_sd) - 1)) &&
	    (strncmp(name.str, dns_sd, sizeof(dns_sd) - 1) == 0)) {
		if ((rtype == MDNS_RECORDTYPE_PTR) || (rtype == MDNS_RECORDTYPE_ANY)) {
			// The PTR query was for the DNS-SD domain, send answer with a PTR record for the
			// service name we advertise, typically on the "<_service-name>._tcp.local." format

			// Answer PTR record reverse mapping "<_service-name>._tcp.local." to
			// "<hostname>.<_service-name>._tcp.local."
			mdns_record_t answer;
      answer.name =name;
      answer.type = MDNS_RECORDTYPE_PTR;
			answer.data.ptr.name = service->service;

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
			printf("  --> answer %.*s (%s)\n", MDNS_STRING_FORMAT(answer.data.ptr.name),
			       (unicast ? "unicast" : "multicast"));

			if (unicast) {
				mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
				                          query_id, (mdns_record_type_t)rtype, name.str, name.length, answer,
                                  0, 0, 0, 0);
			} else {
				mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0, 0,
				                            0);
			}
		}
	} else if ((name.length == service->service.length) &&
	           (strncmp(name.str, service->service.str, name.length) == 0)) {
		if ((rtype == MDNS_RECORDTYPE_PTR) || (rtype == MDNS_RECORDTYPE_ANY)) {
			// The PTR query was for our service (usually "<_service-name._tcp.local"), answer a PTR
			// record reverse mapping the queried service name to our service instance name
			// (typically on the "<hostname>.<_service-name>._tcp.local." format), and add
			// additional records containing the SRV record mapping the service instance name to our
			// qualified hostname (typically "<hostname>.local.") and port, as well as any IPv4/IPv6
			// address for the hostname as A/AAAA records, and two test TXT records

			// Answer PTR record reverse mapping "<_service-name>._tcp.local." to
			// "<hostname>.<_service-name>._tcp.local."
			mdns_record_t answer = service->record_ptr;

			mdns_record_t additional[5] = {0};
			size_t additional_count = 0;

			// SRV record mapping "<hostname>.<_service-name>._tcp.local." to
			// "<hostname>.local." with port. Set weight & priority to 0.
			additional[additional_count++] = service->record_srv;

			// A/AAAA records mapping "<hostname>.local." to IPv4/IPv6 addresses
			if (service->address_ipv4.sin_family == AF_INET)
				additional[additional_count++] = service->record_a;
			if (service->address_ipv6.sin6_family == AF_INET6)
				additional[additional_count++] = service->record_aaaa;

			// Add two test TXT records for our service instance name, will be coalesced into
			// one record with both key-value pair strings by the library
			//additional[additional_count++] = service->txt_record[0];
			//additional[additional_count++] = service->txt_record[1];

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
			printf("  --> answer %.*s (%s)\n",
			       MDNS_STRING_FORMAT(service->record_ptr.data.ptr.name),
			       (unicast ? "unicast" : "multicast"));

			if (unicast) {
				mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
				                          query_id, (mdns_record_type)rtype, name.str, name.length, answer, 0, 0,
				                          additional, additional_count);
			} else {
				mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0,
				                            additional, additional_count);
			}
		}
	} else if ((name.length == service->service_instance.length) &&
	           (strncmp(name.str, service->service_instance.str, name.length) == 0)) {
		if ((rtype == MDNS_RECORDTYPE_SRV) || (rtype == MDNS_RECORDTYPE_ANY)) {
			// The SRV query was for our service instance (usually
			// "<hostname>.<_service-name._tcp.local"), answer a SRV record mapping the service
			// instance name to our qualified hostname (typically "<hostname>.local.") and port, as
			// well as any IPv4/IPv6 address for the hostname as A/AAAA records, and two test TXT
			// records

			// Answer PTR record reverse mapping "<_service-name>._tcp.local." to
			// "<hostname>.<_service-name>._tcp.local."
			mdns_record_t answer = service->record_srv;

			mdns_record_t additional[5] = {0};
			size_t additional_count = 0;

			// A/AAAA records mapping "<hostname>.local." to IPv4/IPv6 addresses
			if (service->address_ipv4.sin_family == AF_INET)
				additional[additional_count++] = service->record_a;
			if (service->address_ipv6.sin6_family == AF_INET6)
				additional[additional_count++] = service->record_aaaa;

			// Add two test TXT records for our service instance name, will be coalesced into
			// one record with both key-value pair strings by the library
			additional[additional_count++] = service->txt_record[0];
			additional[additional_count++] = service->txt_record[1];

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
			printf("  --> answer %.*s port %d (%s)\n",
			       MDNS_STRING_FORMAT(service->record_srv.data.srv.name), service->port,
			       (unicast ? "unicast" : "multicast"));

			if (unicast) {
				mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
				                          query_id, (mdns_record_type)rtype, name.str, name.length, answer, 0, 0,
				                          additional, additional_count);
			} else {
				mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0,
				                            additional, additional_count);
			}
		}
	} else if ((name.length == service->hostname_qualified.length) &&
	           (strncmp(name.str, service->hostname_qualified.str, name.length) == 0)) {
		if (((rtype == MDNS_RECORDTYPE_A) || (rtype == MDNS_RECORDTYPE_ANY)) &&
		    (service->address_ipv4.sin_family == AF_INET)) {
			// The A query was for our qualified hostname (typically "<hostname>.local.") and we
			// have an IPv4 address, answer with an A record mappiing the hostname to an IPv4
			// address, as well as any IPv6 address for the hostname, and two test TXT records

			// Answer A records mapping "<hostname>.local." to IPv4 address
			mdns_record_t answer = service->record_a;

			mdns_record_t additional[5] = {0};
			size_t additional_count = 0;

			// AAAA record mapping "<hostname>.local." to IPv6 addresses
			if (service->address_ipv6.sin6_family == AF_INET6)
				additional[additional_count++] = service->record_aaaa;

			// Add two test TXT records for our service instance name, will be coalesced into
			// one record with both key-value pair strings by the library
			additional[additional_count++] = service->txt_record[0];
			additional[additional_count++] = service->txt_record[1];

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
			mdns_string_t addrstr = ip_address_to_string(
			    addrbuffer, sizeof(addrbuffer), (struct sockaddr*)&service->record_a.data.a.addr,
			    sizeof(service->record_a.data.a.addr));
			printf("  --> answer %.*s IPv4 %.*s (%s)\n", MDNS_STRING_FORMAT(service->record_a.name),
			       MDNS_STRING_FORMAT(addrstr), (unicast ? "unicast" : "multicast"));

			if (unicast) {
				mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
				                          query_id, (mdns_record_type)rtype, name.str, name.length, answer, 0, 0,
				                          additional, additional_count);
			} else {
				mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0,
				                            additional, additional_count);
			}
		} else if (((rtype == MDNS_RECORDTYPE_AAAA) || (rtype == MDNS_RECORDTYPE_ANY)) &&
		           (service->address_ipv6.sin6_family == AF_INET6)) {
			// The AAAA query was for our qualified hostname (typically "<hostname>.local.") and we
			// have an IPv6 address, answer with an AAAA record mappiing the hostname to an IPv6
			// address, as well as any IPv4 address for the hostname, and two test TXT records

			// Answer AAAA records mapping "<hostname>.local." to IPv6 address
			mdns_record_t answer = service->record_aaaa;

			mdns_record_t additional[5] = {0};
			size_t additional_count = 0;

			// A record mapping "<hostname>.local." to IPv4 addresses
			if (service->address_ipv4.sin_family == AF_INET)
				additional[additional_count++] = service->record_a;

			// Add two test TXT records for our service instance name, will be coalesced into
			// one record with both key-value pair strings by the library
			additional[additional_count++] = service->txt_record[0];
			additional[additional_count++] = service->txt_record[1];

			// Send the answer, unicast or multicast depending on flag in query
			uint16_t unicast = (rclass & MDNS_UNICAST_RESPONSE);
			mdns_string_t addrstr =
			    ip_address_to_string(addrbuffer, sizeof(addrbuffer),
			                         (struct sockaddr*)&service->record_aaaa.data.aaaa.addr,
			                         sizeof(service->record_aaaa.data.aaaa.addr));
			printf("  --> answer %.*s IPv6 %.*s (%s)\n",
			       MDNS_STRING_FORMAT(service->record_aaaa.name), MDNS_STRING_FORMAT(addrstr),
			       (unicast ? "unicast" : "multicast"));

			if (unicast) {
				mdns_query_answer_unicast(sock, from, addrlen, sendbuffer, sizeof(sendbuffer),
				                          query_id, (mdns_record_type)rtype, name.str, name.length, answer, 0, 0,
				                          additional, additional_count);
			} else {
				mdns_query_answer_multicast(sock, sendbuffer, sizeof(sendbuffer), answer, 0, 0,
				                            additional, additional_count);
			}
		}
	}
	return 0;
}

// Provide a mDNS service, answering incoming DNS-SD and mDNS queries
void service_mdns(const char* hostname, const char* service_name, int service_port) {
	int sockets[32];
	int num_sockets = open_service_sockets(sockets, sizeof(sockets) / sizeof(sockets[0]));
	if (num_sockets <= 0) {
		printf("Failed to open any client sockets\n");
		return;
	}
	printf("Opened %d socket%s for mDNS service\n", num_sockets, num_sockets ? "s" : "");

	size_t service_name_length = strlen(service_name);
	if (!service_name_length) {
		printf("Invalid service name\n");
		return;
	}

	char* service_name_buffer = (char *)malloc(service_name_length + 2);
	memcpy(service_name_buffer, service_name, service_name_length);
	if (service_name_buffer[service_name_length - 1] != '.')
		service_name_buffer[service_name_length++] = '.';
	service_name_buffer[service_name_length] = 0;
	service_name = service_name_buffer;

	printf("Service mDNS: %s:%d\n", service_name, service_port);
	printf("Hostname: %s\n", hostname);

	size_t capacity = 2048;
	void* buffer = malloc(capacity);

	mdns_string_t service_string;
  service_string.str = service_name;
  service_string.length = strlen(service_name);
	mdns_string_t hostname_string;
  hostname_string.str = hostname;
  hostname_string.length = strlen(hostname);

	// Build the service instance "<hostname>.<_service-name>._tcp.local." string
	char service_instance_buffer[256] = {0};
	snprintf(service_instance_buffer, sizeof(service_instance_buffer) - 1, "%.*s.%.*s",
	         MDNS_STRING_FORMAT(hostname_string), MDNS_STRING_FORMAT(service_string));

  mdns_string_t service_instance_string;
  service_instance_string.str = service_instance_buffer;
  service_instance_string.length = strlen(service_instance_buffer);

	// Build the "<hostname>.local." string
	char qualified_hostname_buffer[256] = {0};
	snprintf(qualified_hostname_buffer, sizeof(qualified_hostname_buffer) - 1, "%.*s.local.",
	         MDNS_STRING_FORMAT(hostname_string));
	mdns_string_t hostname_qualified_string;
  hostname_qualified_string.str = qualified_hostname_buffer;
  hostname_qualified_string.length = strlen(qualified_hostname_buffer);

	service_t service = {0};
	service.service = service_string;
	service.hostname = hostname_string;
	service.service_instance = service_instance_string;
	service.hostname_qualified = hostname_qualified_string;
	service.address_ipv4 = service_address_ipv4;
	service.address_ipv6 = service_address_ipv6;
	service.port = service_port;

	// Setup our mDNS records

	// PTR record reverse mapping "<_service-name>._tcp.local." to
	// "<hostname>.<_service-name>._tcp.local."
  service.record_ptr.name = service.service;
  service.record_ptr.type = MDNS_RECORDTYPE_PTR;
  service.record_ptr.rclass = 0;
  service.record_ptr.ttl = 0;
  service.record_ptr.data.ptr.name = service.service_instance;

	// SRV record mapping "<hostname>.<_service-name>._tcp.local." to
	// "<hostname>.local." with port. Set weight & priority to 0.
  service.record_srv.name = service.service_instance;
  service.record_srv.type = MDNS_RECORDTYPE_SRV;
  service.record_srv.data.srv.name = service.hostname_qualified;
  service.record_srv.data.srv.port = service.port;
  service.record_srv.data.srv.priority = 0;
  service.record_srv.data.srv.weight = 0;
  service.record_srv.rclass = 0;
  service.record_srv.ttl = 0;


 	// A/AAAA records mapping "<hostname>.local." to IPv4/IPv6 addresses

  service.record_a.name = service.hostname_qualified;
  service.record_a.type = MDNS_RECORDTYPE_A;
  service.record_a.data.a.addr = service.address_ipv4;
  service.record_a.rclass = 0;
  service.record_a.ttl = 0;


  service.record_aaaa.name = service.hostname_qualified;
  service.record_aaaa.type = MDNS_RECORDTYPE_AAAA;
  service.record_aaaa.data.aaaa.addr = service.address_ipv6;
  service.record_aaaa.rclass = 0;
  service.record_aaaa.ttl = 0;


 	// Add two test TXT records for our service instance name, will be coalesced into
	// one record with both key-value pair strings by the library
#if 0
  service.txt_record[0].name = service.service_instance;
  service.txt_record[0].type = MDNS_RECORDTYPE_TXT;
  service.txt_record[0].data.txt.key = {MDNS_STRING_CONST("test")};
  service.txt_record[0].data.txt.value = {MDNS_STRING_CONST("1")};
  service.txt_record[0].rclass = 0;
  service.txt_record[0].ttl = 0;

  service.txt_record[1].name = service.service_instance;
  service.txt_record[1].type = MDNS_RECORDTYPE_TXT;
  service.txt_record[1].data.txt.key = {MDNS_STRING_CONST("other")};
  service.txt_record[1].data.txt.value = {MDNS_STRING_CONST("value")};
  service.txt_record[1].rclass = 0;
  service.txt_record[1].ttl = 0;
#endif


	// Send an announcement on startup of service
	{
		printf("Sending announce\n");
		mdns_record_t additional[5] = {0};
		size_t additional_count = 0;
		additional[additional_count++] = service.record_srv;
		if (service.address_ipv4.sin_family == AF_INET)
			additional[additional_count++] = service.record_a;
		if (service.address_ipv6.sin6_family == AF_INET6)
			additional[additional_count++] = service.record_aaaa;
		//additional[additional_count++] = service.txt_record[0];
		//additional[additional_count++] = service.txt_record[1];

		for (int isock = 0; isock < num_sockets; ++isock)
			mdns_announce_multicast(sockets[isock], buffer, capacity, service.record_ptr, 0, 0,
			                        additional, additional_count);
	}

	// This is a crude implementation that checks for incoming queries
	while (running_server) {
		int nfds = 0;
		fd_set readfs;
		FD_ZERO(&readfs);
		for (int isock = 0; isock < num_sockets; ++isock) {
			if (sockets[isock] >= nfds)
				nfds = sockets[isock] + 1;
			FD_SET(sockets[isock], &readfs);
		}

		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;

		if (select(nfds, &readfs, 0, 0, &timeout) >= 0) {
			for (int isock = 0; isock < num_sockets; ++isock) {
				if (FD_ISSET(sockets[isock], &readfs)) {
					mdns_socket_listen(sockets[isock], buffer, capacity,
                             ocpn_service_callback,
					                   &service);
				}
				FD_SET(sockets[isock], &readfs);
			}
		} else {
			break;
		}
	}

	// Send a goodbye on end of service
	{
		printf("Sending goodbye\n");
		mdns_record_t additional[5] = {0};
		size_t additional_count = 0;
		additional[additional_count++] = service.record_srv;
		if (service.address_ipv4.sin_family == AF_INET)
			additional[additional_count++] = service.record_a;
		if (service.address_ipv6.sin6_family == AF_INET6)
			additional[additional_count++] = service.record_aaaa;
		additional[additional_count++] = service.txt_record[0];
		additional[additional_count++] = service.txt_record[1];

		for (int isock = 0; isock < num_sockets; ++isock)
			mdns_goodbye_multicast(sockets[isock], buffer, capacity, service.record_ptr, 0, 0,
			                        additional, additional_count);
	}

	free(buffer);
	free(service_name_buffer);

	for (int isock = 0; isock < num_sockets; ++isock)
		mdns_socket_close(sockets[isock]);
	printf("Closed socket%s\n", num_sockets ? "s" : "");

	return;
}

std::string host;
std::string service;

int StartMDNSService(std::string hostname,
                     std::string service_name,
                     int service_port){

  host = hostname;
  service = service_name;

  std::thread  mdns_service_thread(service_mdns, host.c_str(), service.c_str(), service_port);
  mdns_service_thread.detach();


  return 0;
}

bool StopMDNSService(){

  return true;
}


