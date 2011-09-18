// KeyHook.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "KeyHook.h"


#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	hInstDLL = hModule;
	lastTime = 0;
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

LRESULT CALLBACK hookProcedure(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode < 0) {
		// Do nothing	
	} else {
		time_t currentTime;
		currentTime = time(NULL);
		if (wParam == VK_SNAPSHOT && currentTime > lastTime) {
			PostMessage(mainWindow, UWM_PRINT_SCREEN, 0, 0);
			lastTime = currentTime;
		}
	}
	return CallNextHookEx(hook, nCode, wParam, lParam);
}

KEYHOOK_API void setHook(HWND hWnd)
{
	mainWindow = hWnd;
	hook = SetWindowsHookEx(WH_KEYBOARD, (HOOKPROC)hookProcedure, hInstDLL, 0);	
}

KEYHOOK_API void clearHook()
{
	UnhookWindowsHookEx(hook);
}
