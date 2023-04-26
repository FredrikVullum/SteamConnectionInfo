#include "helpers/PlayerSerializer.h"
#include "helpers/VirtualMethodTableHooker.h"
#include "helpers/SignatureFinder.h"
#include "helpers/SharedMemoryProducer.h"
#include "helpers/IpToCountryResolver.h"

#include "hooks/GetFriendPersonaName.h"
#include "hooks/SendP2PPacket.h"

namespace Hooks 
{
	std::unique_ptr<VirtualMethodTableHooker> SteamNetworkingHooker = nullptr;
	SendP2PPacket_t		 SendP2PPacket = nullptr;

	std::unique_ptr<VirtualMethodTableHooker> SteamFriendsHooker = nullptr;
	GetFriendPersonaName_t		 GetFriendPersonaName = nullptr;

	std::vector<Player> players;
	std::mutex playersMutex;

	SharedMemoryProducer producer;

	void Run() {
		if (!producer.Initialize("SteamConnectionInfoSharedMemoryRegion", 4096))
			return;

		while (true) 
		{
			auto current_time = std::chrono::steady_clock::now();

			playersMutex.lock();
			for (auto it = players.begin(); it != players.end();) 
			{
				auto time_delta = std::chrono::duration_cast<std::chrono::seconds>(current_time - it->time_point);

				if (time_delta.count() > 4) {
					it = players.erase(it);
					continue;
				}

				if (it->country.empty()) {
					it->country = IpToCountryResolver::Resolve(it->steam_ip);
				}

				++it;

				/*
				if player's country is in the country filter... remove and Close the p2p session with this player
				country filter can be a file that contains countries that should be removed?
	
				check player's latency.. if it is higher than specified in latency filter..  remove and Close the p2p session with this player
				latency filter can be in a file that contains the maximum latency? maybe the same one as country filter

				these filters can just be additional .json files that contain countries in one file and a latency max in the other
				maybe I try can to read appsettings.json to check if filter for country or latency is enabled
				*/	
			}
			playersMutex.unlock();

			if (producer.ConsumerIsReady()) 
			{
				playersMutex.lock();
				std::string playersStr = PlayerSerializer::SerializeMany(players);
				producer.SetData(playersStr);
				playersMutex.unlock();
			}

			Sleep(1000);
		}
	} 

	void Initialize() 
	{
		DWORD steamclient = 0;

		while (!steamclient) {
			steamclient = reinterpret_cast<DWORD>(GetModuleHandleA("steamclient.dll"));
			Sleep(1000);
		}

		DWORD CClientNetworkingVTableInstance = SignatureFinder::Find("steamclient.dll", "C7 07 ? ? ? ? C7 47 ? ? ? ? ? E8 ? ? ? ? 8B 47 0C") + 2;
		DWORD CUserFriendsVTableInstance = SignatureFinder::Find("steamclient.dll", "C7 07 ? ? ? ? C7 47 ? ? ? ? ? 8B 40 24") + 2;

		SteamNetworkingHooker = std::make_unique<VirtualMethodTableHooker>(*(DWORD**)CClientNetworkingVTableInstance);
		if (!SteamNetworkingHooker)
			return;

		SteamFriendsHooker = std::make_unique<VirtualMethodTableHooker>(*(DWORD**)CUserFriendsVTableInstance);
		if (!SteamFriendsHooker)
			return;

		SendP2PPacket = (SendP2PPacket_t)SteamNetworkingHooker->Hook(0, SendP2PPacket_Hook);
		if (!SendP2PPacket)
			return;

		GetFriendPersonaName = (GetFriendPersonaName_t)SteamFriendsHooker->Hook(13, GetFriendPersonaName_Hook);
		if (!GetFriendPersonaName)
			return;

		Run();
	}

	ISteamFriends* SteamFriends = nullptr;
	const char* __fastcall GetFriendPersonaName_Hook(ISteamFriends* thisptr, void* edx, CSteamID steamIDFriend) 
	{
		if (thisptr && thisptr != SteamFriends) {
			SteamFriends = thisptr;
		}
		return GetFriendPersonaName(thisptr, steamIDFriend);
	}

	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* CClientNetworkingAPIInstance, void* edx, CSteamID steamId, const void* pubData, unsigned int cubData, EP2PSend eP2PSendType, int nChannel)
	{
		auto original = SendP2PPacket(CClientNetworkingAPIInstance, steamId, pubData, cubData, eP2PSendType, nChannel);

		if (!steamId.IsValid() || !SteamFriends)
			return original;

		std::string steam_name = SteamFriends->GetFriendPersonaName(steamId);
		if (steam_name.empty())
			return original;

		P2PSessionState_t session;

		bool sessionResult = CClientNetworkingAPIInstance->GetP2PSessionState(steamId, &session);
		if (!sessionResult)
			return original;

		bool sessionIsErrorFree = session.m_eP2PSessionError == 0;
		if (!sessionIsErrorFree)
			return original;

		bool sessionConnecting = session.m_bConnecting;
		if (sessionConnecting)
			return original;

		bool sessionActive = session.m_bConnectionActive;
		if (!sessionActive)
			return original;

		uint64_t steam_id = steamId.ConvertToUint64();

		bool playerAlreadyExists = false;

		playersMutex.lock();
		for (auto& player : players) {
			if (player.steam_id != steam_id)
				continue;

			playerAlreadyExists = true;

			player.time_point = steady_clock::now();
			player.steam_ip = session.m_nRemoteIP;
			player.steam_port = session.m_nRemotePort;
			player.steam_name = steam_name;
			player.steam_relay = session.m_bUsingRelay;
		}
		playersMutex.unlock();

		if (!playerAlreadyExists) {
			Player playerToAdd;
			playerToAdd.time_point = steady_clock::now();
			playerToAdd.steam_ip = session.m_nRemoteIP;
			playerToAdd.steam_port = session.m_nRemotePort;
			playerToAdd.steam_name = steam_name;
			playerToAdd.steam_id = steam_id;
			playerToAdd.steam_relay = session.m_bUsingRelay;
			playersMutex.lock();
			players.push_back(playerToAdd);
			playersMutex.unlock();
		}

		return original;
	}

	void Restore() {
		if (SteamNetworkingHooker)
			SteamNetworkingHooker->UnhookAll();

		if (SteamFriendsHooker)
			SteamFriendsHooker->UnhookAll();

		producer.Destroy();

		FreeLibraryAndExitThread(GetModuleHandle(NULL), 0);
	}
}

BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)  // reserved
{
	if (reason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(handle);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Hooks::Initialize, NULL, NULL, NULL);
	}
	else if (reason == DLL_PROCESS_DETACH) {
		if (!reserved) {
			Hooks::Restore();
		}
	}
	return TRUE;
}