#pragma once
#include <nlohmann/json.hpp>
#include "../models/player.h"

using json = nlohmann::json;

class PlayerSerializer {
public:
	static std::string Serialize(const std::map<CSteamID, Player>& players) {
		json jsonArray = json::array();

		for (const auto& [steam_id, player] : players) {
			if (player.ping == 0 || player.country.empty() || player.steam_name.empty())
				continue;
			json jsonObject = json{
				{"steam_ip", player.steam_ip},
				{"steam_port", player.steam_port},
				{"steam_name", player.steam_name},
				{"steam_id", steam_id.ConvertToUint64()},
				{"country", player.country},
				{"ping", player.ping}
			};
			jsonArray.push_back(jsonObject);
		}
		return jsonArray.dump();
	}
};