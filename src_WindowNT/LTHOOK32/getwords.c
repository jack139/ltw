//***************************************************************************//
//File Name : getwords.c
//Content   : get words demo using nhw32.dll
//Version   : 0.01
//Date      : 2000/7/15
//Aurthor   : ITC RD-VIOLET1
//***************************************************************************//

#include <assert.h>
#include "getwords.h"

//-----------------------------------------------------------------------------
// Gloable vars:
//-----------------------------------------------------------------------------

HINSTANCE	g_hGetWordInst = NULL;
BOOL	g_bInGetWord = FALSE;
char	g_TextBuffer[NHD_MAX_TEXTLEN];
RECT	g_WordRect;
UINT	g_WM_GetWordOk;
UINT	g_nGWTimerID = 0;
HWND	g_hNHMainWin = NULL;
HWND	g_hFlyWin = NULL;

/*----------------------------------------------------------------------------

	nhw32.dll exports two function mainly:
    ******************************************************************
	DWORD WINAPI BL_SetFlag32(UINT nFlag,
	                          HWND hNotifyWnd,
	                          int MouseX, 
	                          int MouseY
							  )
	functions:
		Start or Stop getting word.
	parameters:
		nFlag 
			[in] specify the following values
				GETWORD_ENABLE: start getting word. set this flag before repaint region where
								the	word is	around. In order to get words, need to repaint 
								region where the word is around.
				GETWORD_DISABLE: stop getting word.
		hNotifyWnd 
			[in] handle to be notified window. when the word has been got, send a registered
				GWMSG_GETWORDOK message to window which handle is hNotifyWnd.
		MouseX 
			[in] Specify the x-coordinate of point where the word is around.
		MouseY 
			[in] Specify the y-coordinate of point where the word is around.
	return values:
		can be ignored.
	==============================================================================
	DWORD WINAPI BL_GetText32(LPSTR lpszCurWord, 
	                          int nBufferSize, 
	                          LPRECT lpWordRect
							  )
	functions:
		get words text string from internal buffer.
	Parameters:
		lpszCurWord
			[in] pointer to destination buffer to which copy words got.
		nBufferSize
			[in] size of destination buffer.
		lpWordRect
			[out] pointer to RECT struct where defines the coordinates of the upper-left and 
			lower-right corners of a corresponding word's rectangle.
	Return values:
		Current Caret place in total word.

	**************************************************************************
	The NT version nhw32.dll specially exports other two function beside 
	above two function:
	**************************************************************************

	BOOL WINAPI SetNHW32()
	functions:
		initial function only valid in Win NT/2000 environment.
	return values:
		TRUE if successful, FALSE if failed.
	===========================================================================
	BOOL WINAPI ResetNHW32()
	functions:
		only valid in Win NT/2000 environment, must be called before exit programm.
	return values:
		TRUE if successful, FALSE if failed.
	=============================================================================

  note: The WinNT/2000 version nhw32.dll is different from Win95/98 version nhw32.dll, but
		BL_SetFlag32 and BL_GetText32 - two function's interface is same.

-------------------------------------------------------------------------------*/

//-----------------------------------------------------------------------------
// import functions from nhw32.dll
//-----------------------------------------------------------------------------

typedef DWORD (WINAPI *BL_SETFLAG32)(UINT nFlag, HWND hNotifyWnd, int MouseX, int MouseY);
typedef DWORD (WINAPI *BL_GETTEXT32)(LPSTR lpszCurWord, int nBufferSize, LPRECT lpWordRect);

BL_GETTEXT32	fpBL_GetText32 = NULL;
BL_SETFLAG32	fpBL_SetFlag32 = NULL;

//-----------------------------------------------------------------------------
// internal functions called from this file
//-----------------------------------------------------------------------------

void NHD_FreeLoadedLib(void);
void CALLBACK NHD_GetWordTimerProc(HWND hwnd, UINT msg, UINT idTimer, DWORD dwTime);
HWND NHD_CreateWindow(HINSTANCE hInst);
BOOL NHD_DestroyWindow(void);
BOOL NHD_LoadGetWordLib(void);
LRESULT CALLBACK NHD_FlyWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
VOID dbgPrintf(LPTSTR fmt, ...);

