/////////////////////////////////////////////////////////////////////////
//
// exports.c
//
// Author : Chen Shuqing
//
// Date   : 04/18/99
//
// Fix Bug: Zhang Haining
//
// Date   : 01/17/2000
//
/////////////////////////////////////////////////////////////////////////

#include <windows.h>

#include "findword.h"
#include "exports.h"
#include "hookapi.h"
#include "public.h"
#include "dbgprint.h"

#pragma data_seg(".sdata")

UINT g_nFlag = 0;	//must share
HWND g_hNotifyWnd = NULL;	//must share
int g_MouseX = 0;	//must share
int g_MouseY = 0;	//must share
BOOL g_bNewGetWordFlag = FALSE;		//must share

char g_szTotalWord[BUFFERLENGTH] = "";	//must share
RECT g_TotalWordRect = {0,0,0,0};			// 用於記錄完整詞的區域大小﹒must share

int  g_bMouseInTotalWord = FALSE;           // 用於記錄光標是否在完整詞中﹒must share
int  g_nCurCaretPlaceInTotalWord = -1;		// 用於記錄光標在完整詞中的位置﹒must share
RECT g_rcFirstWordRect = {0,0,0,0};	//must share

int g_nGetWordStyle = 0;	//must share
int g_nWordsInPhrase = -1;	//must share
BOOL g_bPhraseNeeded = FALSE;	//must share

BOOL g_bHooked = FALSE;	//must share

//int g_nProcessHooked = 0;		//must share

char szMemDCWordBuff[BUFFERLENGTH] = "";	// 用於記錄所有 MemDC 中的 Text 文本﹒
int  pnMemDCCharLeft[BUFFERLENGTH];			// 用於記錄在 TextOut 中所有字的左相對值﹒
int  pnMemDCCharRight[BUFFERLENGTH];		// 用於記錄在 TextOut 中所有字的右相對值﹒
WORDPARA WordBuffer[MEMDC_MAXNUM];			// 用於記錄在 TextOut 中切詞後所有詞的信息﹒
int nWordNum = 0;							// 記錄 MemDC 中單詞的個數﹒

#pragma data_seg()

UINT g_uMsg = 0;

BOOL g_bOldGetWordFlag = FALSE;

HWND g_hWndParent = NULL;

BOOL  g_bAllowGetCurWord = FALSE;	

int  g_CharType = CHAR_TYPE_OTHER;			// 用於記錄完整詞的類型﹒

// 當前詞數據結構﹒( 當前詞：為由輸入緩衝區中切出的單詞 )
char g_szCurWord[WORDMAXLEN] = "";
RECT g_CurWordRect = {0,0,0,0};
int  g_nCurCaretPlace = 0;
POINT g_CurMousePos = {0,0};

UINT         g_nTextAlign = 0;
POINT        g_dwDCOrg = {0,0};
int          g_nExtra = 0;
POINT        g_CurPos = {0,0};
TEXTMETRIC   g_tm;

BOOL bRecAllRect = TRUE;
RECT g_rcTotalRect = {0,0,0,0};

UINT BL_HASSTRING = 0;

int g_nPhraseCharType = CHAR_TYPE_OTHER;

//#pragma data_seg()

HHOOK g_hHook = NULL;	//can not share else calling UnhookWindowsHookEx() will occur error
HINSTANCE g_hinstDll = NULL;
HANDLE hMutex = NULL;

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
LPSTR UnicodeToAnsi(LPTSTR lpString, UINT cbCount);

