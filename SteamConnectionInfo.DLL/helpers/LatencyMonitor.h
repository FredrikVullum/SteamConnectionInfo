#pragma once
#include "../models/player.h"
#include <pcap.h>
#include <WinSock2.h>

namespace LatencyMonitor {
    std::vector<Player>* playersPtr;
    std::mutex* playersMutexPtr;
    char deviceIpStr[INET_ADDRSTRLEN];

    static void OnPacketArrival(u_char* user, const struct pcap_pkthdr* packetHeader, const u_char* packetData)
    {
        const u_char* ipHeader = packetData + 14; // Skip the Ethernet header
        in_addr* sourceIP = (in_addr*)(ipHeader + 12); // Get the source IP address
        in_addr* destIP = (in_addr*)(ipHeader + 16); // Get the destination IP address
        char sourceIPStr[INET_ADDRSTRLEN];
        char destIPStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, sourceIP, sourceIPStr, sizeof(sourceIPStr)); // Convert the source IP to a string
        inet_ntop(AF_INET, destIP, destIPStr, sizeof(destIPStr)); // Convert the destination IP to a string
        printf("%s -> %s\n", sourceIPStr, destIPStr); // Print the source and destination IP addresses
    }

    void Run(std::vector<Player>* playersPtrToSet, std::mutex* playersMutexPtrToSet)
    {
        playersPtr = playersPtrToSet;
        playersMutexPtr = playersMutexPtrToSet;

        char errbuf[PCAP_ERRBUF_SIZE];
        pcap_t* pcapHandle;
        pcap_if_t* deviceList;
        int result = pcap_findalldevs_ex(PCAP_SRC_IF_STRING, nullptr, &deviceList, errbuf);
        if (result != 0) {
            std::cout << "failed to get list of devices for capture" << std::endl;
            return;
        }

        pcap_if_t* device = nullptr;
        for (pcap_if_t* d = deviceList; d != nullptr; d = d->next) {
            if ((d->flags & PCAP_IF_LOOPBACK) != 0 || d->addresses == nullptr) {
                continue;
            }

            for (pcap_addr_t* a = d->addresses; a != nullptr; a = a->next) {
                if (a->addr->sa_family == AF_INET) {
                    device = d;
                    in_addr deviceIpAddr = ((struct sockaddr_in*)(a->addr))->sin_addr;
                    inet_ntop(AF_INET, &deviceIpAddr, deviceIpStr, INET_ADDRSTRLEN);
                    break;
                }
            }
            if (device != nullptr) {
                break;
            }
        }

        if (device == nullptr) {
            std::cout << "found no running devices for capture" << std::endl;
            pcap_freealldevs(deviceList);
            return;
        }

        pcapHandle = pcap_open_live(device->name, 65536, 1, 1000, errbuf);
        if (pcapHandle == nullptr) {
            std::cout << "failed to open device for capture" << std::endl;
            pcap_freealldevs(deviceList);
            return;
        }

        //monitor only stun packets with message length of 24
        const char filterStr[] = "udp and udp[10:2] = 0x0024";

        struct bpf_program filter;
        if (pcap_compile(pcapHandle, &filter, filterStr, 1, PCAP_NETMASK_UNKNOWN) == -1) {

            std::cout << "bad filter" << std::endl;
            pcap_close(pcapHandle);
            pcap_freealldevs(deviceList);
            return;
        }

        // Apply the filter to the device
        if (pcap_setfilter(pcapHandle, &filter) == -1) {
            std::cout << "failed to apply filter" << std::endl;
            pcap_freecode(&filter);
            pcap_close(pcapHandle);
            pcap_freealldevs(deviceList);
            return;
        }

        std::cout << "[" << deviceIpStr << "] " << device->description << std::endl;

        while (true) {
            int ret = pcap_loop(pcapHandle, -1, OnPacketArrival, NULL);
            if (ret == -1) {
                std::cout << "pcap_loop error: " << pcap_geterr(pcapHandle) << std::endl;
            }
        }

        pcap_freecode(&filter);
        pcap_close(pcapHandle);
        pcap_freealldevs(deviceList);
    }
}
