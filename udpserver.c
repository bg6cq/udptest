/* UDPserver: receive udp
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

#define MAX_PACKET_SIZE		65536

void usage(void)
{
	printf("Usage:\n");
	printf("./udpserver [ -4 | -6 ] listen_port\n");
	exit(0);
}

unsigned long int udp_len, eth_len, wire_len;
unsigned long int packet_count = 0;

int main(int argc, char *argv[])
{
	char buf[MAX_PACKET_SIZE];
	int sockfd;
	if (argc != 3)
		usage();
	fprintf(stderr, "udpserver %s listen on port %d\n", argv[1], atoi(argv[2]));
	if (strcmp(argv[1], "-4") == 0) {	// IPv4
		struct sockaddr_in addr;
		int addr_len;
		if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			perror("socket:");
			return (1);
		} else {
			printf("socket created ...\n");
			printf("socket id :%d \n", sockfd);
		}

		addr_len = sizeof(struct sockaddr_in);
		bzero(&addr, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(atoi(argv[2]));
		addr.sin_addr.s_addr = INADDR_ANY;
		if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("bind");
			return (1);
		}
	} else {		// IPv6
		struct sockaddr_in6 addr;
		int addr_len;
		if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
			perror("socket:");
			return (1);
		} else {
			printf("socket created ...\n");
			printf("socket id :%d \n", sockfd);
		}

		addr_len = sizeof(struct sockaddr_in6);
		bzero(&addr, sizeof(addr));
		addr.sin6_family = AF_INET6;
		addr.sin6_port = htons(atoi(argv[2]));
		addr.sin6_addr = in6addr_any;
		if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
			perror("bind");
			return (1);
		}
	}
	int n;
	socklen_t ln;
	if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, &ln) == 0) {
		fprintf(stderr, "UDP socket RCVBUF was %d\n", n);
		n = 40 * 1024 * 1024;
		setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));
		if (getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, &ln) == 0) {
			fprintf(stderr, "UDP socket RCVBUF setting to %d\n", n);
		}

	}
	fprintf(stderr, "waiting for UDP packets");
	while (1) {
		int got_packet;
		struct timeval tv;
		struct timeval start_tm, end_tm;
		got_packet = 0;
		udp_len = eth_len = wire_len = packet_count = 0;
		fprintf(stderr, ".");
		while (1) {
			fd_set readfds;
			int r;
			FD_ZERO(&readfds);
			FD_SET(sockfd, &readfds);
			tv.tv_sec = 1;
			tv.tv_usec = 0;
			select(sockfd + 1, &readfds, NULL, NULL, &tv);
			if (FD_ISSET(sockfd, &readfds)) {	//got packet
				if (got_packet == 0) {	// this is first packet
					//printf("============\n I got first packet\n");
					gettimeofday(&start_tm, NULL);
					got_packet = 1;
				}
				r = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0, NULL, 0);
				// printf("r=%d\n", r);
				if (r <= 0) {
					printf("r=%d\n", r);
					continue;
				}
				packet_count++;
				udp_len += r;
				if (r <= 18) {
					eth_len += 60;	// MAC+Pro+IP+UDP= 12+2+20+8 = 42
					wire_len += 84;
				} else {
					eth_len += (r + 42);
					wire_len += (r + 42 + 24);	// IPG+PRE+CRC=12+8+4 = 24
				}
			} else
				break;	// timeout
		}
		//printf("time out\n");
		if (got_packet == 0) {
			//printf("noting get, rerun\n");
			continue;
		}
		gettimeofday(&end_tm, NULL);
		float tspan = ((end_tm.tv_sec - start_tm.tv_sec - 1) * 1000000L + end_tm.tv_usec) - start_tm.tv_usec;
		tspan = tspan / 1000000L;
		fprintf(stderr, "\n%0.3f seconds %lu packets %lu bytes\n", tspan, packet_count, udp_len);
		fprintf(stderr, "PPS: %.0f PKT/S\n", (float)packet_count / tspan);
		fprintf(stderr, "UDP BPS: %.0f BPS\n", 8.0 * (udp_len) / tspan);
		fprintf(stderr, "ETH BPS: %.0f BPS\n", 8.0 * (eth_len) / tspan);
		fprintf(stderr, "WireBPS: %.0f BPS\n", 8.0 * (wire_len) / tspan);
		fprintf(stderr, "========================================\n");
		fprintf(stderr, "waiting for UDP packets");
	}
	exit(0);
}
