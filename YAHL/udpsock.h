/*	Apache 2.0 License
	
	Copyright (c) 2014 Andrey Chilikin https://github.com/achilikin

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
#ifndef __YAHL_UDP_SOCKET_H__
#define __YAHL_UDP_SOCKET_H__

#include <stdarg.h>
#include <netinet/in.h>

#define UDP_MAX_LINE 512

class UdpSocket {
private:
	int sock;
	struct sockaddr_in addr;	 // server IPv4 address string
	struct sockaddr_in client;	 // client IPv4 address string
	char saddr[INET_ADDRSTRLEN]; // server IPv4 address string
	char caddr[INET_ADDRSTRLEN]; // client IPv4 address string

public:
	UdpSocket(void);
	~UdpSocket(void);

	// create UDP socket for send()
	int create(void);
	// begin server listening on specified port
	int  begin(uint16_t port, const char *ip_addr = NULL);
	// close UDP socket
	void end(void);

	// sends data to specified ip and port
	// if msglen == -1 then msglen = strlen(msg)+1
	int send(const char *ip_addr, uint16_t port, const char *msg, int msglen = -1);
	
	// following function work only in server mode

	// returns true if data is ready to be read
	int ready(void);
	// reads received packet
	int read(char *buffer, uint32_t buflen);

	// checks if any client connected so far
	int connected(void) { return (caddr[0] == '\0') ? 0 : 1; }
	// returns UDP port of the last received packet
	uint16_t get_client_port(void);
	// returns IPv$ address of the last received packet
	const char *get_client_ip(void) { return caddr; }

	// sends a message using ip and port of the last received packet
	int write(const char *msg, int msglen = -1);
	int	print(const char *format, ...);
};

#endif
