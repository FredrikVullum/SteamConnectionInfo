#include "helpers/PlayerSerializer.h"
#include "helpers/VirtualMethodTableHooker.h"
#include "helpers/SignatureFinder.h"
#include "helpers/SharedMemoryProducer.h"
#include "helpers/IpToCountryResolver.h"
#include "helpers/LatencyMonitor.h"

#include "hooks/GetPersonaName.h"
#include "hooks/SendP2PPacket.h"

namespace Hooks 
{
	std::unique_ptr<VirtualMethodTableHooker> steamNetworkingHooker = nullptr;
	SendP2PPacket_t							  SendP2PPacket = nullptr;

	std::unique_ptr<VirtualMethodTableHooker> steamFriendsHooker = nullptr;
	GetPersonaName_t						  GetPersonaName = nullptr;

	std::vector<Player> players;
	std::mutex playersMutex;

	SharedMemoryProducer producer;

	void PacketMonitoringThread() {
		LatencyMonitor::Run(&players, &playersMutex);
	}
	
	void Run() {
		if (!producer.Initialize("SteamConnectionInfoSharedMemoryRegion", 4096))
			return;

		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)PacketMonitoringThread, NULL, NULL, NULL);
		
		while (true) 
		{
			auto currentTime = std::chrono::steady_clock::now();

			playersMutex.lock();
			for (auto it = players.begin(); it != players.end();) 
			{
				auto delta = std::chrono::duration_cast<std::chrono::seconds>(currentTime - it->last_seen);

				if (delta.count() > 6) {
					it = players.erase(it);
					continue;
				}

				if (it->country.empty()) {
					it->country = IpToCountryResolver::Resolve(it->steam_ip);
				}

				++it;
			}
			playersMutex.unlock();

			if (producer.ConsumerIsReady()) 
			{
				playersMutex.lock();
				std::string playersAsJsonStr = PlayerSerializer::SerializeMany(players);
				playersMutex.unlock();

				producer.SetData(playersAsJsonStr);
			}

			Sleep(1000);
		}
	} 

	void Initialize() 
	{
		HMODULE steamClientModuleHandle = 0;

		while (!steamClientModuleHandle) {
			steamClientModuleHandle = GetModuleHandleA("steamclient.dll");
			Sleep(250);
		}

		DWORD CClientNetworkingVTableInstance = SignatureFinder::Find(steamClientModuleHandle,"C7 07 ? ? ? ? C7 47 ? ? ? ? ? E8 ? ? ? ? 8B 47 0C") + 2;

		DWORD CUserFriendsVTableInstance = SignatureFinder::Find(steamClientModuleHandle,"C7 07 ? ? ? ? C7 47 ? ? ? ? ? 8B 40 24") + 2;

		steamNetworkingHooker = std::make_unique<VirtualMethodTableHooker>(*(DWORD**)CClientNetworkingVTableInstance);
		if (!steamNetworkingHooker)
			return;

		steamFriendsHooker = std::make_unique<VirtualMethodTableHooker>(*(DWORD**)CUserFriendsVTableInstance);
		if (!steamFriendsHooker)
			return;

		
		SendP2PPacket = (SendP2PPacket_t)steamNetworkingHooker->Hook(0, SendP2PPacket_Hook);
		if (!SendP2PPacket)
			return;

		GetPersonaName = (GetPersonaName_t)steamFriendsHooker->Hook(0, GetPersonaName_Hook);
		if (!GetPersonaName)
			return;

		Run();
	}

	ISteamFriends* SteamFriends = nullptr;

	const char* __fastcall GetPersonaName_Hook(ISteamFriends* thisptr)
	{
		if (thisptr && thisptr != SteamFriends) {
			SteamFriends = thisptr;
		}
		return GetPersonaName(thisptr);
	}

	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* SteamNetworking, void* edx, CSteamID steamId, const void* pubData, unsigned int cubData, EP2PSend eP2PSendType, int nChannel)
	{
		auto result = SendP2PPacket(SteamNetworking, steamId, pubData, cubData, eP2PSendType, nChannel);

		if (!steamId.IsValid())
			return result;

		if (!SteamFriends)
			return result;

		std::string playerSteamName = SteamFriends->GetFriendPersonaName(steamId);
		
		if (playerSteamName.empty())
			return result;

		P2PSessionState_t session;

		bool sessionResult = SteamNetworking->GetP2PSessionState(steamId, &session);
		if (!sessionResult)
			return result;

		bool sessionIsErrorFree = session.m_eP2PSessionError == 0;
		if (!sessionIsErrorFree)
			return result;

		bool sessionConnecting = session.m_bConnecting;
		if (sessionConnecting)
			return result;

		bool sessionActive = session.m_bConnectionActive;
		if (!sessionActive)
			return result;

		uint64_t playerSteamId = steamId.ConvertToUint64();

		bool playerAlreadyExists = false;

		playersMutex.lock();
		for (auto& player : players) 
		{
			if (player.steam_id != playerSteamId)
				continue;

			playerAlreadyExists = true;

			player.last_seen = steady_clock::now();
			player.steam_ip = session.m_nRemoteIP;
			player.steam_port = session.m_nRemotePort;
			player.steam_name = playerSteamName;
			player.steam_relay = session.m_bUsingRelay;
		}
		playersMutex.unlock();

		if (!playerAlreadyExists) 
		{
			Player playerToAdd;
			playerToAdd.last_seen = steady_clock::now();
			playerToAdd.steam_ip = session.m_nRemoteIP;
			playerToAdd.steam_port = session.m_nRemotePort;
			playerToAdd.steam_name = playerSteamName;
			playerToAdd.steam_id = playerSteamId;
			playerToAdd.steam_relay = session.m_bUsingRelay;

			playersMutex.lock();
			players.push_back(playerToAdd);
			playersMutex.unlock();
		}

		return result;
	}

	void Restore() {
		if (steamNetworkingHooker)
			steamNetworkingHooker->UnhookAll();

		if (steamFriendsHooker)
			steamFriendsHooker->UnhookAll();

		producer.Destroy();

		FreeLibraryAndExitThread(GetModuleHandle(NULL), 0);
	}
};

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