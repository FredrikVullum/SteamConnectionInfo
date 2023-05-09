#pragma once

#include "../globals/players.h"

#include "../steam/ISteamFriends.h"
#include "../steam/ISteamNetworking.h"

#include "../helpers/VirtualMethodTableHooker.h"
#include "../helpers/SignatureFinder.h"


namespace HookService {
	typedef const char* (__thiscall* GetPersonaName_t)(ISteamFriends*);
	const char* __fastcall GetPersonaName_Hook(ISteamFriends* thisptr);

	typedef bool(__thiscall* SendP2PPacket_t)(ISteamNetworking*, CSteamID, const void*, unsigned int, EP2PSend, int);
	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* steamNetworkingInstance, void* edx, CSteamID steamId, const void* pubData, unsigned int cubData, EP2PSend eP2PSendType, int nChannel);

	std::unique_ptr<VirtualMethodTableHooker> SteamNetworkingHooker = nullptr;
	std::unique_ptr<VirtualMethodTableHooker> SteamFriendsHooker = nullptr;

	SendP2PPacket_t							  SendP2PPacket = nullptr;
	GetPersonaName_t						  GetPersonaName = nullptr;

	std::mutex								  mutex;

	void Run()
	{
		HMODULE steam_client_handle = 0;

		while (!steam_client_handle) {
			steam_client_handle = GetModuleHandleA("steamclient.dll");
			Sleep(200);
		}

		DWORD CClientNetworking = SignatureFinder::Find(steam_client_handle, "C7 07 ? ? ? ? C7 47 ? ? ? ? ? E8 ? ? ? ? 8B 47 0C") + 2;

		DWORD CUserFriends = SignatureFinder::Find(steam_client_handle, "C7 07 ? ? ? ? C7 47 ? ? ? ? ? 8B 40 24") + 2;

		SteamNetworkingHooker = std::make_unique<VirtualMethodTableHooker>(*(DWORD**)CClientNetworking);
		if (!SteamNetworkingHooker)
			return;

		SteamFriendsHooker = std::make_unique<VirtualMethodTableHooker>(*(DWORD**)CUserFriends);
		if (!SteamFriendsHooker)
			return;

		SendP2PPacket = (SendP2PPacket_t)SteamNetworkingHooker->Hook(0, SendP2PPacket_Hook);
		if (!SendP2PPacket)
			return;

		GetPersonaName = (GetPersonaName_t)SteamFriendsHooker->Hook(0, GetPersonaName_Hook);
		if (!GetPersonaName)
			return;
	}

	ISteamFriends* SteamFriends;
	const char* __fastcall GetPersonaName_Hook(ISteamFriends* steam_friends_interface)
	{
		mutex.lock();
		if (SteamFriends != steam_friends_interface) {
			SteamFriends = steam_friends_interface;
		}
		mutex.unlock();

		return GetPersonaName(steam_friends_interface);
	}

	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* steam_networking_interface, void* edx, CSteamID steam_id, const void* bytes, unsigned int num_bytes, EP2PSend send_type, int channel)
	{
		mutex.lock();
		auto player = &players[steam_id];
		player->last_p2p_send = std::chrono::steady_clock::now();
		steam_networking_interface->GetP2PSessionState(steam_id, &(player->session_state));

		if(SteamFriends){
			std::string steam_name = SteamFriends->GetFriendPersonaName(steam_id);
			if (player->steam_name != steam_name)
				player->steam_name = steam_name;
		}
		mutex.unlock();

		return SendP2PPacket(steam_networking_interface, steam_id, bytes, num_bytes, send_type, channel);
	}

	void Stop() {
		if (SteamNetworkingHooker)
			SteamNetworkingHooker->UnhookAll();

		if (SteamFriendsHooker)
			SteamFriendsHooker->UnhookAll();
	}
};