//-----------------------------------------------------------------------------
// function used to output debug messagebox
//-----------------------------------------------------------------------------

VOID dbgPrintf(LPTSTR fmt, ...)
{
		va_list marker;
		TCHAR szBuf[1024];

		va_start(marker, fmt);
		wvsprintf(szBuf, fmt, marker);
		va_end(marker);

		OutputDebugString(szBuf);
		OutputDebugString(TEXT("\r\n"));
}

#ifdef _DEBUG
	#define DbgPrintf  dbgPrintf
#else
	#define DbgPrintf  1 ? ((void)0) : dbgPrintf
#endif //_DEBUG



//-----------------------------------------------------------------------------
// load nhw32.dll
//-----------------------------------------------------------------------------

BOOL NHD_LoadGetWordLib(void)
{
	typedef BOOL (WINAPI *SETNHW32)();
	SETNHW32 SetNHW32 = NULL;

	g_hGetWordInst = LoadLibrary("nhw32.dll");
	if (!g_hGetWordInst)
	{
		DbgPrintf("NHD_LoadGetWordLib loading error!\n") ;	
		return FALSE; 
	}

	fpBL_GetText32 = (BL_GETTEXT32)GetProcAddress(g_hGetWordInst, "BL_GetText32");
	if (!fpBL_GetText32)
	{
		return FALSE;
	}

	fpBL_SetFlag32 = (BL_SETFLAG32)GetProcAddress(g_hGetWordInst, "BL_SetFlag32");
	if (!fpBL_SetFlag32)
	{
		return FALSE;
	}

	//only valid in windows NT environment

	SetNHW32 = GetProcAddress(g_hGetWordInst, "SetNHW32");
	if(SetNHW32)
	{
		if(!SetNHW32())
		{
			DbgPrintf("Unable to Set nhw32!");
			return FALSE;
		}
	}
	
	return TRUE;
}	

//-----------------------------------------------------------------------------
// free nhw32.dll
//-----------------------------------------------------------------------------

void NHD_FreeLoadedLib(void)
{
	if (g_hGetWordInst)
	{
		//only valid in windows NT enviroment

		typedef BOOL (WINAPI *RESETNHW32)();
		RESETNHW32 ResetNHW32 = NULL;
		ResetNHW32 = GetProcAddress(g_hGetWordInst, "ResetNHW32");
		if(ResetNHW32)
		{
			ResetNHW32();
		}

		FreeLibrary(g_hGetWordInst);	
		g_hGetWordInst = NULL;
	}

	return ;
}

//-----------------------------------------------------------------------------
// auxiliary window process function
//-----------------------------------------------------------------------------

LRESULT CALLBACK NHD_FlyWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    //Unhook textout when reveived msg from getword;
	if (msg == g_WM_GetWordOk)
	{
		if (g_bInGetWord)
		{
			g_bInGetWord = FALSE;
			assert(g_nGWTimerID != 0);
			KillTimer(g_hFlyWin, NHD_GETWORD_TIMER);
			g_nGWTimerID = 0;
			
			//UnHook TextOut;
			fpBL_SetFlag32(GETWORD_DISABLE, NULL, 0, 0);
			
			//get word on cursor pos;
			if (wParam == 0)
			{
				//get word from 16BIT API HOOK for NORMAL App;
				fpBL_GetText32(g_TextBuffer, NHD_MAX_TEXTLEN, &g_WordRect);
			}
			
			PostMessage(g_hNHMainWin, NHD_WM_GETWORD_OK, 0, 0);   
			
			return (0);
		}
	}

    return (DefWindowProc(hWnd, msg, wParam, lParam));
}

//-----------------------------------------------------------------------------
// create auxiliary window used to help get text on screen
//-----------------------------------------------------------------------------

