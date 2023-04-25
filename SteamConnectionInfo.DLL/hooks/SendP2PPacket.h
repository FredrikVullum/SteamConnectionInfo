
#include "../steam/ISteamNetworking.h"

namespace Hooks {
	typedef bool(__thiscall* SendP2PPacket_t)(ISteamNetworking*, CSteamID, const void*, unsigned int, EP2PSend, int);
	bool __fastcall SendP2PPacket_Hook(ISteamNetworking* steamNetworkingInstance, void* edx, CSteamID steamId, const void* pubData, unsigned int cubData, EP2PSend eP2PSendType, int nChannel);
}