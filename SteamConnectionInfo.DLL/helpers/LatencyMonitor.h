#pragma once
#include "../models/player.h"
#include <pcap.h>
#include <WinSock2.h>

namespace LatencyMonitor {
    const uint16_t       STUN_REQUEST_SIZE  = 56;
    const uint16_t       STUN_RESPONSE_SIZE = 68;

    std::vector<Player>* players;
    std::mutex*          players_mutex;

    static void packet_callback(u_char* user_data, const struct pcap_pkthdr* packet_header, const u_char* packet_data)
    {
        const u_char* ip_header  = packet_data + 14;
        const u_char* udp_header = ip_header + 20;

        uint16_t udp_length = ntohs(*((uint16_t*)(udp_header + 4)));
        uint16_t udp_payload_size = udp_length - 8;

        in_addr* in_src_ip = ( in_addr*)(ip_header + 12);
        in_addr* in_dst_ip = (in_addr*)(ip_header + 16);

  
        uint32_t src_ip =   ntohl(in_src_ip->S_un.S_addr);
        uint16_t src_port = ntohs(*((uint16_t*)udp_header));

        uint32_t dst_ip =   ntohl(in_dst_ip->S_un.S_addr);
        uint16_t dst_port = ntohs(*((uint16_t*)(udp_header + 2)));


        char src_ip_str[INET_ADDRSTRLEN];
        char dst_ip_str[INET_ADDRSTRLEN];

        inet_ntop(AF_INET, in_src_ip, src_ip_str, sizeof(src_ip_str));
        inet_ntop(AF_INET, in_dst_ip, dst_ip_str, sizeof(dst_ip_str));

        timeval current_time = packet_header->ts;

        if (udp_payload_size == STUN_REQUEST_SIZE) {
            players_mutex->lock();
            for (auto& player : *players) {
                if (player.steam_ip == dst_ip && player.steam_port == dst_port) {
                    player.last_stun_send_request.tv_sec = current_time.tv_sec;
                    player.last_stun_send_request.tv_usec = current_time.tv_usec;
                    break;
                }
            }
            players_mutex->unlock();
            return;
        }
        
        if (udp_payload_size == STUN_RESPONSE_SIZE) {

            players_mutex->lock();
            for (auto& player : *players) 
            {
                if (player.steam_ip == src_ip && player.steam_port == src_port && player.last_stun_send_request.tv_sec) {
                    player.last_stun_recv_response.tv_sec = current_time.tv_sec;
                    player.last_stun_recv_response.tv_usec = current_time.tv_usec;

                    int64_t time_diff_ms = (current_time.tv_sec - player.last_stun_send_request.tv_sec) * 1000 + (current_time.tv_usec - player.last_stun_send_request.tv_usec) / 1000;
 
                    player.ping = time_diff_ms;

                    player.last_stun_send_request.tv_sec = 0;
                    player.last_stun_send_request.tv_usec = 0;
                    break;
                }
            }
            players_mutex->unlock();
        }
    }

    void Run(std::vector<Player>* players_to_set, std::mutex* players_mutex_to_set)
    {
        players =       players_to_set;
        players_mutex = players_mutex_to_set;

        char error_buffer[PCAP_ERRBUF_SIZE];
        pcap_t* pcap_handle;
        pcap_if_t* device_list;

        int findalldevs_result = pcap_findalldevs_ex(PCAP_SRC_IF_STRING, nullptr, &device_list, error_buffer);
        if (findalldevs_result != 0) {
            std::cout << "failed to get list of devices for capture" << std::endl;
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
            std::cout << "found no running devices for capture" << std::endl;
            pcap_freealldevs(device_list);
            return;
        }

        pcap_handle = pcap_open_live(device->name, 65536, 1, 1000, error_buffer);
        if (pcap_handle == nullptr) {
            std::cout << "failed to open device for capture" << std::endl;
            pcap_freealldevs(device_list);
            return;
        }

   
        const char filter[] = "udp and ((udp[8:2] = 0x0001) or (udp[8:2] = 0x0101)) and ((udp[10:2] = 0x0030) or (udp[10:2] = 0x0024))";

        struct bpf_program bpf_filter;
        if (pcap_compile(pcap_handle, &bpf_filter, filter, 1, PCAP_NETMASK_UNKNOWN) == -1) {

            std::cout << "bad filter" << std::endl;
            pcap_close(pcap_handle);
            pcap_freealldevs(device_list);
            return;
        }

        if (pcap_setfilter(pcap_handle, &bpf_filter) == -1) {
            std::cout << "failed to apply filter" << std::endl;
            pcap_freecode(&bpf_filter);
            pcap_close(pcap_handle);
            pcap_freealldevs(device_list);
            return;
        }

        while (true) {
            int ret = pcap_loop(pcap_handle, -1, packet_callback, NULL);
            if (ret == -1) {
                std::cout << "pcap_loop error: " << pcap_geterr(pcap_handle) << std::endl;
            }
        }

        pcap_freecode(&bpf_filter);
        pcap_close(pcap_handle);
        pcap_freealldevs(device_list);
    }
}
