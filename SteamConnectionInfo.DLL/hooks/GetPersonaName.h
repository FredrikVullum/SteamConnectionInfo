#include "../steam/ISteamFriends.h"

namespace Hooks {
	typedef const char* (__thiscall* GetPersonaName_t)(ISteamFriends*);
	const char* __fastcall GetPersonaName_Hook(ISteamFriends* thisptr);
}