APIHOOKSTRUCT g_BitBltHook = {
	"gdi32.dll",
	"BitBlt",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHBitBlt",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

APIHOOKSTRUCT g_TextOutAHook = {
	"gdi32.dll",
	"TextOutA",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHTextOutA",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

APIHOOKSTRUCT g_TextOutWHook = {
	"gdi32.dll",
	"TextOutW",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHTextOutW",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

APIHOOKSTRUCT g_ExtTextOutAHook = {
	"gdi32.dll",
	"ExtTextOutA",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHExtTextOutA",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

APIHOOKSTRUCT g_ExtTextOutWHook = {
	"gdi32.dll",
	"ExtTextOutW",
	0,
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	NULL,
	"NHExtTextOutW",
	NULL,
	{0, 0, 0, 0, 0, 0, 0},
	0,
	{0XFF, 0X15, 0XFA, 0X13, 0XF3, 0XBF, 0X33}
};

#pragma comment(linker,"-section:.sdata,rws")

////////////////////////////////////////////////////////////////////////////////
//
//	DllMain()
//
////////////////////////////////////////////////////////////////////////////////
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason) 
	{
		case DLL_PROCESS_ATTACH:

			g_hinstDll = hinstDLL;

			g_BitBltHook.hInst = hinstDLL;
			g_TextOutAHook.hInst = hinstDLL;
			g_TextOutWHook.hInst = hinstDLL;
			g_ExtTextOutAHook.hInst = hinstDLL;
			g_ExtTextOutWHook.hInst = hinstDLL;

			g_uMsg = RegisterWindowMessage("Noble Hand");
			if(!g_uMsg)
			{
				return FALSE;
			}

			//Added by ZHHN on 2000.2.2
			//Because forget to add this function before, with the result that it gets word
			//little slowly

			BL_HASSTRING = RegisterWindowMessage(MSG_HASSTRINGNAME);
			if(!BL_HASSTRING)
			{
				return FALSE;
			}

			// create mutex

			hMutex = CreateMutex(NULL, FALSE, MUTEXNAME);
			if (NULL == hMutex)
			{
				return FALSE;
			}
/*
			// 01/17/2000
			// Fix Bug1: NHW32 can not be free from memory when nhstart.exe exit

			// Fix Bug1 begin

			WaitForSingleObject(hMutex, INFINITE);

			g_nProcessHooked++;

			if(g_nProcessHooked == 1 && g_hHook == NULL)
			{
				g_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstDll, 0);
				if (g_hHook == NULL)
				{
					//MessageBox(NULL, __TEXT("Error hooking."), 
					//		   __TEXT("GetWord"), MB_OK);
					ReleaseMutex(hMutex);
					return FALSE;
				}
			}

			ReleaseMutex(hMutex);

			// Fix Bug1 end
*/
			break;

		case DLL_THREAD_ATTACH:
			 break;

		case DLL_THREAD_DETACH:
			 break;
		
		case DLL_PROCESS_DETACH:

			// restore
			NHUnHookWin32Api();
/*
			// 01/17/2000
			// Fix Bug1: NHW32 can not be free from memory when nhstart.exe exit

			// Fix Bug1 begin

			WaitForSingleObject(hMutex, INFINITE);

			g_nProcessHooked--;
			
			if (g_nProcessHooked == 0 && g_hHook != NULL)
			{			
				if (!UnhookWindowsHookEx(g_hHook))
				{
					//{
					//	char cBuffer[0x100];
					//	wsprintf(cBuffer, "Error unhooking: %d\n", GetLastError());
					//	OutputDebugString(cBuffer);
					//	MessageBox(NULL, __TEXT(cBuffer), 
					//		   __TEXT("GetWord"), MB_OK);
					//}

					ReleaseMutex(hMutex);
					return FALSE;
				}
				g_hHook = NULL;
			}

			ReleaseMutex(hMutex);

			// Fix Bug1 end
*/
			// close mutex
			if (NULL != hMutex)
			{
				CloseHandle(hMutex);
			}

			break;
    }

    return TRUE;
}

__inline void ProcessWin32API()
{
	if (g_bNewGetWordFlag != g_bOldGetWordFlag)
	{
		if (g_bNewGetWordFlag)
		{
			WaitForSingleObject(hMutex, INFINITE);

			g_nGetWordStyle = g_nFlag;
			g_bAllowGetCurWord = TRUE;
			g_CurMousePos.x = g_MouseX;
			g_CurMousePos.y = g_MouseY;
			g_szCurWord[0] = 0x00;
			g_nCurCaretPlace = -1;

			if (!g_bHooked)
			{
				NHHookWin32Api();
				g_bHooked = TRUE;
			}

			g_bAllowGetCurWord = TRUE;

			ReleaseMutex(hMutex);
		}
		else
		{
			WaitForSingleObject(hMutex, INFINITE);

			g_bAllowGetCurWord = FALSE;

			if (g_bHooked)
			{
				RestoreWin32Api(&g_BitBltHook, HOOK_ONLY_READ);
				RestoreWin32Api(&g_TextOutAHook, HOOK_ONLY_READ);
				RestoreWin32Api(&g_TextOutWHook, HOOK_ONLY_READ);
				RestoreWin32Api(&g_ExtTextOutAHook, HOOK_ONLY_READ);
				RestoreWin32Api(&g_ExtTextOutWHook, HOOK_ONLY_READ);
				g_bHooked = FALSE;
			}

			ReleaseMutex(hMutex);
		}
	}

	g_bOldGetWordFlag = g_bNewGetWordFlag;
}

DLLEXPORT DWORD WINAPI BL_SetFlag32(UINT nFlag, HWND hNotifyWnd,
									int MouseX, int MouseY)
{
	POINT ptWindow;
	HWND hWnd;
	//char classname[256];
	DWORD dwCurrProcessId = 0;
	DWORD dwProcessIdOfWnd = 0;

/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "%s\n", "BL_SetFlag32");
		OutputDebugString(cBuffer);
	}
*/
	g_nFlag = nFlag;
	g_hNotifyWnd = hNotifyWnd;
	g_MouseX = MouseX;
	g_MouseY = MouseY;

	ptWindow.x = MouseX;
	ptWindow.y = MouseY;
	hWnd = WindowFromPoint(ptWindow);

	g_hWndParent = hWnd;

/*
	classname[0] = 0x0 ;
	GetClassName(hWnd, classname, sizeof(classname));

	{
		char cBuffer[0x100];
		wsprintf(cBuffer, ">>>>>>ClassName: %s\n", classname);
		OutputDebugString(cBuffer);
	}
*/

	// 01/17/2000
	// Fix Bug2: can not get word from detail dict window
	// Fix bug2 begin

	switch (nFlag)
	{
//		case GETWORD_ENABLE:
		case GETWORD_D_ENABLE:
		case GETWORD_TW_ENABLE:
		case GETPHRASE_ENABLE:

			g_bNewGetWordFlag = TRUE;

			BL_SetGetWordStyle(GETPHRASE_ENABLE);

			dwCurrProcessId = GetCurrentProcessId();
			GetWindowThreadProcessId(g_hWndParent, &dwProcessIdOfWnd);

			if(dwProcessIdOfWnd == dwCurrProcessId)
			{
				ProcessWin32API();
			}
			else
			{				
				SendMessage(g_hWndParent, g_uMsg, 0, 0);
				PostMessage(g_hWndParent, g_uMsg, 0, 0);			
			}

			break;

		case GETWORD_D_TYPING_ENABLE:

			g_bNewGetWordFlag = TRUE;

			BL_SetGetWordStyle(GETPHRASE_DISABLE);

			PostMessage(g_hWndParent, g_uMsg, 0, 0);			

			break;

		case GETWORD_DISABLE:
		case GETPHRASE_DISABLE:

			g_bNewGetWordFlag = FALSE;

			dwCurrProcessId = GetCurrentProcessId();
			GetWindowThreadProcessId(g_hWndParent, &dwProcessIdOfWnd);

			if(dwProcessIdOfWnd == dwCurrProcessId)
			{
				ProcessWin32API();
			}
			else
			{
				SendMessage(g_hWndParent, g_uMsg, 0, 0);
				PostMessage(g_hWndParent, g_uMsg, 0, 0);			
			}

			break;

		case GETWORD_D_TYPING_DISABLE:

			g_bNewGetWordFlag = FALSE;

			PostMessage(g_hWndParent, g_uMsg, 0, 0);			

			break;

		default:
			break;
	}

	// Fix bug2 end

	return BL_OK;
}

