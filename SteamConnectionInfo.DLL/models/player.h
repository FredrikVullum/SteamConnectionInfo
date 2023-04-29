#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <deque>
#include <WinSock2.h>

typedef std::chrono::steady_clock steady_clock;
typedef steady_clock::time_point timestamp;

class Player {
public:
	timestamp			 last_seen;
	timeval				 last_stun_send_request;
	timeval				 last_stun_recv_response;
	uint32_t			 steam_ip;
	uint16_t			 steam_port;
	std::string			 steam_name;
	uint64_t			 steam_id;
	uint8_t				 steam_relay;
	int64_t			     ping;
	std::string			 country;
	
	Player() {
		last_stun_send_request.tv_sec = 0;
		last_stun_send_request.tv_usec = 0;

		last_stun_recv_response.tv_sec = 0;
		last_stun_recv_response.tv_usec = 0;

		steam_ip = 0;
		steam_port = 0;
		steam_name = "";
		steam_id = 0;
		steam_relay = 0;
		ping = 0;
		country = "";
	}
};