#pragma once

#include "../globals/players.h"

#include "../steam/ISteamFriends.h"
#include "../steam/ISteamNetworking.h"

#include "../services/IpToCountryService.h"
#include "../services/WorkerService.h"
#include "../services/PingMonitorService.h"

#include "../helpers/VirtualMethodTableHooker.h"
#include "../helpers/SignatureFinder.h"
#include "../helpers/MessageBoxShower.h"

namespace HookService 
{
	typedef const char* (__thiscall* GetPersonaName_t)(ISteamFriends*);
	const char* __fastcall GetPersonaName_Hook(ISteamFriends* thisptr, void* edx);

	typedef bool(__thiscall* SendP2PPacket_t)(ISteamNetworking*, CSteamID, const void*, unsigned int, EP2PSend, int);
	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* steamNetworkingInstance, void* edx, CSteamID steamId, const void* pubData, unsigned int cubData, EP2PSend eP2PSendType, int nChannel);

	VirtualMethodTableHooker				  SteamNetworkingHooker;
	VirtualMethodTableHooker				  SteamFriendsHooker;

	SendP2PPacket_t							  SendP2PPacket = nullptr;
	GetPersonaName_t						  GetPersonaName = nullptr;

	std::mutex								  mutex;

	std::thread								  worker_thread;
	std::thread								  ping_monitoring_thread;
	std::thread								  ip_to_country_thread;
	void Run()
	{
		HMODULE steam_client_handle = 0;

		while (!steam_client_handle) {
			steam_client_handle = GetModuleHandleA("steamclient.dll");
			Sleep(200);
		}

		DWORD CClientNetworkingOffset = SignatureFinder::Find(steam_client_handle, "C7 07 ? ? ? ? C7 47 ? ? ? ? ? E8 ? ? ? ? 8B 47 0C") + 2;
		if (!CClientNetworkingOffset) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find the CClientNetworking offset");
			return;
		}

		DWORD* CClientNetworkingVmt = *(DWORD**)CClientNetworkingOffset;
		if (!CClientNetworkingVmt) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find the CClientNetworking virtual method table");
			return;
		}

		DWORD CUserFriendsOffset = SignatureFinder::Find(steam_client_handle, "C7 07 ? ? ? ? C7 47 ? ? ? ? ? 8B 40 24") + 2;
		if (!CUserFriendsOffset) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find the CUserFriends offset");
			return;
		}
		
		DWORD* CUserFriendsVmt = *(DWORD**)CUserFriendsOffset;
		if (!CUserFriendsVmt) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find the CUserFriends virtual method table");
			return;
		}

		SteamNetworkingHooker.SetVmt(CClientNetworkingVmt);
		SteamFriendsHooker.SetVmt(CUserFriendsVmt);
	
		SendP2PPacket =  (SendP2PPacket_t)SteamNetworkingHooker.Hook(0, SendP2PPacket_Hook);
		if (!SendP2PPacket) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to hook SendP2PPacket()");
			return;
		}

		GetPersonaName = (GetPersonaName_t)SteamFriendsHooker.Hook(0, GetPersonaName_Hook);
		if (!GetPersonaName) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to hook GetPersonaName()");
			return;
		}

		worker_thread = std::thread(WorkerService::Run);
		ping_monitoring_thread = std::thread(PingMonitorService::Run);
		ip_to_country_thread = std::thread(IpToCountryService::Run);

		worker_thread.detach();
		ping_monitoring_thread.detach();
		ip_to_country_thread.detach();
	}

	ISteamFriends* SteamFriends;

	const char* __fastcall GetPersonaName_Hook(ISteamFriends* steam_friends_interface, void* edx)
	{
		if (SteamFriends != steam_friends_interface) {
			SteamFriends = steam_friends_interface;
		}
		return GetPersonaName(steam_friends_interface);
	}

	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* steam_networking_interface, void* edx, CSteamID steam_id, const void* bytes, unsigned int num_bytes, EP2PSend send_type, int channel)
	{
		mutex.lock();
		try {
			auto player = &players[steam_id];

			player->last_p2p_send = std::chrono::steady_clock::now();

			steam_networking_interface->GetP2PSessionState(steam_id, &(player->session_state));

			if (SteamFriends) {
				player->steam_name = SteamFriends->GetFriendPersonaName(steam_id);
			}
		}
		catch (const std::exception& ex) {

		}
		mutex.unlock();

		return SendP2PPacket(steam_networking_interface, steam_id, bytes, num_bytes, send_type, channel);
	}

	void Stop() {
		SteamNetworkingHooker.UnhookAll();
		SteamFriendsHooker.UnhookAll();	

		IpToCountryService::Stop();
		PingMonitorService::Stop();
		WorkerService::Stop();

		worker_thread.join();
		ping_monitoring_thread.join();
		ip_to_country_thread.join();

		FreeConsole();
		FreeLibraryAndExitThread(GetModuleHandle(NULL), 0);
	}
};