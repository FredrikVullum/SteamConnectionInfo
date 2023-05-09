#pragma once
#include <cstdint>
#include <string>
#include <chrono>
#include <map>
#include <WinSock2.h>

#include "../steam/ISteamNetworking.h"

typedef std::chrono::steady_clock steady_clock;
typedef steady_clock::time_point timestamp;

class Player {
public:
	P2PSessionState_t    session_state;

	timestamp			 last_p2p_send;
	timeval				 last_stun_send_request;
	timeval				 last_stun_recv_response;

	bool			     needs_country;
	uint32_t			 steam_ip;
	uint16_t			 steam_port;
	std::string			 steam_name;
	int64_t			     ping;
	std::string			 country;
	
	Player() {
		needs_country = false;
		last_stun_send_request.tv_sec = 0;
		last_stun_send_request.tv_usec = 0;
		last_stun_recv_response.tv_sec = 0;
		last_stun_recv_response.tv_usec = 0;
		session_state = {};
		steam_ip = 0;
		steam_port = 0;
		steam_name = "";
		ping = 0;
		country = "";
	}
};