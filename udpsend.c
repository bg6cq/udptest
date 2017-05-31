/* UDPsend: send udp 
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

int udp_len = 1472, eth_len, wire_len;
int send_buf_size = 4 * 1024 * 1024;
unsigned long int pkt_cnt, packet_count = 10;
int ignore_error;

void usage(void)
{
	printf("Usage:\n");
	printf("./udpsend [ options ] remoteip port\n");
	printf("    options: \n");
	printf("         -l udp_len    default %d\n", udp_len);
	printf("         -c packet_cout   default %lu\n", packet_count);
	printf("         -b send_buf_size default %u\n", send_buf_size);
	printf("         -i               ignore erros\n");
	printf(" send UDP packets to remoteip port\n");
	printf("Note: \n");
	printf("long udp packet, len=1472, count=81274\n");
	printf("small udp packet, len<=18, count=1488095\n");
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

		if (strcmp(argv[i], "-i") == 0) {
			ignore_error = 1;
		} else if (strcmp(argv[i], "-l") == 0) {
			i++;
			if (argc - i <= 0)
				usage();
			udp_len = atoi(argv[i]);
		} else if (strcmp(argv[i], "-c") == 0) {
			i++;
			if (argc - i <= 0)
				usage();
			packet_count = atoi(argv[i]);
		} else if (strcmp(argv[i], "-b") == 0) {
			i++;
			if (argc - i <= 0)
				usage();
			send_buf_size = atoi(argv[i]);
		} else
			got_one = 0;
		if (got_one)
			i++;
	}
	while (got_one);

	if (argc != i + 2)
		usage();

	if (udp_len > MAX_PACKET_SIZE)
		udp_len = MAX_PACKET_SIZE;

	int n;
	fprintf(stderr, "udp_len = %d, packet_count = %lu\n", udp_len, packet_count);
	if (udp_len <= 18) {
		eth_len = 60;	// MAC+Pro+IP+UDP= 12+2+20+8 = 42
		wire_len = 84;
	} else {
		eth_len = udp_len + 42;
		wire_len = udp_len + 42 + 24;	// IPG+PRE+CRC=12+8+4 = 24
	}
	fprintf(stderr,"total udp_len = %lu, eth_len=%lu, wire_len=%lubits\n",
			udp_len * packet_count, eth_len*packet_count, wire_len*packet_count*8);

	fprintf(stderr, "sending to ");
	for (n = i; n < argc; n++)
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

	setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, (char *)&send_buf_size, sizeof(send_buf_size));

	memset(buf, 'a', udp_len);
	struct timeval start_tm, end_tm;
	unsigned long int pkt_cnt;
	gettimeofday(&start_tm, NULL);
	for (pkt_cnt = 0; pkt_cnt < packet_count; pkt_cnt++) {
		int r;
		r = send(sockfd, buf, udp_len, 0);
		if ((ignore_error == 0) && (r < 0)) {
			fprintf(stderr, "send error, send %lu, remains %lu packets\n", pkt_cnt, packet_count - pkt_cnt);
			exit(0);
		}
	}
	gettimeofday(&end_tm, NULL);
	float tspan = ((end_tm.tv_sec - start_tm.tv_sec) * 1000000L + end_tm.tv_usec) - start_tm.tv_usec;
	tspan = tspan / 1000000L;
	fprintf(stderr, "%0.3f seconds %lu packets %lu bytes\n", tspan, packet_count, packet_count * udp_len);
	fprintf(stderr, "PPS: %.0f PKT/S\n", (float)packet_count / tspan);


	fprintf(stderr, "UDP BPS: %.0f BPS\n", 8.0 * (udp_len) * (float)packet_count / tspan);
	fprintf(stderr, "ETH BPS: %.0f BPS\n", 8.0 * (eth_len) * (float)packet_count / tspan);
	fprintf(stderr, "WireBPS: %.0f BPS\n", 8.0 * (wire_len) * (float)packet_count / tspan);
	fprintf(stderr, "done\n");
	exit(0);
}
