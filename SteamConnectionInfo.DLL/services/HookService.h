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
#include "../helpers/ProcessTerminator.h"

namespace HookService 
{
	typedef const char* (__thiscall* GetPersonaName_t)(ISteamFriends*);
	const char* __fastcall GetPersonaName_Hook(ISteamFriends* thisptr, void* edx);

	typedef bool(__thiscall* SendP2PPacket_t)(ISteamNetworking*, CSteamID, const void*, unsigned int, EP2PSend, int);
	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* steamNetworkingInstance, void* edx, CSteamID steamId, const void* pubData, unsigned int cubData, EP2PSend eP2PSendType, int nChannel);

	typedef bool(__thiscall* ReadP2PPacket_t)(ISteamNetworking*, void*, uint32, uint32*, CSteamID*, int32);
	bool __fastcall ReadP2PPacket_Hook(ISteamNetworking* steam_networking_interface, void* edx, void* pubDest, uint32 cubDest, uint32* pcubMsgSize, CSteamID* psteamIDRemote, int32 iVirtualPort);

	VirtualMethodTableHooker	SteamNetworkingHooker;
	VirtualMethodTableHooker	SteamFriendsHooker;

	SendP2PPacket_t				SendP2PPacket = nullptr;
	ReadP2PPacket_t				ReadP2PPacket = nullptr;

	GetPersonaName_t			GetPersonaName = nullptr;

	void*						module_handle_ptr = nullptr;

	std::thread					worker_thread;
	std::thread					ping_monitoring_thread;
	std::thread					ip_to_country_thread;

	void Exit() 
	{
		SteamNetworkingHooker.UnhookAll();
		SteamFriendsHooker.UnhookAll();

		IpToCountryService::Stop();
		PingMonitorService::Stop();
		WorkerService::Stop();

		if(worker_thread.joinable())
			worker_thread.join();

		if (ping_monitoring_thread.joinable())
			ping_monitoring_thread.join();

		if (ip_to_country_thread.joinable())
			ip_to_country_thread.join();

		std::this_thread::sleep_for(std::chrono::milliseconds(1000));

		FreeLibraryAndExitThread(static_cast<HMODULE>(module_handle_ptr), 0);
	}

	void Run(void* module_handle_ptr_from)
	{
		module_handle_ptr = module_handle_ptr_from;

		HMODULE steam_client_handle = 0;

		while (!steam_client_handle) {
			steam_client_handle = GetModuleHandleA("steamclient.dll");
			Sleep(200);
		}

		DWORD CClientNetworkingOffset = SignatureFinder::Find(steam_client_handle, "C7 07 ? ? ? ? C7 47 ? ? ? ? ? E8 ? ? ? ? 8B 47 0C") + 2;
		if (!CClientNetworkingOffset) {
			
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find the CClientNetworking offset");
			ProcessTerminator::TerminateProcessByName("SteamConnectionInfo.WPF.exe");
			Exit();
			return;
		}

		DWORD* CClientNetworkingVmt = *(DWORD**)CClientNetworkingOffset;
		if (!CClientNetworkingVmt) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find the CClientNetworking virtual method table");
			ProcessTerminator::TerminateProcessByName("SteamConnectionInfo.WPF.exe");
			Exit();
			return;
		}

		DWORD CUserFriendsOffset = SignatureFinder::Find(steam_client_handle, "C7 07 ? ? ? ? C7 47 ? ? ? ? ? 8B 40 24") + 2;
		if (!CUserFriendsOffset) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find the CUserFriends offset");
			ProcessTerminator::TerminateProcessByName("SteamConnectionInfo.WPF.exe");
			Exit();
			return;
		}
		
		DWORD* CUserFriendsVmt = *(DWORD**)CUserFriendsOffset;
		if (!CUserFriendsVmt) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to find the CUserFriends virtual method table");
			ProcessTerminator::TerminateProcessByName("SteamConnectionInfo.WPF.exe");
			Exit();
			return;
		}

		SteamNetworkingHooker.SetVmt(CClientNetworkingVmt);
		SteamFriendsHooker.SetVmt(CUserFriendsVmt);
	
		SendP2PPacket =  (SendP2PPacket_t)SteamNetworkingHooker.Hook(0, SendP2PPacket_Hook);
		if (!SendP2PPacket) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to hook SendP2PPacket()");
			ProcessTerminator::TerminateProcessByName("SteamConnectionInfo.WPF.exe");
			Exit();
			return;
		}

		ReadP2PPacket = (ReadP2PPacket_t)SteamNetworkingHooker.Hook(2, ReadP2PPacket_Hook);
		if (!ReadP2PPacket) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to hook ReadP2PPacket()");
			ProcessTerminator::TerminateProcessByName("SteamConnectionInfo.WPF.exe");
			Exit();
			return;
		}

		GetPersonaName = (GetPersonaName_t)SteamFriendsHooker.Hook(0, GetPersonaName_Hook);
		if (!GetPersonaName) {
			MessageBoxShower::ShowError("SteamConnectionInfo.dll Error", "Failed to hook GetPersonaName()");
			ProcessTerminator::TerminateProcessByName("SteamConnectionInfo.WPF.exe");
			Exit();
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

	bool __fastcall ReadP2PPacket_Hook(ISteamNetworking* steam_networking_interface, void* edx, void* pubDest, uint32 cubDest, uint32* pcubMsgSize, CSteamID* psteamIDRemote, int32 iVirtualPort)
	{
		//number 2
		if (GetAsyncKeyState(0x32) & 0x8000) {
			return false;
		}

		return ReadP2PPacket(steam_networking_interface, pubDest, cubDest, pcubMsgSize, psteamIDRemote, iVirtualPort);
	}

	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* steam_networking_interface, void* edx, CSteamID steam_id, const void* bytes, unsigned int num_bytes, EP2PSend send_type, int channel)
	{
		std::lock_guard<std::mutex> lock(players_mutex);

		auto player = &players[steam_id];
		player->last_p2p_send = std::chrono::steady_clock::now();

		try 
		{
			steam_networking_interface->GetP2PSessionState(steam_id, &(player->session_state));

			if (SteamFriends) 
			{
				player->steam_name = SteamFriends->GetFriendPersonaName(steam_id);
			}
		}
		catch (const std::exception& ex) 
		{

		}
	
		return SendP2PPacket(steam_networking_interface, steam_id, bytes, num_bytes, send_type, channel);
	}
};