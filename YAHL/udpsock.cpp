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
#include <Arduino.h>

#ifdef __ARDUINO_X86__

#include <poll.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "udpsock.h"

UdpSocket::UdpSocket(void)
{
	sock = -1;
	saddr[0] = '\0';
	caddr[0] = '\0';
}

void UdpSocket::end(void)
{
	if (sock != -1)
		close(sock);
	sock = -1;
	saddr[0] = '\0';
	caddr[0] = '\0';
}

UdpSocket::~UdpSocket(void)
{
	end();
}

int UdpSocket::create(void)
{
	end();

	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		return -1;

	return sock;
}

int UdpSocket::begin(uint16_t port, const char *ip_addr)
{
	if (port == 0)
		return -1;

	end();
	if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		return -1;

	int opt = 1;
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	uint32_t addrlen = sizeof(addr);
	memset(&addr, 0, addrlen);
	addr.sin_family = AF_INET;
	if (ip_addr == NULL) {
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		strcpy(saddr, "localhost");
	}
	else {
		addr.sin_addr.s_addr = inet_addr(ip_addr);
		strcpy(saddr, ip_addr);
	}
	addr.sin_port = htons(port);

	if (bind(sock, (struct sockaddr *)&addr, addrlen) < 0) {
		end();
		return 0;
	}

	opt = 1;
	setsockopt(sock, IPPROTO_IP, IP_PKTINFO, &opt, sizeof(opt));

	return sock;
}

/* check is socket is ready to be read */
int UdpSocket::ready(void)
{
	pollfd fd;

	fd.fd = sock;
	fd.events = POLLIN;

	if (poll(&fd, 1, 0) > 0)
		return 1;
	return 0;
}

uint16_t UdpSocket::get_client_port(void)
{
	if (connected())
		return ntohs(client.sin_port);
	return 0;
}

/* read our socket and store client address */
int UdpSocket::read(char *msg, uint32_t msglen)
{
	if (sock == -1)
		return 0;

	int rlen;

	struct iovec iov;
	struct msghdr mh;
	struct cmsghdr *pcmsg;
	char cmsg[CMSG_SPACE(sizeof(struct in_pktinfo))];

	/* receive a message from remote side */
	iov.iov_base = msg;
	iov.iov_len = msglen - 1; /* -1 to allocate terminating null */

	memset(&mh, 0, sizeof(mh));
	mh.msg_name = &client;
	mh.msg_namelen = sizeof(struct sockaddr);
	mh.msg_iov = &iov;
	mh.msg_iovlen = 1;
	mh.msg_control = cmsg;
	mh.msg_controllen = sizeof(cmsg);

	rlen = recvmsg(sock, &mh, 0);
	if (rlen < 1)
		return 0;
	msg[rlen] = '\0';

	/* make sure to receive valid ASCII string */
	for(int i = 0; i < rlen; i++) {
		if (!isprint(msg[i])) {
			msg[i] = '\0';
			rlen = i - 1;
			break;
		}
	}
	if (rlen <= 1)
		return 0;
	
	// to get information on what IP address this packet was received on
	// uncomment code block below
#if 0
	struct in_pktinfo *pi = NULL;
	pcmsg = CMSG_FIRSTHDR(&mh);
	for(; pcmsg != NULL; pcmsg = CMSG_NXTHDR(&mh, pcmsg)) {
		if (pcmsg->cmsg_level != IPPROTO_IP || pcmsg->cmsg_type != IP_PKTINFO) {
			continue;
		}
		pi = (struct in_pktinfo*)CMSG_DATA(pcmsg);
		break;
	}

	if (pi != NULL)
		inet_ntop(AF_INET, &pi->ipi_spec_dst.s_addr, saddr, INET_ADDRSTRLEN);
	else
		strcpy(saddr, "localhost");
#endif

	inet_ntop(AF_INET, &client.sin_addr, caddr, INET_ADDRSTRLEN);
	rlen = strlen(msg);

	return rlen;
}

int UdpSocket::write(const char *msg, int msglen)
{
	if (!connected())
		return 0;

	if (msglen < 0)
		msglen = strlen(msg) + 1;

	int rlen = sendto(sock, msg, msglen, 0, (struct sockaddr *)&client, sizeof(client));
	return rlen;
}

int	UdpSocket::print(const char *format, ...)
{
	int len;
	char buffer[UDP_MAX_LINE];
	va_list ap;

	va_start(ap, format);
//	int len = vsnprintf(NULL, 0, format, ap);
//	char *buffer = (char *)alloca(len);
	len = vsnprintf(buffer, UDP_MAX_LINE, format, ap);
	va_end(ap);

	write(buffer, -1);

	return len;
}

int UdpSocket::send(const char *ip_addr, uint16_t port, const char *msg, int msglen)
{
	struct sockaddr_in uaddr; 
	uint32_t addrlen = sizeof(uaddr);
	memset(&uaddr, 0, addrlen);
	uaddr.sin_family = AF_INET;
	uaddr.sin_addr.s_addr = inet_addr(ip_addr);
	uaddr.sin_port = htons(port);

	if (msglen < 0)
		msglen = strlen(msg) + 1;

	int rlen = sendto(sock, msg, msglen, 0, (struct sockaddr *)&uaddr, sizeof(uaddr));
	return rlen;
}

#endif
