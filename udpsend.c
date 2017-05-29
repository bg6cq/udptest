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

#define MAX_PACKET_SIZE		65536

int packet_len = 1472;
unsigned long int pkt_cnt, packet_count = 10;
int ignore_error;

void usage(void)
{
	printf("Usage:\n");
	printf("./udpsend [ options ] remoteip remoteport\n");
	printf("    options: \n");
	printf("         -l packet_len    default is %d\n", packet_len);
	printf("         -c packet_cout   default is %lu\n", packet_count);
	printf(" send UDP packets to remoteip\n");
	exit(0);
}

int main(int argc, char *argv[])
{
	int i = 1;
	char buf[MAX_PACKET_SIZE];
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
			packet_len = atoi(argv[i]);
		} else if (strcmp(argv[i], "-c") == 0) {
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

	if (argc != i + 2)
		usage();

	fprintf(stderr, "packet_len = %d, packet_count = %lu\n", packet_len, packet_count);

	int n;
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

	if (packet_len > MAX_PACKET_SIZE)
		packet_len = MAX_PACKET_SIZE;
	memset(buf, 'a', packet_len);
	struct timeval start_tm, end_tm;
	gettimeofday(&start_tm, NULL);
	pkt_cnt = packet_count;
	while (1) {
		int r;
		r = send(sockfd, buf, packet_len, 0);
		if ((ignore_error == 0) && (r < 0)) {
			fprintf(stderr, "send error, send %lu, remains %lu packets\n", packet_count - pkt_cnt, pkt_cnt);
			exit(0);
		}

		pkt_cnt--;
		if (pkt_cnt == 0)
			break;
	}
	gettimeofday(&end_tm, NULL);
	float tspan = ((end_tm.tv_sec - start_tm.tv_sec) * 1000000L + end_tm.tv_usec) - start_tm.tv_usec;
	tspan = tspan / 1000000L;
	fprintf(stderr, "%0.3f seconds %lu packets %lu bytes\n", tspan, packet_count, packet_count * packet_len);
	fprintf(stderr, "PPS: %.0f PKT/S\n", (float)packet_count / tspan);
	fprintf(stderr, "UDP BPS: %.0f BPS\n", 8.0 * (packet_len) * (float)packet_count / tspan);
	fprintf(stderr, "ETH BPS: %.0f BPS\n", 8.0 * (packet_len + 28) * (float)packet_count / tspan);
	fprintf(stderr, "WireBPS: %.0f BPS\n", 8.0 * (packet_len + 28 + 38) * (float)packet_count / tspan);
	fprintf(stderr, "done\n");
	exit(0);
}
