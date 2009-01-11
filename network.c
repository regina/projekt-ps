#include <unistd.h>
#include "network.h"
#include "main.h"

extern struct aids_global_conf aids_conf;

void network_usage(const char *dev, struct network_traffic *traffic)
{
	pcap_t *handle;
	struct network_stats stats;
	struct pcap_pkthdr header_start, header_finish;
	double seconds;


	seconds = 0.0;

	handle = pcap_open_live(dev, 99999, 1, 1000, errbuf);
	if (handle == NULL)
	{
		fprintf(stderr, "Couldn't open device %s: %s\n", dev, errbuf);
		return;
	}

	memset(&stats, 0, sizeof(struct network_stats));

	/*
	 * time and size of first received packet
	 */
	pcap_next(handle, &header_start);
	stats.start = header_start.ts;
	stats.length += header_start.len;

	pcap_loop(handle, 98, got_packet, (u_char *)&stats);

	/*
	 * time and size of last received packet
	 */
	pcap_next(handle, &header_finish);
	stats.finish = header_finish.ts;
	stats.length += header_finish.len;

	seconds = (stats.finish.tv_sec - stats.start.tv_sec) * 1000000 + (stats.finish.tv_usec - stats.start.tv_usec);
	seconds /= 1000000;
	pcap_close(handle);

	/*
	 * return kb/s
	 */
	traffic -> in = stats.length / seconds / 1000.0;
	traffic -> out = stats.length / seconds / 1000.0;
}

void got_packet(u_char *args, const struct pcap_pkthdr *header, const u_char *packet)
{
	struct network_stats *stats;

	stats = (struct network_stats *)args;
	stats -> length += header -> len;
}

void aids_gather_network(void)
{
	struct network_traffic traffic;

	while (1)
	{
		network_usage("en1", &traffic);
		sleep(aids_conf.network_timeout);
	}
}
