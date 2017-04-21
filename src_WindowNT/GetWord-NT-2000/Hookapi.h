// hookapi.h
#ifndef _INC_HOOKAPI
#define _INC_HOOKAPI

#include <windows.h>
#include "public.h"

#define PC_WRITEABLE	0x00020000
#define PC_USER			0x00040000
#define PC_STATIC		0x20000000

#define HOOK_NEED_CHECK 0
#define HOOK_CAN_WRITE	1
#define HOOK_ONLY_READ	2

#define BUFFERLEN		7

#define GETWORDEND_EVENT_NAME __TEXT("NH_GetWordEnd")	//added by ZHHN 1999.12.30

typedef struct _tagApiHookStruct
{
	LPSTR  lpszApiModuleName;
	LPSTR  lpszApiName;
	DWORD  dwApiOffset;
	LPVOID lpWinApiProc;
	BYTE   WinApiFiveByte[7];

	LPSTR  lpszHookApiModuleName;
	LPSTR  lpszHookApiName;
	LPVOID lpHookApiProc;
	BYTE   HookApiFiveByte[7];
	
	HINSTANCE hInst;

	BYTE   WinApiBakByte[7];
}
APIHOOKSTRUCT, *LPAPIHOOKSTRUCT;

FARPROC WINAPI NHGetFuncAddress(HINSTANCE hInst, LPCSTR lpMod, LPCSTR lpFunc);
void MakeJMPCode(LPBYTE lpJMPCode, LPVOID lpCodePoint);
void MakeMemCanWrite(LPVOID lpMemPoint, BOOL bCanWrite, int nSysMemStatus);
void HookWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus);
void RestoreWin32Api(LPAPIHOOKSTRUCT lpApiHook, int nSysMemStatus);

#endif // _INC_HOOKAPI