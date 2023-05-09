#pragma once
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <future>

#include "../models/player.h"

using json = nlohmann::json;

namespace IpToCountryService
{
	std::mutex mutex;
	bool running;

	static size_t ResponseCallback(void* contents, size_t size, size_t nmemb, void* userp)
	{
		mutex.lock();
		Player* player = static_cast<Player*>(userp);
		std::string response(reinterpret_cast<char*>(contents), size * nmemb);
		try {
			json j = json::parse(response);
			player->country = j["country"];
			player->needs_country = false;
		}
		catch (const std::exception& ex) {
			player->country = "Unknown";
		}
		mutex.unlock();
		return size * nmemb;
	}

	void ResolvePlayerCountry(Player& player)
	{
		mutex.lock();
		const std::string ip_address_str = std::to_string((player.steam_ip >> 24) & 0xFF) + "." +
			std::to_string((player.steam_ip >> 16) & 0xFF) + "." +
			std::to_string((player.steam_ip >> 8) & 0xFF) + "." +
			std::to_string(player.steam_ip & 0xFF);
		mutex.unlock();

		const std::string url = "http://ip-api.com/json/" + ip_address_str;

		if (running) {
			CURL* curl_handle = curl_easy_init();
			if (curl_handle) {
				curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
				curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, ResponseCallback);
				mutex.lock();
				curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, &player);
				mutex.unlock();
				curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 1);
				curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 1);
				auto res = curl_easy_perform(curl_handle);
				if (res != CURLE_OK) {
					mutex.lock();
					player.country = "Unknown";
					mutex.unlock();
				}
				curl_easy_cleanup(curl_handle);
			}
		}
	}

	void Run()
	{
		running = true;

		while (running) {
			std::vector<std::future<void>> futures;

			mutex.lock();
			for (auto& [steam_id, player] : players)
			{
				if (!player.needs_country)
					continue;

				auto future = std::async(std::launch::async, ResolvePlayerCountry, std::ref(player));

				futures.push_back(std::move(future));
			}
			mutex.unlock();

			for (auto& f : futures) {
				f.wait();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(600));
		}
	}

	void Stop() {
		running = false;
	}
};