#pragma once
#include "services/HookService.h"

BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) 
	{
		DisableThreadLibraryCalls(handle);

		std::thread(HookService::Run).detach();
	}
	else if (reason == DLL_PROCESS_DETACH) 
	{
		if (!reserved) 
		{
			HookService::Stop();
		}
	}
	return TRUE;
}