HWND NHD_CreateWindow(HINSTANCE hInst)
{
	HWND      hwnd;
	WNDCLASS  wc;

	if (hInst == NULL)
	{
		return NULL;
	}

	//create a very small window, cause AP paint by moving it;
	wc.lpszMenuName     = NULL;
	wc.lpszClassName    = "NHD_FLYWIN_DEMO";
    wc.hInstance        = hInst;
	wc.hIcon	        = NULL;
    wc.hCursor          = NULL;
	wc.hbrBackground    = NULL;
    wc.style            = WS_EX_TOPMOST;
	wc.lpfnWndProc	    = NHD_FlyWndProc;
    wc.cbClsExtra       = 0;
    wc.cbWndExtra       = 0;

    RegisterClass(&wc);

	hwnd = CreateWindowEx (WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
						   "NHD_FLYWIN_DEMO",
						   "NHD_FlyWindow_Demo",
						   WS_POPUP | WS_VISIBLE,
						   NHD_WIN_INITPOSX,
						   NHD_WIN_INITPOSY,
						   NHD_FLYWIN_WIDTH,
						   NHD_FLYWIN_HEIGHT,
						   NULL,  
						   NULL, 
						   hInst,
						   NULL);
    
	return hwnd;
}

//-----------------------------------------------------------------------------
// destroy auxiliary window
//-----------------------------------------------------------------------------

BOOL NHD_DestroyWindow()
{
	if (g_hFlyWin)
	{
		DestroyWindow(g_hFlyWin);
		g_hFlyWin = NULL;
	}

	return TRUE;
}

//-----------------------------------------------------------------------------
// initial function called once before getting words
//-----------------------------------------------------------------------------

HWND NHD_InitGetWords(HINSTANCE hInst, HWND hwnd)
{
	//save NH main window to send run time error messages:
	g_hNHMainWin = hwnd;

	if (!NHD_LoadGetWordLib())
	{	
		NHD_FreeLoadedLib();
		return NULL;
	}

	//Create fly_window (cause paint) and show text window;
	g_hFlyWin = NHD_CreateWindow(hInst);
	if (!g_hFlyWin)
	{
		NHD_FreeLoadedLib();
		return NULL; 
	}

	g_WM_GetWordOk = RegisterWindowMessage(GWMSG_GETWORDOK);
	if (!g_WM_GetWordOk)
	{
		NHD_FreeLoadedLib();
		return NULL;
	}

	return g_hFlyWin;
}

//-----------------------------------------------------------------------------
// deinit function called when not get words any more 
//-----------------------------------------------------------------------------

BOOL NHD_ExitGetWords(void)
{
	//free libarys:
	NHD_FreeLoadedLib();

	NHD_DestroyWindow();

	return TRUE;
}

//-----------------------------------------------------------------------------
// timer process callback function used to get text from buffer
//-----------------------------------------------------------------------------

void CALLBACK NHD_GetWordTimerProc(HWND hwnd, UINT msg, UINT idTimer, DWORD dwTime)
{
	assert (g_nGWTimerID != 0);

	//may be proior finished by Getword message;
	if (g_bInGetWord)
	{
		g_bInGetWord = FALSE;
		//UnHook TextOut;
		fpBL_SetFlag32(GETWORD_DISABLE, NULL, 0, 0);
		fpBL_GetText32(g_TextBuffer, NHD_MAX_TEXTLEN, &g_WordRect);
	}
	
	KillTimer(g_hFlyWin, NHD_GETWORD_TIMER);
	g_nGWTimerID = 0;

	PostMessage(g_hNHMainWin, NHD_WM_GETWORD_OK, 0, 0);   
	
	return;
}

//-----------------------------------------------------------------------------
// begin get words
//-----------------------------------------------------------------------------

