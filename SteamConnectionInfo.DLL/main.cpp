#pragma once
#include "services/HookService.h"

BOOL WINAPI DllMain(const HMODULE hModule, const DWORD dwReason, const LPVOID lpReserved)
{
	switch (dwReason)
	{
		case DLL_PROCESS_ATTACH: 
		{
			DisableThreadLibraryCalls(hModule);

			HANDLE hookThreadHandle = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HookService::Run, hModule, 0, 0);

			if (hookThreadHandle)
				CloseHandle(hookThreadHandle);
			else
				return FALSE;
		}
		break;
	}
	return TRUE;
}

