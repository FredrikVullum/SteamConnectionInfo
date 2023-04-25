#include "CSteamID.h"
#include <stdlib.h>

typedef UINT32 uint;

int V_snprintf(char* pDest, int maxLen, char const* pFormat, ...)
{

	if (!maxLen)
		return 0;

	if (!pFormat)
		return 0;


	va_list marker;

	va_start(marker, pFormat);
#ifdef _WIN32
	int len = _vsnprintf(pDest, maxLen, pFormat, marker);
#elif POSIX
	int len = vsnprintf(pDest, maxLen, pFormat, marker);
#else
#error "define vsnprintf type."
#endif
	va_end(marker);

	// Len > maxLen represents an overflow on POSIX, < 0 is an overflow on windows
	if (len < 0 || len >= maxLen)
	{
		len = maxLen;
		pDest[maxLen - 1] = 0;
	}

	return len;
}

uint64 V_atoui64(const char* str)
{
	if (!str)
		return 0;

	uint64             val;
	uint64             c;

	if (strlen(str) <= 0)
		return 0;

	val = 0;

	//
	// check for hex
	//
	if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X'))
	{
		str += 2;
		while (1)
		{
			c = *str++;
			if (c >= '0' && c <= '9')
				val = (val << 4) + c - '0';
			else if (c >= 'a' && c <= 'f')
				val = (val << 4) + c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				val = (val << 4) + c - 'A' + 10;
			else
				return val;
		}
	}

	//
	// check for character
	//
	if (str[0] == '\'')
	{
		return str[1];
	}

	//
	// assume decimal
	//
	while (1)
	{
		c = *str++;
		if (c < '0' || c > '9')
			return val;
		val = val * 10 + c - '0';
	}

	return 0;
}

CSteamID::CSteamID(const char* pchSteamID, EUniverse eDefaultUniverse /* = k_EUniverseInvalid */)
{
	SetFromString(pchSteamID, eDefaultUniverse);
}



void CSteamID::SetFromString(const char* pchSteamID, EUniverse eDefaultUniverse)
{
	uint nAccountID = 0;
	uint nInstance = 1;
	EUniverse eUniverse = eDefaultUniverse;

	if (!(sizeof(eUniverse) == sizeof(int)))
		return;

	EAccountType eAccountType = k_EAccountTypeIndividual;
	if (*pchSteamID == '[')
		pchSteamID++;

	// BUGBUG Rich use the Q_ functions
	if (*pchSteamID == 'A')
	{
		// This is test only
		pchSteamID++; // skip the A
		eAccountType = k_EAccountTypeAnonGameServer;
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :

		if (strchr(pchSteamID, '('))
			sscanf(strchr(pchSteamID, '('), "(%u)", &nInstance);
		const char* pchColon = strchr(pchSteamID, ':');
		if (pchColon && *pchColon != 0 && strchr(pchColon + 1, ':'))
		{
			sscanf(pchSteamID, "%u:%u:%u", (uint*)&eUniverse, &nAccountID, &nInstance);
		}
		else if (pchColon)
		{
			sscanf(pchSteamID, "%u:%u", (uint*)&eUniverse, &nAccountID);
		}
		else
		{
			sscanf(pchSteamID, "%u", &nAccountID);
		}

		if (nAccountID == 0)
		{
			// i dont care what number you entered
			CreateBlankAnonLogon(eUniverse);
		}
		else
		{
			InstancedSet(nAccountID, nInstance, eUniverse, eAccountType);
		}
		return;
	}
	else if (*pchSteamID == 'G')
	{
		pchSteamID++; // skip the G
		eAccountType = k_EAccountTypeGameServer;
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :
	}
	else if (*pchSteamID == 'C')
	{
		pchSteamID++; // skip the C
		eAccountType = k_EAccountTypeContentServer;
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :
	}
	else if (*pchSteamID == 'g')
	{
		pchSteamID++; // skip the g
		eAccountType = k_EAccountTypeClan;
		nInstance = 0;
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :
	}
	else if (*pchSteamID == 'c')
	{
		pchSteamID++; // skip the c
		eAccountType = k_EAccountTypeChat;
		nInstance = k_EChatInstanceFlagClan;
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :
	}
	else if (*pchSteamID == 'L')
	{
		pchSteamID++; // skip the c
		eAccountType = k_EAccountTypeChat;
		nInstance = k_EChatInstanceFlagLobby;
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :
	}
	else if (*pchSteamID == 'T')
	{
		pchSteamID++; // skip the T
		eAccountType = k_EAccountTypeChat;
		nInstance = 0;	// Anon chat
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :
	}
	else if (*pchSteamID == 'U')
	{
		pchSteamID++; // skip the U
		eAccountType = k_EAccountTypeIndividual;
		nInstance = 1;
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :
	}
	else if (*pchSteamID == 'i')
	{
		pchSteamID++; // skip the i
		eAccountType = k_EAccountTypeInvalid;
		nInstance = 1;
		if (*pchSteamID == '-' || *pchSteamID == ':')
			pchSteamID++; // skip the optional - or :
	}
	else
	{
		// Check for a 64-bit steamID, unadorned.
		uint64 unSteamID64 = V_atoui64(pchSteamID);
		if (unSteamID64 > 0xffffffffU)
		{
			SetFromUint64(unSteamID64);
			return;
		}
	}

	if (strchr(pchSteamID, ':'))
	{
		if (*pchSteamID == '[')
			pchSteamID++; // skip the optional [
		sscanf(pchSteamID, "%u:%u", (uint*)&eUniverse, &nAccountID);
		if (eUniverse == k_EUniverseInvalid)
			eUniverse = eDefaultUniverse;
	}
	else
	{
		sscanf(pchSteamID, "%u", &nAccountID);
	}

	InstancedSet(nAccountID, nInstance, eUniverse, eAccountType);
}

bool CSteamID::BValidExternalSteamID() const
{
	if (m_steamid.m_comp.m_EAccountType == k_EAccountTypePending)
		return false;
	if (m_steamid.m_comp.m_EAccountType != k_EAccountTypeAnonGameServer && m_steamid.m_comp.m_EAccountType != k_EAccountTypeContentServer && m_steamid.m_comp.m_EAccountType != k_EAccountTypeAnonUser)
	{
		if (m_steamid.m_comp.m_unAccountID == 0 && m_steamid.m_comp.m_unAccountInstance == 0)
			return false;
	}
	return true;
}