DLLEXPORT DWORD WINAPI NHHookWin32Api()
{
	g_nCurCaretPlace = -1;
	g_szCurWord[0] = 0x00;
	g_szTotalWord[0] = 0x00;
	g_nCurCaretPlaceInTotalWord = -1;
	g_CharType = CHAR_TYPE_OTHER;
	g_bMouseInTotalWord = FALSE;
	g_bAllowGetCurWord = TRUE;

	nWordNum = 0;
	szMemDCWordBuff[0] = 0x00;

	HookAllTextOut();

	return BL_OK;
}

DLLEXPORT DWORD WINAPI NHUnHookWin32Api()
{
	g_bAllowGetCurWord = FALSE;
	UnHookAllTextOut();
        
	return BL_OK;
}

void HookAllTextOut()
{
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "HookAllTextOut : %s\n", "ok");
		OutputDebugString(cBuffer);
	}
*/
	HookWin32Api(&g_BitBltHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_TextOutAHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_TextOutWHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_ExtTextOutAHook, HOOK_CAN_WRITE);
	HookWin32Api(&g_ExtTextOutWHook, HOOK_CAN_WRITE);
}

void UnHookAllTextOut()
{
	RestoreWin32Api(&g_BitBltHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_TextOutAHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_TextOutWHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_ExtTextOutAHook, HOOK_NEED_CHECK);
	RestoreWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);
}

