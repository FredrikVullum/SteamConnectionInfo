#pragma once
#include <map>
#include <mutex>
#include "../models/player.h"
#include "../models/ping_info.h"

inline std::map<CSteamID, Player>	players;
inline std::map<uint64_t, PingInfo>	player_pings;

inline std::mutex					players_mutex;