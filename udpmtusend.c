/* UDPmtusend: send udp packet to test mtu
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
#include <locale.h>

#define MAX_PACKET_SIZE		65536

int udp_len = 1472, eth_len;
unsigned long int pkt_cnt, packet_count = 1;

void usage(void)
{
	printf("Usage:\n");
	printf("./udpsend [ options ] remoteip port udp_len \n");
	printf("    options: \n");
	printf("         -c packet_cout   default %lu\n", packet_count);
	printf(" send UDP packets to remoteip port\n");
	printf("Note: \n");
	printf("UDP len   ether MTU\n");
	printf("   1472        1500\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int i = 1;
	char buf[MAX_PACKET_SIZE];
	setlocale(LC_NUMERIC, "");
	int got_one = 0;
	do {
		got_one = 1;
		if (argc - i <= 0)
			usage();

		if (strcmp(argv[i], "-c") == 0) {
			i++;
			if (argc - i <= 0)
				usage();
			packet_count = atoi(argv[i]);
		} else
			got_one = 0;
		if (got_one)
			i++;
	}
	while (got_one);

	if (argc < i + 2)
		usage();

	if (argc == i +3 ) 
		udp_len = atoi(argv[i+2]);
	

	if (udp_len > MAX_PACKET_SIZE)
		udp_len = MAX_PACKET_SIZE;

	int n;
	eth_len = udp_len + 28;
	if (eth_len <= 50) 
		eth_len = 50;
	fprintf(stderr, "udp_len = %d, eth_len = %d, packet_count = %lu\n", udp_len, eth_len, packet_count);
	
	fprintf(stderr, "sending to ");
	for (n = i; n < i+2; n++)
		fprintf(stderr, "%s ", argv[n]);
	printf("\n");

	int sockfd;
	struct addrinfo hints, *res, *ressave;

	bzero(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	if ((n = getaddrinfo(argv[i], argv[i + 1], &hints, &res)) != 0) {
		fprintf(stderr, "host name lookup error for %s, %s", argv[i], argv[i + 1]);
	}
	ressave = res;
	do {
		sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

		if (connect(sockfd, res->ai_addr, res->ai_addrlen) == 0)
			break;	/* success */
	}
	while ((res = res->ai_next) != NULL);

	memset(buf, 'a', udp_len);
	for (pkt_cnt = 0; pkt_cnt < packet_count; pkt_cnt++) {
		int r;
		r = send(sockfd, buf, udp_len, 0);
		if (r<0)
			perror("send udp");
		r = recv(sockfd, buf, MAX_PACKET_SIZE, 0);
		fprintf(stderr, "recv %d bytes\n", r);
		r = recv(sockfd, buf, MAX_PACKET_SIZE, 0);
		fprintf(stderr, "recv %d bytes\n", r);
		r = recv(sockfd, buf, MAX_PACKET_SIZE, 0);
		fprintf(stderr, "recv %d bytes\n", r);
	}
	fprintf(stderr, "done\n");
	exit(0);
}