DLLEXPORT BOOL WINAPI NHBitBlt(HDC hdcDest,
						       int nXDest,
						       int nYDest,
						       int nWidth,
						       int nHeight,
						       HDC hdcSrc,
						       int nXSrc,
						       int nYSrc,
						       DWORD dwRop)
{
	int x, y;

	POINT pt;
	HWND  hWDC;
	HWND  hWPT;
	//char lpClassName[256];
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHBitBlt : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("-> NHBitBlt : %s\n", "start");

	// restore
	RestoreWin32Api(&g_BitBltHook, HOOK_NEED_CHECK);

	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdcDest);
	hWPT = WindowFromPoint(pt);	

	if (hWPT == hWDC) //added by zhhn 01/17/2000
	{
		if (nWidth > 5)
		{
				GetDCOrgEx(hdcDest, &g_dwDCOrg);
				x = g_dwDCOrg.x;
				y = g_dwDCOrg.y;
				x += nXDest;
				y += nYDest;
				g_dwDCOrg.x = x;
				g_dwDCOrg.y = y;
    
				CheckMemDCWordBuffer(hdcDest, hdcSrc);
		}
		else
		{
			if (CheckDCWndClassName(hdcDest))
			{
				GetDCOrgEx(hdcDest, &g_dwDCOrg);
				x = g_dwDCOrg.x;
				y = g_dwDCOrg.y;
				x += nXDest;
				y += nYDest;
				g_dwDCOrg.x = x;
				g_dwDCOrg.y = y;
    
				CheckMemDCWordBuffer(hdcDest, hdcSrc);
			}
		}
	}

	// call BitBlt
	BitBlt(hdcDest, nXDest, nYDest, nWidth, nHeight,
		   hdcSrc, nXSrc, nYSrc, dwRop);

	HookWin32Api(&g_BitBltHook, HOOK_NEED_CHECK);

	return TRUE;
}

