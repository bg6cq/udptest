/* UDPmtuserver: receive udp
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
		fprintf(stderr, ".");
		while (1) {
			int r;
			struct sockaddr_storage ss;
			socklen_t sock_len = sizeof(struct sockaddr_storage);
			r = recvfrom(sockfd, buf, MAX_PACKET_SIZE, 0, (struct sockaddr *)&ss, &sock_len);
			fprintf(stderr,"recv packet udp_len=%d ether_len=%d\n", r, r+28);
			if (r <= 0) {
				continue;
			}
			sendto(sockfd, buf, 100, 0, (struct sockaddr *)&ss, sock_len);
			sendto(sockfd, buf, r, 0, (struct sockaddr *)&ss, sock_len);
			sendto(sockfd, buf, 1473, 0, (struct sockaddr *)&ss, sock_len);
		}
	}
	exit(0);
}
