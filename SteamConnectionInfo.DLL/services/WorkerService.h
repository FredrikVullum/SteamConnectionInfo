#pragma once
#include "../models/player.h"
#include "../helpers/SharedMemoryProducer.h"
#include "../helpers/PlayerSerializer.h"
#include "../globals/players.h"

namespace WorkerService
{
	SharedMemoryProducer producer;
	std::mutex			 mutex;
	bool				 running;

	void Run() {
		bool producer_initialized = producer.Initialize("SteamConnectionInfoSharedMemoryRegion", 4096);

		running = true;

		while (producer_initialized && running)
		{
			auto currentTime = std::chrono::steady_clock::now();

			mutex.lock();
			for (auto it = players.begin(); it != players.end();) {
				auto& [steam_id, player] = *it;
				auto delta = std::chrono::duration_cast<std::chrono::seconds>(currentTime - player.last_p2p_send);

				if (delta.count() <= 5) {
					if (player.session_state.m_nRemoteIP && player.steam_ip != player.session_state.m_nRemoteIP) {
						player.steam_ip = player.session_state.m_nRemoteIP;
						player.needs_country = true;
					}
					if (player.session_state.m_nRemotePort && player.steam_port != player.session_state.m_nRemotePort) {
						player.steam_port = player.session_state.m_nRemotePort;
					}
					it++;
				}
				else {
					it = players.erase(it);
				}
			}
			mutex.unlock();

			if (producer.ConsumerIsReady())
			{
				mutex.lock();
				producer.SetData(PlayerSerializer::Serialize(players));
				mutex.unlock();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Stop() {
		running = false;
	}
};