DLLEXPORT BOOL WINAPI NHTextOutA(HDC hdc,
							     int nXStart,
							     int nYStart,
							     LPCTSTR lpString,
							     int cbString)
{
	POINT pt;
	HWND  hWDC;
	HWND  hWPT;
	DWORD dwThreadIdWithPoint = 0;
	DWORD dwThreadIdCurr = 0;

/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHTextOutA : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/

	//DbgFilePrintf("-> NHTextOutA : lpString(%s), cbString(%d)\n", lpString, cbString);

	// restore
	RestoreWin32Api(&g_TextOutAHook, HOOK_NEED_CHECK);

	//
	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdc);
	hWPT = WindowFromPoint(pt);

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);
	dwThreadIdCurr = GetCurrentThreadId();

	if(dwThreadIdWithPoint == dwThreadIdCurr)
	{
		if (hWDC == NULL || hWPT == hWDC
			|| IsParentOrSelf(hWPT, hWDC)
			|| IsParentOrSelf(hWDC, hWPT))
		{
			if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpString, cbString))
				&& (cbString > 0))
			{
					g_nTextAlign = GetTextAlign(hdc);
					g_nExtra     = GetTextCharacterExtra(hdc);
					GetCurrentPositionEx(hdc, &g_CurPos);
					GetTextMetrics(hdc, &g_tm);
        
					g_dwDCOrg.x = 0;
					g_dwDCOrg.y = 0;
					bRecAllRect = FALSE;
					GetStringRect(hdc, (LPSTR)lpString, cbString, nXStart,
								  nYStart, &g_rcTotalRect, NULL);
					bRecAllRect = TRUE;					

					if ((WindowFromDC != NULL)&&(WindowFromDC(hdc) == NULL))
					{
							g_dwDCOrg.x = 0;
							g_dwDCOrg.y = 0;

							AddToTextOutBuffer(hdc, (LPSTR)lpString, cbString,
											   nXStart, nYStart, NULL);
					}
					else
					{
							GetDCOrgEx(hdc, &g_dwDCOrg);
        
							GetCurMousePosWord(hdc, (LPSTR)lpString, cbString,
											   nXStart, nYStart, NULL);
					}
			}
		}
	}

	// call TextOutA
	TextOutA(hdc, nXStart, nYStart, lpString, cbString);

	HookWin32Api(&g_TextOutAHook, HOOK_NEED_CHECK);

	return TRUE;
}

DLLEXPORT BOOL WINAPI NHTextOutW(HDC hdc,
							     int nXStart,
							     int nYStart,
							     LPCWSTR lpString,
							     int cbString)
{
	POINT pt;
	HWND  hWDC;
	HWND  hWPT;
	DWORD dwThreadIdWithPoint = 0;
	DWORD dwThreadIdCurr = 0;

/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHTextOutW : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("-> NHTextOutW : lpString(%s), cbString(%d)\n", lpString, cbString);

	// restore
	RestoreWin32Api(&g_TextOutWHook, HOOK_NEED_CHECK);

	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdc);
	hWPT = WindowFromPoint(pt);

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);
	dwThreadIdCurr = GetCurrentThreadId();

	if(dwThreadIdWithPoint == dwThreadIdCurr)
	{
		if (hWDC == NULL || hWPT == hWDC
			|| IsParentOrSelf(hWPT, hWDC)
			|| IsParentOrSelf(hWDC, hWPT))
		{
			if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpString, cbString))
				&& (cbString > 0))
			{
				g_nTextAlign = GetTextAlign(hdc);
				g_nExtra     = GetTextCharacterExtra(hdc);
				GetCurrentPositionEx(hdc, &g_CurPos);
				GetTextMetrics(hdc, &g_tm);
				g_dwDCOrg.x = 0;
				g_dwDCOrg.y = 0;
				bRecAllRect = FALSE;
				GetStringRectW(hdc, lpString, cbString, nXStart,
					nYStart, &g_rcTotalRect, NULL);
				bRecAllRect = TRUE;

				if ((WindowFromDC != NULL)&&(WindowFromDC(hdc) == NULL))
				{
						g_dwDCOrg.x = 0;
						g_dwDCOrg.y = 0;

                    	// 01/19/2000
						// Fix Bug5: get word position error sometimes
						// Fix Bug5 begin

						AddToTextOutBufferW(hdc, lpString, cbString,
							nXStart, nYStart, NULL);

						//Fix Bug5 end
				}
				else
				{
						GetDCOrgEx(hdc, &g_dwDCOrg);

                    	// 01/19/2000
						// Fix Bug5: get word position error sometimes
						// Fix Bug5 begin
    
						GetCurMousePosWordW(hdc, lpString, cbString,
							nXStart, nYStart, NULL);

						//Fix Bug5 end
				}
			}
		}
	}

	// call TextOutW
	TextOutW(hdc, nXStart, nYStart, lpString, cbString);

	HookWin32Api(&g_TextOutWHook, HOOK_NEED_CHECK);

	return TRUE;
}