void NHD_BeginGetWord(POINT *ptMousePos)
{
	char szAppClassName[NHD_CLASSNAME_LEN + 1];
	HWND hAppWin;
    int  nFlyWinLeft;
	int  nFlyWinWidth;
	RECT rcAppWin;
	
	//get window from mouse point;
	hAppWin = WindowFromPoint(*ptMousePos);

	//check if the app window is EDIT, if it is, redraw whole line;
	GetClassName(hAppWin, szAppClassName, NHD_CLASSNAME_LEN);

	DbgPrintf("hAppWin: %x\n", hAppWin);
	DbgPrintf("ClassName: %s\n", szAppClassName);
	
	// special window class
	if (stricmp(szAppClassName, "Edit") == 0 ||						//NotePad
		stricmp(szAppClassName, "Internet Explorer_Server") == 0 || //IE4.0
		stricmp(szAppClassName, "RichEdit") == 0 ||					//
		stricmp(szAppClassName, "RichEdit20A") == 0 ||				//WordPad 
		stricmp(szAppClassName, "RichEdit20W") == 0 ||				//WordPad 
		stricmp(szAppClassName, "HTML_Internet Explorer") == 0 ||	//IE3.0
		stricmp(szAppClassName, "ThunderTextBox") == 0 ||			//VB Edit
		stricmp(szAppClassName, "ThunderRT5TextBox") == 0 ||		//VB Edit
		stricmp(szAppClassName, "ThunderRT6TextBox") == 0 ||		//VB Edit
		stricmp(szAppClassName, "EXCEL<") == 0 ||					//Excel 2000
		stricmp(szAppClassName, "EXCEL7") == 0 ||					//Excel 2000
		stricmp(szAppClassName, "EXCEL6") == 0 ||					//Excel 2000
		stricmp(szAppClassName, "ConsoleWindowClass") == 0 ||		//NT V86
		stricmp(szAppClassName, "_WwG") == 0 ||
		stricmp(szAppClassName, "tty") == 0 ||
		stricmp(szAppClassName, "ttyGrab") == 0)					//Word97
	{
		GetWindowRect(hAppWin, &rcAppWin);
		nFlyWinLeft = rcAppWin.left - 4;
		nFlyWinWidth = rcAppWin.right - rcAppWin.left - 8;
		
		//don't not repaint whole line if too long;
		if ((ptMousePos->x - nFlyWinLeft) > 200)
		{
			nFlyWinLeft = ptMousePos->x - 200;
		}

		DbgPrintf("!!!!tty window");
	}
	else
	{
		nFlyWinLeft = ptMousePos->x;
		nFlyWinWidth = NHD_FLYWIN_WIDTH;
	}

	//note: move the flywin to cursor pos "x - 1" to aviod mouse shape changing between ARROW and EDIT in edit area; 
	//use SetWindowPos instead of MoveWindow, for MoveWindow can not make menu item redraw.
	SetWindowPos(g_hFlyWin, HWND_TOPMOST, 
			     nFlyWinLeft, 
				 ptMousePos->y - 1 ,
				 nFlyWinWidth,
				 NHD_FLYWIN_HEIGHT,
				 SWP_NOACTIVATE | SWP_NOREDRAW);
	
	//set flag to avoid re-entry;
	g_bInGetWord = TRUE;

	//hook TextOut;
	fpBL_SetFlag32(GETWORD_ENABLE, g_hFlyWin, ptMousePos->x, ptMousePos->y);

	MoveWindow(g_hFlyWin, -1, -1, NHD_FLYWIN_WIDTH, NHD_FLYWIN_HEIGHT, TRUE); 		

	DbgPrintf("ptMousePos(%d, %d)\n", ptMousePos->x, ptMousePos->y);

	assert (g_nGWTimerID == 0);

	g_nGWTimerID = SetTimer(g_hFlyWin, NHD_GETWORD_TIMER, NHD_GW_WAITING_TIME, (TIMERPROC)NHD_GetWordTimerProc);

	assert (g_nGWTimerID != 0);
}

//-----------------------------------------------------------------------------
// copy words in buffer to destination
//-----------------------------------------------------------------------------

BOOL NHD_CopyWordsTo(char *szBuffer, int nBufferSize)
{
	int nLen = strlen(g_TextBuffer);
	assert(szBuffer);

	if(nLen + 1 > nBufferSize)
	{
		return FALSE;
	}

	memset(szBuffer, 0, nBufferSize);
	memcpy(szBuffer, g_TextBuffer, nLen);

	return TRUE;
}
