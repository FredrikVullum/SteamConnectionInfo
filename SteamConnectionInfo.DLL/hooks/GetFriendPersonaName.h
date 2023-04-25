#include "../steam/ISteamFriends.h"

namespace Hooks {
	typedef const char* (__thiscall* GetFriendPersonaName_t)(ISteamFriends*, CSteamID);
	const char* __fastcall GetFriendPersonaName_Hook(ISteamFriends* thisptr, void* edx, CSteamID steamIDFriend);
}