DLLEXPORT BOOL WINAPI NHExtTextOutA(HDC hdc,
								    int X,
								    int Y,
								    UINT fuOptions,
								    CONST RECT *lprc,
								    LPCTSTR lpString,
								    UINT cbCount,
								    CONST INT *lpDx)
{
	POINT pt;
	HWND  hWDC;
	HWND  hWPT;
	DWORD dwThreadIdWithPoint = 0;
	DWORD dwThreadIdCurr = 0;

	// restore
	RestoreWin32Api(&g_ExtTextOutAHook, HOOK_NEED_CHECK);
/*
	if (cbCount != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHExtTextOutA : %s (%s) (%d)\n", "start", lpString, cbCount);
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("-> NHExtTextOutA : lpString(%s), cbCount(%d)\n", lpString, cbCount);

	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdc);
	hWPT = WindowFromPoint(pt);

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);
	dwThreadIdCurr = GetCurrentThreadId();

	if(dwThreadIdWithPoint == dwThreadIdCurr)
	{
		if (hWDC == NULL || hWPT == hWDC
			|| IsParentOrSelf(hWPT, hWDC)
			|| IsParentOrSelf(hWDC, hWPT))
		{
			if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpString, cbCount))
				&& (cbCount > 0))
			{
				g_nTextAlign = GetTextAlign(hdc);
				g_nExtra     = GetTextCharacterExtra(hdc);
				GetCurrentPositionEx(hdc, &g_CurPos);
				GetTextMetrics(hdc, &g_tm);
				g_dwDCOrg.x = 0;
				g_dwDCOrg.y = 0;
				bRecAllRect = FALSE;
				GetStringRect(hdc, (LPSTR)lpString, cbCount, X, Y,
					&g_rcTotalRect, lpDx);
				bRecAllRect = TRUE;

				if ((WindowFromDC != NULL)&&(WindowFromDC(hdc) == NULL))
				{
						g_dwDCOrg.x = 0;
						g_dwDCOrg.y = 0;
                    
						AddToTextOutBuffer(hdc, (LPSTR)lpString, cbCount,
							X, Y, lpDx);
				}
				else
				{
						GetDCOrgEx(hdc, &g_dwDCOrg);
    
						GetCurMousePosWord(hdc, (LPSTR)lpString, cbCount,
							X, Y, lpDx);
				}
			}
		}
	}
	// call ExtTextOutA
	ExtTextOutA(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);

	HookWin32Api(&g_ExtTextOutAHook, HOOK_NEED_CHECK);

	return TRUE;
}

DLLEXPORT BOOL WINAPI NHExtTextOutW(HDC hdc,
								    int X,
								    int Y,
								    UINT fuOptions,
								    CONST RECT *lprc,
									LPCWSTR lpString,
								    UINT cbCount,
								    CONST INT *lpDx)
{
	POINT pt;
	HWND  hWDC;
	HWND  hWPT;

	DWORD dwThreadIdWithPoint = 0;
	DWORD dwThreadIdCurr = 0;

	// restore
	RestoreWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "-> NHExtTextOutW : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("-> NHExtTextOutW : lpString(%s), cbCount(%d)\n", lpString, cbCount);

	pt.x = g_CurMousePos.x;
	pt.y = g_CurMousePos.y;
	hWDC = WindowFromDC(hdc);
	hWPT = WindowFromPoint(pt);

	// 01/17/2000
	// Fix Bug3: get word error when IE window overlaps.
	// Fix Bug3 begin

	dwThreadIdWithPoint = GetWindowThreadProcessId(hWPT, NULL);
	dwThreadIdCurr = GetCurrentThreadId();

	if(dwThreadIdWithPoint == dwThreadIdCurr)
	{
		// Fix Bug3 end

		if (hWDC == NULL || hWPT == hWDC
			|| IsParentOrSelf(hWPT, hWDC)
			|| IsParentOrSelf(hWDC, hWPT))
		{
			if ((g_bAllowGetCurWord) && (!IsBadReadPtr(lpString, cbCount))
				&& (cbCount > 0))
			{
	/*
				{
					//char cBuffer[0x100];
					//wsprintf(cBuffer, ">>>----> NHExtTextOutW : (%s) %d\n", lpTemp, cbCount);
					//OutputDebugString(cBuffer);					
				}
	*/
				g_nTextAlign = GetTextAlign(hdc);
				g_nExtra     = GetTextCharacterExtra(hdc);
				GetCurrentPositionEx(hdc, &g_CurPos);
				GetTextMetrics(hdc, &g_tm);
				g_dwDCOrg.x = 0;
				g_dwDCOrg.y = 0;
				bRecAllRect = FALSE;
				GetStringRectW(hdc, lpString, cbCount, X, Y,
					&g_rcTotalRect, lpDx);
				bRecAllRect = TRUE;

				//{DbgFilePrintf("--> NHExtTextOutW: lpTemp(%s)len(%d)\n", lpTemp, strlen(lpTemp));}
				//{DbgFilePrintf("--> NHExtTextOutW: X(%d)Y(%d), g_rcTotalRect(%d,%d,%d,%d)\n", X, Y, g_rcTotalRect.left, g_rcTotalRect.top, g_rcTotalRect.right, g_rcTotalRect.bottom);}

				if ((WindowFromDC != NULL)&&(WindowFromDC(hdc) == NULL))
				{
						g_dwDCOrg.x = 0;
						g_dwDCOrg.y = 0;

                    	// 01/19/2000
						// Fix Bug5: get word position error sometimes
						// Fix Bug5 begin

						AddToTextOutBufferW(hdc, lpString, 
							cbCount, X, Y, lpDx);

						// Fix Bug5 end
				}
				else
				{
						GetDCOrgEx(hdc, &g_dwDCOrg);

                    	// 01/19/2000
						// Fix Bug5: get word position error sometimes
						// Fix Bug5 begin
    
						GetCurMousePosWordW(hdc, lpString, 
							cbCount, X, Y, lpDx);

						// Fix Bug5 end
				}
			}
		}
	}

	// call ExtTextOutW
	ExtTextOutW(hdc, X, Y, fuOptions, lprc, lpString, cbCount, lpDx);

	HookWin32Api(&g_ExtTextOutWHook, HOOK_NEED_CHECK);

	return TRUE;
}

