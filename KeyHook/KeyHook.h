// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the KEYHOOK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// KEYHOOK_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef KEYHOOK_EXPORTS
#define KEYHOOK_API __declspec(dllexport)
#else
#define KEYHOOK_API __declspec(dllimport)
#endif

// Private declare
#ifdef KEYHOOK_EXPORTS

#include <time.h>

// Simple share memory bewteen process
#pragma data_seg(".LF")
HWND mainWindow = NULL;
time_t lastTime = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.LF,rws")

HHOOK hook;

HINSTANCE hInstDLL;

LRESULT CALLBACK hookProcedure(int nCode, WPARAM wParam, LPARAM lParam);

#include <time.h>



#endif

// Export deaclare

#define UWM_PRINT_SCREEN WM_USER + 11

KEYHOOK_API void setHook(HWND hWnd);

KEYHOOK_API void clearHook();