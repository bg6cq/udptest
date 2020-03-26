/* UDPmtuserver: server side
	  by james@ustc.edu.cn 2009.04.02
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <net/if.h>
#include <stdarg.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "util.c"

#define MAX_PACKET_SIZE		65536

void usage(void)
{
	printf("Usage:\n");
	printf("./udpserver listen_address listen_port\n");
	exit(0);
}

int header_len = 28;

int main(int argc, char *argv[])
{
	if (argc != 3)
		usage();
	fprintf(stderr, "udpserver listen on port %s:%d\n", argv[1], atoi(argv[2]));

	unsigned char buf[MAX_PACKET_SIZE];
	int sockfd, connected = 0;
	struct addrinfo hints, *res;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(argv[1], argv[2], &hints, &res) != 0) {
		fprintf(stderr, "host name lookup error for %s, %s", argv[1], argv[2]);
		exit(0);
	}
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0) {
			perror("socket");
			continue;
		}
		if (bind(sockfd, res->ai_addr, res->ai_addrlen) >= 0) {
			if (res->ai_family == AF_INET6)
				header_len = 48;
			connected = 1;
			break;
		}
	}
	while ((res = res->ai_next) != NULL);

	if (connected == 0) {
		fprintf(stderr, "socket and bind error, please check listen addr\n");
		exit(0);
	}
	int n;
	socklen_t ln;
	if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, &ln) == 0) {
		fprintf(stderr, "UDP socket RCVBUF was %d\n", n);
		n = 64 * 1024;
		setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));
		if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, &ln) == 0) {
			fprintf(stderr, "UDP socket RCVBUF setting to %d\n", n);
		}

	}
	fprintf(stderr, "waiting for UDP packets\n");
	while (1) {
		int r;
		struct sockaddr_storage ss;
		socklen_t sock_len = sizeof(struct sockaddr_storage);
		r = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&ss, &sock_len);
		if (r <= 0)
			continue;
		fprintf(stderr, "recv UDP packet udp_len=%d ip_len=%d\n", r, r + header_len);
		if (memcmp(buf, "REQ", 3) == 0) {
			buf[r] = 0;
			int x;
			if (sscanf((char *)(buf + 3), "%d", &x) == 1) {
				strcpy((char *)buf, "RET");
				fill_buffer(buf + 3, x - 3);
				r = sendto(sockfd, buf, x, 0, (struct sockaddr *)&ss, sock_len);
				fprintf(stderr, "sending %d bytes RET packet\n", x);
				if (r <= 0)
					perror("sendto");
			}
			continue;
		}
		strcpy((char *)buf, "ACK");
		r = 3 + sprintf((char *)buf + 3, "%d", r);
		r = sendto(sockfd, buf, r, 0, (struct sockaddr *)&ss, sock_len);
		fprintf(stderr, "sending %d bytes ACK packet\n", r);
		if (r <= 0)
			perror("sendto");
	}
	exit(0);
}
