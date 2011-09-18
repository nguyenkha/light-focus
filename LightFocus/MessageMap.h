#ifndef _MESSAGE_MAP
#define _MESSAGE_MAP

#define messageMap(message, handler) case message: return handler(hWnd, message, wParam, lParam); break;

#define beginMessageMap() LRESULT CALLBACK windowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) { switch (message) {

#define endMessageMap() default: return DefWindowProc(hWnd, message, wParam, lParam); } return 0; }

#endif