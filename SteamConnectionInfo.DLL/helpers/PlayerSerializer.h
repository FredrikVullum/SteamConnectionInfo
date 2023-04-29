#pragma once
#include <nlohmann/json.hpp>
#include <vector>
#include "../models/player.h"

using json = nlohmann::json;

class PlayerSerializer {
public:
	static std::string Serialize(const Player& player) {
		const json jsonObject = json{
			{"steam_ip", player.steam_ip},
			{"steam_port", player.steam_port},
			{"steam_name", player.steam_name},
			{"steam_id", player.steam_id},
			{"steam_relay", player.steam_relay},
			{"country", player.country},
			{"ping", player.ping}
		};
		return jsonObject.dump();
	}
	static std::string SerializeMany(const std::vector<Player>& players) {
		json jsonArray = json::array();
		for (const Player& player : players) {
			json jsonObject = json{
				{"steam_ip", player.steam_ip},
				{"steam_port", player.steam_port},
				{"steam_name", player.steam_name},
				{"steam_id", player.steam_id},
				{"steam_relay", player.steam_relay},
				{"country", player.country},
				{"ping", player.ping}
			};
			jsonArray.push_back(jsonObject);
		}
		return jsonArray.dump();
	}
	static std::vector<Player> DeserializeMany(const std::string& json_str) {
		std::vector<Player> players;
		json jsonArray = json::parse(json_str);
		for (const json& jsonObject : jsonArray) {
			Player player;
			jsonObject.at("steam_ip").get_to(player.steam_ip);
			jsonObject.at("steam_port").get_to(player.steam_port);
			jsonObject.at("steam_name").get_to(player.steam_name);
			jsonObject.at("steam_id").get_to(player.steam_id);
			jsonObject.at("steam_relay").get_to(player.steam_relay);
			jsonObject.at("country").get_to(player.country);
			jsonObject.at("ping").get_to(player.ping);
			players.push_back(player);
		}
		return players;
	}

	static Player Deserialize(const std::string& json_str) {
		json jsonObject = json::parse(json_str);

		Player player;
		jsonObject.at("steam_ip").get_to(player.steam_ip);
		jsonObject.at("steam_port").get_to(player.steam_port);
		jsonObject.at("steam_name").get_to(player.steam_name);
		jsonObject.at("steam_id").get_to(player.steam_id);
		jsonObject.at("steam_relay").get_to(player.steam_relay);
		jsonObject.at("country").get_to(player.country);
		jsonObject.at("ping").get_to(player.ping);
		return player;
	}
};