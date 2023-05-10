#pragma once

#include "ISteamNetworking.h"


CreateInterfaceFn GetFactory(HMODULE hMod)
{
    return(CreateInterfaceFn)GetProcAddress(hMod, "CreateInterface");
}

template<typename T>
T* CaptureInterface(CreateInterfaceFn f, const char* szInterfaceVersion)
{
    return (T*)f(szInterfaceVersion, NULL);
}

class ISteamClient
{
public:
    virtual int CreateSteamPipe() = 0;
    virtual int BReleaseSteamPipe(int) = 0;
    virtual int ConnectToGlobalUser(int) = 0;
    virtual int CreateLocalUser(int*, EAccountType) = 0;
    virtual int ReleaseUser(int, int) = 0;
    virtual void* GetISteamUser(int, int, char const*) = 0;
    virtual int GetISteamGameServer(int, int, char const*) = 0;
    virtual int SetLocalIPBinding(SteamIPAddress_t const&, unsigned short) = 0;
    virtual ISteamFriends* GetISteamFriends(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) = 0;
    virtual int GetISteamUtils(int, char const*) = 0;
    virtual int GetISteamMatchmaking(int, int, char const*) = 0;
    virtual int GetISteamMatchmakingServers(int, int, char const*) = 0;
    virtual int GetISteamGenericInterface(int, int, char const*) = 0;
    virtual int GetISteamUserStats(int, int, char const*) = 0;
    virtual int GetISteamGameServerStats(int, int, char const*) = 0;
    virtual int GetISteamApps(int, int, char const*) = 0;
    virtual ISteamNetworking* GetISteamNetworking(HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char* pchVersion) = 0;
    virtual int GetISteamRemoteStorage(int, int, char const*) = 0;
    virtual int GetISteamScreenshots(int, int, char const*) = 0;
    virtual int GetISteamGameSearch(int, int, char const*) = 0;
    virtual int RunFrame() = 0;
    virtual int GetIPCCallCount() = 0;
    virtual int SetWarningMessageHook(void (*)(int, char const*)) = 0;
    virtual int BShutdownIfAllPipesClosed() = 0;
    virtual int GetISteamHTTP(int, int, char const*) = 0;
    virtual int DEPRECATED_GetISteamUnifiedMessages(int, int, char const*) = 0;
    virtual int GetISteamController(int, int, char const*) = 0;
    virtual int GetISteamUGC(int, int, char const*) = 0;
    virtual int GetISteamAppList(int, int, char const*) = 0;
    virtual int GetISteamMusic(int, int, char const*) = 0;
    virtual int GetISteamMusicRemote(int, int, char const*) = 0;
    virtual int GetISteamHTMLSurface(int, int, char const*) = 0;
    virtual int DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess(void (*)()) = 0;
    virtual int DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess(void (*)()) = 0;
    virtual int Set_SteamAPI_CCheckCallbackRegisteredInProcess(unsigned int (*)(int)) = 0;
    virtual int GetISteamInventory(int, int, char const*) = 0;
    virtual int GetISteamVideo(int, int, char const*) = 0;
    virtual int GetISteamParentalSettings(int, int, char const*) = 0;
    virtual int GetISteamInput(int, int, char const*) = 0;
    virtual int GetISteamParties(int, int, char const*) = 0;
    virtual int GetISteamRemotePlay(int, int, char const*) = 0;
    virtual int DestroyAllInterfaces() = 0;
};
