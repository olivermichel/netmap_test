#include <iostream>

#include <sys/mman.h>
#include <net/if.h>
#include <csignal>

#include <netinet/if_ether.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>

#include <poll.h>

#include <chrono>

#include "netmap_iface.h"

bool stop = false;

void sig_handler(int sig)
{
	if (sig == SIGINT)
		stop = true;
}

int main()
{
	signal(SIGINT, sig_handler);

	netmap_iface nm("enp6s0f1");

	std::cout << "receiver: " << nm.count_tx_rings() << " tx rings, " << nm.count_rx_rings()
			<< " rx rings" << std::endl;

	unsigned long long pkt_count = 0;

	while (!stop) {

		struct pollfd pfd[1];
		pfd[0].fd = nm.fd();
		pfd[0].events = POLLIN;

		int ret = poll(pfd, 1, 1000);

		if (ret < 0) {
			std::cerr << "poll()" << std::endl;
		} else if (ret == 0) {
			continue; // timeout
		}

		auto start = std::chrono::high_resolution_clock::now();

		unsigned head = nm.rx_rings[0]->head;
		unsigned tail = nm.rx_rings[0]->tail;

		for (; head != tail; head = nm_ring_next(nm.rx_rings[0], head)) {
			// std::cout << "rx" << std::endl;
			struct netmap_slot* slot = nm.rx_rings[0]->slot + head;
			char* rx_buf = NETMAP_BUF(nm.rx_rings[0], slot->buf_idx);

			auto eth = (struct ether_header*) rx_buf;
			if (ntohs(eth->ether_type) == ETH_P_IP) {
				auto ip = (struct iphdr*) (rx_buf + sizeof(ether_header));
				if (ip->protocol == IPPROTO_UDP) {
					auto udp = (struct udphdr*) (rx_buf + sizeof(ether_header) + sizeof(iphdr));

					if (pkt_count++ % 1000000 == 0) {
						auto end = std::chrono::high_resolution_clock::now();
						auto dur = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
						double time_s = (double) dur.count() / 1000000;

						std::cout << pkt_count << ": " << time_s << std::endl;
					}
				}
			}
		}
		nm.rx_rings[0]->cur = nm.rx_rings[0]->head = head;
	}

	return 0;
}