DLLEXPORT DWORD WINAPI BL_GetText32(LPSTR lpszCurWord, int nBufferSize, LPRECT lpWordRect)
{
	WORDRECT wr;
	DWORD dwCaretPlace;

	dwCaretPlace = BL_GetBuffer16(lpszCurWord, (short)nBufferSize, &wr);

	lpWordRect->left   = wr.left;   
	lpWordRect->right  = wr.right;  
	lpWordRect->top    = wr.top;    
	lpWordRect->bottom = wr.bottom;
/*
	{
		//char cBuffer[0x100];
		//wsprintf(cBuffer, "******BL_GetBuffer32 : (%s) %d %d %d %d\n", 
		//	lpszCurWord, wr.left, wr.top, wr.right, wr.bottom);
		//OutputDebugString(cBuffer);
	}
*/
	return dwCaretPlace;
}

DLLEXPORT DWORD WINAPI BL_GetBuffer16(LPSTR lpszBuffer, short nBufferSize, LPWORDRECT lpWr)
{
        int len;
		char* pcFirstSpacePos = NULL;	//position of first space
		char* pcTemp = NULL;
		int nSrc = 0, nDest = 0;

        if (!g_bMouseInTotalWord)
        {
            g_szTotalWord[0] = 0x00;
            g_nCurCaretPlaceInTotalWord = -1;
	    }

        if ((len = strlen(g_szTotalWord)) >= nBufferSize)
        {
            len = nBufferSize - 1;
        }  

		while ((g_szTotalWord[len - 1] == ' ') && (len > 0))
		{
			len--;
			g_szTotalWord[len] = 0x00;
		}

		if (g_szTotalWord[0] < 0)
		{
			strncpy(lpszBuffer, g_szTotalWord, len);
			lpszBuffer[len] = 0x00;
			lpWr->left   = g_TotalWordRect.left;
			lpWr->right  = g_TotalWordRect.right;
			lpWr->top    = g_TotalWordRect.top;
			lpWr->bottom = g_TotalWordRect.bottom;
		}
		else
		{
			if (g_szTotalWord[0] == ' ')
			{
				//this conditions should not happen.
				strncpy(lpszBuffer, g_szTotalWord, len);
				lpszBuffer[len] = 0x00;
			}
			else
			{
				while (nSrc < len)
				{
					lpszBuffer[nDest] = g_szTotalWord[nSrc];
					nDest++;
					nSrc++;
					
					if (g_szTotalWord[nSrc]	== ' ' && nSrc < len)
					{
						lpszBuffer[nDest] = g_szTotalWord[nSrc];
						nDest++;
						nSrc++;
						while (g_szTotalWord[nSrc]	== ' ' && nSrc < len)
						{
							nSrc++;
						}
					}
				}
			}

			//strncpy(lpszBuffer, g_szTotalWord, len);
			lpszBuffer[len] = 0x00;
			lpWr->left   = g_rcFirstWordRect.left;
			lpWr->right  = g_rcFirstWordRect.right;
			lpWr->top    = g_rcFirstWordRect.top;
			lpWr->bottom = g_rcFirstWordRect.bottom;
		}

        return (DWORD)g_nCurCaretPlaceInTotalWord;    
}

