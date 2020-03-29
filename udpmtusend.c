/*
 * UDPmtusend: send udp packet to test mtu by james@ustc.edu.cn 2009.04.02
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
#include <locale.h>

#define MAX_PACKET_SIZE		65536

int min_len = 1470, max_len = 1500;
int header_len = 28;

#include "util.c"

void usage(void)
{
	printf("Usage:\n");
	printf("./udpsend remoteip port [ min_len max_len ]\n");
	printf(" send UDP packets to remoteip port\n");
	printf("Note: \n");
	printf("UDP len   ether MTU\n");
	printf("   1472        1500\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int n;
	unsigned char buf[MAX_PACKET_SIZE];
	setlocale(LC_NUMERIC, "");

	if (argc < 3)
		usage();

	if (argc >= 4)
		min_len = atoi(argv[3]);
	if (argc >= 5)
		max_len = atoi(argv[4]);

	if (min_len < 1000)
		min_len = 1000;

	if (min_len > MAX_PACKET_SIZE)
		min_len = MAX_PACKET_SIZE;

	if (max_len < min_len)
		max_len = min_len;
	if (max_len > MAX_PACKET_SIZE)
		max_len = MAX_PACKET_SIZE;

	fprintf(stderr, "sending %d - %d bytes UDP to %s:%s\n", min_len, max_len, argv[1], argv[2]);

	int sockfd;
	struct addrinfo hints, *res;
	int connected = 0;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if (getaddrinfo(argv[1], argv[2], &hints, &res) != 0) {
		fprintf(stderr, "host name lookup error for %s %s", argv[1], argv[2]);
	}
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		if (sockfd < 0)
			perror("socket");

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0) {
			if (res->ai_family == AF_INET6)
				header_len = 48;
			connected = 1;
			break;	/* success */
		}
	}
	while ((res = res->ai_next) != NULL);

	if (connected == 0) {
		fprintf(stderr, "connect to %s %s error\n", argv[1], argv[2]);
		exit(0);
	}
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

	for (n = min_len; n <= max_len; n++) {
		int r;
		fprintf(stderr, "udp_len=%d  ip_pkt_len=%d  ", n, n + header_len);

		strcpy((char *)buf, "PKT");
		fill_buffer(buf + 3, n - 3);
		r = send(sockfd, buf, n, 0);
		if (r < 0)
			perror("send udp");

		strcpy((char *)buf, "REQ");
		r = 3 + sprintf((char *)buf + 3, "%d", n);
		r = send(sockfd, buf, r, 0);
		if (r < 0)
			perror("send udp");

		int c;
		for (c = 0; c < 2; c++) {
			r = recv(sockfd, buf, MAX_PACKET_SIZE, 0);
			if (r <= 0) {
				fprintf(stderr, ".");
				continue;
			}
			if (memcmp(buf, "RET", 3) == 0) {
				if (r == n)
					fprintf(stderr, "S-->C OK   ");
				else
					fprintf(stderr, "S-->C %d bytes? ", r);
			} else if (memcmp(buf, "ACK", 3) == 0) {
				int x;
				buf[r] = 0;
				sscanf((char *)(buf + 3), "%d", &x);
				if (x == n)
					fprintf(stderr, "C-->S OK   ");
				else
					fprintf(stderr, "C-->S %d bytes? ", x);
			}
		}
		fprintf(stderr, "\n");
	}
	fprintf(stderr, "done\n");
	exit(0);
}
