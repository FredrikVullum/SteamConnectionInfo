#pragma once
#include <pcap.h>
#include "../models/player.h"
#include "../globals/players.h"
#include "../helpers/MessageBoxShower.h"

namespace PingMonitorService 
{
	const uint16_t     STUN_REQUEST_SIZE = 56;
	const uint16_t     STUN_RESPONSE_SIZE = 68;

	char			   error_buffer[PCAP_ERRBUF_SIZE];
	struct bpf_program bpf_filter;
	pcap_t*			   pcap_handle;
	pcap_if_t*		   device_list;

	int64_t time_delta(const timeval& start, const timeval& end)
	{
		int64_t startMillis = static_cast<int64_t>(start.tv_sec) * 1000 + static_cast<int64_t>(start.tv_usec) / 1000;
		int64_t endMillis = static_cast<int64_t>(end.tv_sec) * 1000 + static_cast<int64_t>(end.tv_usec) / 1000;

		if (endMillis <= startMillis)
			return 0;

		return endMillis - startMillis;
	}

	static void packet_callback(u_char* user_data, const struct pcap_pkthdr* packet_header, const u_char* packet_data)
	{
		const u_char* ip_header = packet_data + 14;
		const u_char* udp_header = ip_header + 20;

		const uint16_t udp_length = ntohs(*((uint16_t*)(udp_header + 4)));
		const uint16_t udp_payload_size = udp_length - 8;

		const in_addr* in_src_ip = (in_addr*)(ip_header + 12);
		const in_addr* in_dst_ip = (in_addr*)(ip_header + 16);


		const uint32_t src_ip = ntohl(in_src_ip->S_un.S_addr);
		const uint16_t src_port = ntohs(*((uint16_t*)udp_header));

		const uint32_t dst_ip = ntohl(in_dst_ip->S_un.S_addr);
		const uint16_t dst_port = ntohs(*((uint16_t*)(udp_header + 2)));

		timeval current_time = packet_header->ts;

		std::lock_guard<std::mutex> lock(players_mutex);

		if (udp_payload_size == STUN_REQUEST_SIZE)
		{
			uint64_t unique_id = static_cast<uint64_t>(dst_port) << 32 | static_cast<uint64_t>(dst_ip);

			if (unique_id) 
			{
				auto player_ping_info = &player_pings[unique_id];
				player_ping_info->last_stun_send_request = current_time;
			}
		}

		else if (udp_payload_size == STUN_RESPONSE_SIZE) 
		{
			uint64_t unique_id = static_cast<uint64_t>(src_port) << 32 | static_cast<uint64_t>(src_ip);

			if (unique_id) 
			{
				auto player_ping_info = &player_pings[unique_id];

				if (player_ping_info->last_stun_send_request.tv_sec || player_ping_info->last_stun_send_request.tv_usec)
				{
					int64_t ping = time_delta(player_ping_info->last_stun_send_request, current_time);

					if (ping) 
					{
						player_ping_info->last_known_ping = ping;

						player_ping_info->last_n_pings.push_back(ping);
						if (player_ping_info->last_n_pings.size() > 4) 
						{
							player_ping_info->last_n_pings.pop_front();

							int64_t sum = std::accumulate(player_ping_info->last_n_pings.begin(), player_ping_info->last_n_pings.end(), 0);
							int64_t average_ping = sum / static_cast<int64_t>(player_ping_info->last_n_pings.size());

							player_ping_info->average_ping = average_ping;
						}
					}
					
					player_ping_info->last_stun_send_request = {};
				}
			}
		}
	}

	void Run()
	{
		int findalldevs_result = pcap_findalldevs_ex(PCAP_SRC_IF_STRING, nullptr, &device_list, error_buffer);
		if (findalldevs_result != 0) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find any network devices");
			return;
		}

		pcap_if_t* device = nullptr;
		for (pcap_if_t* d = device_list; d != nullptr; d = d->next) {
			if ((d->flags & PCAP_IF_LOOPBACK) != 0 || d->addresses == nullptr) {
				continue;
			}

			for (pcap_addr_t* a = d->addresses; a != nullptr; a = a->next) {
				if (a->addr->sa_family == AF_INET) {
					device = d;
					break;
				}
			}
			if (device != nullptr) {
				break;
			}
		}

		if (device == nullptr) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Found no active network device");
			pcap_freealldevs(device_list);
			return;
		}

		pcap_handle = pcap_open_live(device->name, 65536, 1, 100, error_buffer);
		if (pcap_handle == nullptr) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to open network device");
			pcap_freealldevs(device_list);
			return;
		}

		const char filter[] = "udp and ((udp[8:2] = 0x0001) or (udp[8:2] = 0x0101)) and ((udp[10:2] = 0x0030) or (udp[10:2] = 0x0024))";

		if (pcap_compile(pcap_handle, &bpf_filter, filter, 1, PCAP_NETMASK_UNKNOWN) == -1) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to compile packet filter ");
			pcap_close(pcap_handle);
			pcap_freealldevs(device_list);
			return;
		}

		if (pcap_setfilter(pcap_handle, &bpf_filter) == -1) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to set packet filter for network device");
			pcap_freecode(&bpf_filter);
			pcap_close(pcap_handle);
			pcap_freealldevs(device_list);
			return;
		}

		//after idling for about 30 minutes the cpu usage suddenly spikes 10x and packets are no longer captured? attempted fix
		int pcap_loop_ret = pcap_loop(pcap_handle, -1, packet_callback, NULL);

		pcap_freecode(&bpf_filter);
		pcap_close(pcap_handle);
		pcap_freealldevs(device_list);
		memset(error_buffer, 0, PCAP_ERRBUF_SIZE);

		if (pcap_loop_ret == -1) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Packet monitoring loop terminated with an error");
		}
	}

	void Stop() {
		if(pcap_handle)
			pcap_breakloop(pcap_handle);
	}
}