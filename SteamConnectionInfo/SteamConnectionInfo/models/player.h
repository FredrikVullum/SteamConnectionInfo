#pragma once
#include <cstdint>
#include <string>
#include <chrono>

typedef std::chrono::steady_clock steady_clock;
typedef steady_clock::time_point time_point;

class Player {
public:
	time_point	time_point;
	uint32_t steam_ip;
	uint16_t steam_port;
	std::string steam_name;
	uint64_t steam_id;
	uint8_t steam_relay;

	Player() {
		steam_ip = 0;
		steam_port = 0;
		steam_name = "";
		steam_id = 0;
		steam_relay = 0;
	}
};