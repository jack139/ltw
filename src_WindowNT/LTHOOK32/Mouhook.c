#include <stdio.h>
#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>
#include "windows.h"
#include "windowsx.h"
#include "ltwd.h"
#include "getwords.h"

#define MH_TIMER	3

HANDLE hDLLInst;
BOOL fMouseTrace;
extern DllImport BOOL fDoing; // also used by LTW32d.DLL
POINT pt;
HWND hMHWnd;
char s[0x1000] = {0};
HHOOK hHook=NULL;
DllImport HWND hDlg;
UINT g_nMHTimerID = 0;

void DllImport WINAPI makestr(char far *);			//function in ltwdll.c

void CALLBACK MH_TimerProc(HWND hwnd, UINT msg, UINT idTimer, DWORD dwTime)
{
	KillTimer(hMHWnd, MH_TIMER);
	g_nMHTimerID = 0;

	SetWindowPos(hMHWnd, (HWND)NULL, 
		0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);

	return;
}

void ResizeWin(char *s)
{
	int i, len, Width, Height=30, ScrW, max=0;
    HDC hdc;

	hdc = GetDC(hMHWnd);
	len = strlen(s);
	Width = 0;
	for (i=0; i<len; i++){
			SIZE size;
			if (s[i] & 0x80){
					i++;
					Width += 18;
					continue;
				}
			if ((s[i]=='\n') || (s[i]=='\r')){// s[i] = ' ';
				if (Width>max) max = Width;
				Width = 0;
				Height += 18;
			}
			GetTextExtentPoint32(hdc, s+i, 1, (LPSIZE)&size);
			Width += size.cx;

	}
	if (Width<max) Width = max;
	Width += 5;
	ScrW = GetDeviceCaps(hdc, HORZRES);
	if (pt.x+Width+8-10 > ScrW)		// if X > max 
		pt.x = ScrW - (Width + 8 - 10);
	if (pt.x-10 < 0) pt.x = 10;		// if x < 0 
	if (pt.y-Height-10  < 0){	// if Y < 0
		pt.y += (Height + 10 + 20);
//		pt.x += 25;
	}
	SetWindowPos(hMHWnd, HWND_TOP,
			pt.x - 10, pt.y - Height - 10, Width + 8, Height, SWP_NOACTIVATE);
	ReleaseDC(hMHWnd, hdc);

	PostMessage(hMHWnd, WM_PAINT, 0, 0);
	
	if (g_nMHTimerID) KillTimer(hMHWnd, MH_TIMER);
	g_nMHTimerID = SetTimer(hMHWnd, MH_TIMER, 2000, (TIMERPROC)MH_TimerProc);
}


void MH_OnPaint(HWND hwnd)
{
	HDC hdc;
	PAINTSTRUCT ps;

	hdc = BeginPaint(hwnd, &ps);

	if ((!fMouseTrace) || (!s[0])){
		EndPaint(hwnd, &ps);
		return;
	}

	SetBkColor(hdc, RGB(255, 255, 255));
	SetTextColor(hdc, RGB(255, 0, 0));
	if (!DrawHZ(hdc, 2, 1, s, 1000, RGB(0, 0, 255)))
		DisplayErr(FALSE);
	s[0] = '\0';
	EndPaint(hwnd, &ps);
}

void BeginTrace()
{
	POINT ptMousePos = {0, 0};

	if (GetCursorPos(&ptMousePos)){
		pt.x = ptMousePos.x;
		pt.y = ptMousePos.y;
		NHD_BeginGetWord(&ptMousePos);
	}
}

void MH_OnGetWordsOK(HWND hwnd)
{
	char tmp[256];
	TRANS t;
	WORD nP;
	int pl=0, i, j;

	if(NHD_CopyWordsTo(tmp, sizeof(tmp)))
	{
		// words in tmp

		fDoing = TRUE;
		
		makestr(tmp);
		for (nP=0; tmp[nP]; nP++) if (tmp[nP]==' ') pl++;
		pl++;

		for (i=pl; i>0; i--){
			lstrcpy(t.eng, tmp);
			if (translate(&t)){
				if (!stricmp(tmp, t.eng)) strcpy(s, tmp);
				else{
					for (j=strlen(tmp)-1; tmp[j]!=' ' && j>=0; j--);
					if (j>0){
						tmp[j]='\0';
						continue;
					}
					else{
						strcpy(s, tmp);
						strcat(s, " --> ");
						strcat(s, t.eng);
					}
				}
				strcat(s, "\r");
				strcat(s, t.chn);
				
				ResizeWin(s);
			}
		}
		
		fDoing = FALSE;

	}

}


