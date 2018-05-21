#include <iostream>

#include <sys/mman.h>
#include <net/if.h>

#include <poll.h>

#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include "netmap_iface.h"

bool stop = false;

void sig_handler(int sig)
{
	if (sig == SIGINT)
		stop = true;
}

int main(int argc, char** argv)
{
	signal(SIGINT, sig_handler);

	netmap_iface nm("enp6s0f0");

	char pkt_buf[1500] = { '\0' };
	auto eth = (struct ether_header*) pkt_buf;
	unsigned short pkt_len = sizeof(ether_header);
	eth->ether_type  = htons(ETH_P_IP);
	eth->ether_shost[0] = 0x68;
	eth->ether_shost[1] = 0x05;
	eth->ether_shost[2] = 0xCA;
	eth->ether_shost[3] = 0x39;
	eth->ether_shost[4] = 0x84;
	eth->ether_shost[5] = 0x94;
	eth->ether_dhost[0] = 0x68;
	eth->ether_dhost[1] = 0x05;
	eth->ether_dhost[2] = 0xCA;
	eth->ether_dhost[3] = 0x39;
	eth->ether_dhost[4] = 0x84;
	eth->ether_dhost[5] = 0x95;


	auto ip = (struct iphdr*) (pkt_buf + pkt_len);
	pkt_len += sizeof(iphdr);
	ip->version  = 4;
	ip->ihl      = 5;
	ip->tos      = 0;
	ip->tot_len  = sizeof(iphdr) + sizeof(udphdr);
	ip->id       = 0;
	ip->frag_off = 0;
	ip->ttl      = 255;
	ip->protocol = IPPROTO_UDP;
	ip->check    = 0;
	ip->saddr    = inet_addr("0.0.0.0");
	ip->daddr    = inet_addr("0.0.0.0");


	auto udp = (struct udphdr*) (pkt_buf + pkt_len);
	pkt_len += sizeof(udphdr);
	udp->uh_sport = 0;
	udp->uh_dport = 0;
	udp->uh_ulen  = 8;
	udp->uh_sum   = 0;

	while (!stop) {

		struct pollfd pfd[1];
		pfd[0].fd = nm.fd();
		pfd[0].events = POLLOUT;

		int ret = poll(pfd, 1, 1000);

		if (ret < 0) {
			std::cerr << "poll()" << std::endl;
		} else if (ret == 0) {
			continue; // timeout
		}

		unsigned head = nm.tx_rings[0]->head;
		unsigned tail = nm.tx_rings[0]->tail;

		for ( ; head != tail; head = nm_ring_next(nm.tx_rings[0], head)) {
			// std::cout << "tx" << std::endl;
			struct netmap_slot* slot = nm.tx_rings[0]->slot + head;
			char* tx_buf = NETMAP_BUF(nm.tx_rings[0], slot->buf_idx);
			memcpy(tx_buf, pkt_buf, pkt_len);
			slot->len = pkt_len;
		}

		nm.tx_rings[0]->cur = nm.tx_rings[0]->head = head;
	}

	return 0;

}
