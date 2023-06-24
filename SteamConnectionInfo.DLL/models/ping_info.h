#pragma once
#include <cstdint>
#include <deque>
class PingInfo {
public:
	std::deque<int64_t> last_n_pings;
	timeval				last_stun_send_request;
	int64_t				last_known_ping;
	int64_t				average_ping;

	PingInfo() 
	{
		last_stun_send_request = {};
		last_known_ping = 0;
		average_ping = 0;
	}
};