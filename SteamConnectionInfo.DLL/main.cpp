#pragma once
#include "services/HookService.h"
#include "services/WorkerService.h"
#include "services/PingMonitorService.h"
#include "services/IpToCountryService.h"

BOOL WINAPI DllMain(HINSTANCE handle, DWORD reason, LPVOID reserved)
{
	if (reason == DLL_PROCESS_ATTACH) 
	{
		DisableThreadLibraryCalls(handle);

		std::thread hookThread(HookService::Run);
		std::thread workerThread(WorkerService::Run);
		std::thread pingMonitorThread(PingMonitorService::Run);
		std::thread ipToCountryThread(IpToCountryService::Run);
		
		hookThread.detach();
		workerThread.detach();
		pingMonitorThread.detach();
		ipToCountryThread.detach();
	}
	else if (reason == DLL_PROCESS_DETACH) 
	{
		if (!reserved) 
		{
			IpToCountryService::Stop();
			PingMonitorService::Stop();
			WorkerService::Stop();
			HookService::Stop();
			FreeLibraryAndExitThread(GetModuleHandle(NULL), 0);
		}
	}
	return TRUE;
}