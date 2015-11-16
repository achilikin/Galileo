/*	Apache 2.0 License
	
	Copyright (c) 2015 Andrey Chilikin https://github.com/achilikin

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

	http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/
#ifndef __YAHL_NET_IF_H__
#define __YAHL_NET_IF_H__

#ifdef __ARDUINO_X86__

#include <stdint.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <netinet/in.h>

/*
	Network interface information from 'ifconfig' command
*/

#ifdef __cplusplus
extern "C" {
#endif

#define HW_ADDRSTRLEN 18

// network interface statistics
typedef struct iostat_s
{
	uint32_t packets;
	uint32_t errors;
	uint32_t dropped;
	uint32_t bytes;		// increment to uint64_t if needed
} iostat_t;

// network interface information
typedef struct netif_s
{
	char name[IFNAMSIZ];
	char hwas[HW_ADDRSTRLEN];     // HW address string
	char ip4as[INET_ADDRSTRLEN];  // IPv4 address string
	char ip6as[INET6_ADDRSTRLEN]; // IPv4 address string
	iostat_t rx;
	iostat_t tx;
} netif_t;

// interface name to get information for
int get_netif_info(const char *name, netif_t *netif);

#ifdef __cplusplus
}
#endif

#endif
#endif