DLLEXPORT DWORD WINAPI BL_SetGetWordStyle(int nGetWordStyle)
{
	g_nGetWordStyle = nGetWordStyle;

	if (nGetWordStyle == GETPHRASE_ENABLE)
	{
		g_nWordsInPhrase = -1;
		g_bPhraseNeeded = TRUE;
	}
	else
	{
		g_nWordsInPhrase = -1;
		g_bPhraseNeeded = FALSE;
	}
	return 0L;
}

BOOL IsParentOrSelf(HWND hParent, HWND hChild)
{
	HWND hTemp = hChild;
	HWND hDesktop;
	
	if (hParent == NULL || hChild == NULL)
	{
		return FALSE;
	}

	hDesktop = GetDesktopWindow();
	while (hTemp != NULL && hTemp != hDesktop)
	{
		if (hTemp == hParent)
		{
			return TRUE;
		}

		hTemp = GetParent(hTemp);
	}

	return FALSE;
}

LRESULT CALLBACK GetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	ProcessWin32API();
	return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

LPSTR UnicodeToAnsi(LPTSTR lpString, UINT cbCount)
{
	UINT i;

	if (*lpString > 0)
	{
		for (i = 0; i < cbCount; i++)
		{
			*(lpString + i) = *(lpString + 2 * i);
		}
		*(lpString + cbCount) = '\0';
	}

	return (LPSTR)lpString;
}

//Added by ZHHN on 2000.4

// Fix Bug1: NHW32 can not be free from memory when nhstart.exe exit

DLLEXPORT BOOL WINAPI SetNHW32()
{
	if(g_hHook == NULL)
	{
		g_hHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, g_hinstDll, 0);
		if (g_hHook == NULL)
		{
			//MessageBox(NULL, __TEXT("Error hooking."), 
			//		   __TEXT("GetWord"), MB_OK);
			return FALSE;
		}
	}

	return TRUE;
}

DLLEXPORT BOOL WINAPI ResetNHW32()
{
	if (g_hHook != NULL)
	{
		if (!UnhookWindowsHookEx(g_hHook))
		{
			return FALSE;
		}

		g_hHook = NULL;
	}

	return TRUE;
}

// Fix Bug1 end

//Added end