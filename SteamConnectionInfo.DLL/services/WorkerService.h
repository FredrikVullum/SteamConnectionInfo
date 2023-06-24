#pragma once
#include "../models/player.h"
#include "../helpers/SharedMemoryProducer.h"
#include "../helpers/PlayerSerializer.h"
#include "../globals/players.h"
#include "../helpers/MessageBoxShower.h"

namespace WorkerService
{
	SharedMemoryProducer producer;
	bool				 running;

	void Run() {
		bool producer_initialized = producer.Initialize("SteamConnectionInfoSharedMemoryRegion", 4096);

		if (!producer_initialized) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to initialize shared memory producer");
			return;
		}

		running = true;

		while (running)
		{
			players_mutex.lock();

			auto currentTime = std::chrono::steady_clock::now();

			for (auto it = players.begin(); it != players.end();) {
				auto& [steam_id, player] = *it;
				auto delta = std::chrono::duration_cast<std::chrono::seconds>(currentTime - player.last_p2p_send);
				auto player_network_id = player.GetUniqueNetworkId();
				auto pings_it = player_pings.find(player_network_id);

				if (delta.count() <= 4) 
				{
					if (player.session_state.m_nRemoteIP && player.session_state.m_nRemotePort) 
					{
						if (player.steam_ip != player.session_state.m_nRemoteIP) 
						{
							player.steam_ip = player.session_state.m_nRemoteIP;
							player.needs_country = true;
						}

						if (player.steam_port != player.session_state.m_nRemotePort) 
						{
							player.steam_port = player.session_state.m_nRemotePort;
						}

						if (pings_it != player_pings.end())
						{
							player.ping = pings_it->second.last_known_ping;
						}
					}

					it++;
				}
				else {
					it = players.erase(it);

					if (pings_it != player_pings.end())
					{
						player_pings.erase(pings_it);
					}
				}
			}

			players_mutex.unlock();

			if (producer.ConsumerIsReady())
			{
				players_mutex.lock();
				std::string json_players = PlayerSerializer::Serialize(players);
				players_mutex.unlock();

				producer.SetData(json_players);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void Stop() {
		running = false;
	}
};