long DllExport CALLBACK MHWndProc(HWND hwnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	switch (uMsg){
		case NHD_WM_GETWORD_OK:
			MH_OnGetWordsOK(hwnd);
			break;
		HANDLE_MSG(hwnd, WM_PAINT, MH_OnPaint);

		default:
			return (DefWindowProc(hwnd, uMsg, wParam, lParam));
	}
	return 0L;
}


/* -------------------- Hook WM_KEYDOWN ------------------------ */

/*      
void WINAPI WriteLogFile(UINT nKey) 
{ 
	HANDLE hFile; 
	DWORD dwBytesWrite=1; 
	char szTemp[100]; 
	
	hFile=CreateFile(".\\keydata.txt", 
		GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_WRITE, 
		NULL, 
		OPEN_ALWAYS, 
		FILE_ATTRIBUTE_HIDDEN, 
		NULL 
		); 
	
	SetFilePointer(hFile,0,NULL,FILE_END); 
	sprintf(szTemp, "%X ", nKey);
	WriteFile(hFile,szTemp,lstrlen(szTemp),&dwBytesWrite,0);
	CloseHandle(hFile); 
} 
*/

LRESULT CALLBACK JournalRecordProc(int nCode,WPARAM wParam,LPARAM lParam) 
{ 
	
	EVENTMSG *pMess=(EVENTMSG *)lParam; 
	
	switch(pMess->message) 
	{ 
	case WM_KEYDOWN: 
		if (LOBYTE(pMess->paramL) == VK_CONTROL){
			if (!(GetKeyState(LOBYTE(pMess->paramL))&0x800000000)){
//				WriteLogFile(pMess->paramL); 
//				WriteLogFile(pMess->paramH);
				BeginTrace();
			}
		}
		break; 
	} 
	
	return CallNextHookEx(hHook,nCode,wParam,lParam); 
} 


void WINAPI InstallHook(HINSTANCE hInstance) 
{ 
	if(hHook==NULL){
		hHook=SetWindowsHookEx(WH_JOURNALRECORD, 
			(HOOKPROC)JournalRecordProc, hInstance,0); 
	}
} 


void WINAPI UninstallHook() 
{ 
	if(hHook!=NULL){
		UnhookWindowsHookEx(hHook); 
		hHook=NULL;
	}
} 

/* ------------------------- Hook End -------------------------------- */

BOOL DllExport WINAPI IsTracing(void)
{
	return (fMouseTrace);
}


BOOL DllExport WINAPI SetTrace(HINSTANCE hInst)
{
	if (!NHD_InitGetWords(hInst, hMHWnd))
	{
		MessageBox(NULL, "NHD_InitGetWords error.",
			"Little Translator", MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
	fMouseTrace = TRUE;
	fDoing = FALSE;
	
	InstallHook(hInst);

	return TRUE;
}

void DllExport WINAPI StopTrace(void)
{
	fMouseTrace = FALSE;
	fDoing = FALSE;

	NHD_ExitGetWords();

	UninstallHook();

	SetWindowPos(hMHWnd, (HWND)NULL,
		0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);

	if (g_nMHTimerID) KillTimer(hMHWnd, MH_TIMER);
}

int DllExport WINAPI CreateMH(void)
{
	WNDCLASS wc;

	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hDLLInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName = NULL;
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = MHWndProc;
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
	wc.lpszClassName = "MHWndClass";
	if (!RegisterClass(&wc)){
		MessageBeep(MB_ICONQUESTION);
		MessageBox(NULL, "Application Initialized Error.",
			"Little Translator", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	hMHWnd = CreateWindowEx(
		WS_EX_DLGMODALFRAME | WS_EX_TOOLWINDOW, 
		"MHWndClass",
		NULL,
		WS_VISIBLE |  WS_POPUP, 
		0, 0, 5, 5,
		NULL, //hDlg,
		NULL,
		hDLLInst,
		NULL);
	SetWindowPos(hMHWnd, HWND_TOPMOST,
		0, 0, 0, 0, SWP_NOACTIVATE);

	return 1;
}

void DllExport WINAPI DestoryMH(void)
{
	DestroyWindow(hMHWnd);
	UnregisterClass("MHWndClass", hDLLInst);
}


BOOL WINAPI DllMain(HANDLE hInstance, DWORD fdwReason, LPVOID lpvResrved)
{
	switch (fdwReason){
	case DLL_PROCESS_ATTACH:
		hDLLInst = hInstance;

		fMouseTrace = FALSE;
		fDoing = FALSE;
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if (fMouseTrace){
			NHD_ExitGetWords();
		
			UninstallHook();

			if (g_nMHTimerID) KillTimer(hMHWnd, MH_TIMER);
		}
		break;
	}

	return TRUE;
}

