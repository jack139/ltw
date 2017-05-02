#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>
#include "windows.h"
#include "windowsx.h"
#include "ltwd.h"

#define ID_TIMER	0x100

typedef struct {
		DWORD ofs;
		WORD seg;
} FWORD;

HANDLE hDLLInst;
HANDLE hInstHook16;
BOOL fMouseTrace;
extern DllImport BOOL fDoing; // also used by LTW32d.DLL
POINT pt;
HWND hMHWnd;
TIMERPROC lpfnTimerProc;
char s[0x1000] = {0};
struct {
		LPSTR s;
		int nXStart;
        int cbString;
		POINT pt;
	} bu;  
DllImport HWND hDlg;

/*	16-bit KERNEL func	*/
HINSTANCE WINAPI LoadLibrary16( PSTR );
void WINAPI FreeLibrary16( HINSTANCE );
FARPROC WINAPI GetProcAddress16( HINSTANCE, PSTR );

/*	my 16-bit func	*/
FWORD CheckBuff16;
FWORD SetGDIHook16;
FWORD UnhookGDIHook16;
FWORD SetCurPos16;

void DllImport WINAPI makestr(char far *);			//function in ltwdll.c

////////////////////// thunk to LTHOOK16.DLL ///////////////////////////
void SetGDIHook(void)	
{
	_asm{
		pusha
		call fword ptr [SetGDIHook16]
		popa
	}
}

void UnhookGDIHook(void)
{
	_asm{
		pusha
		call fword ptr [UnhookGDIHook16]
		popa
	}
}

DWORD CheckBuff(void)	
{
	BOOL r;

	_asm{
		pusha
		call fword ptr [CheckBuff16]
		mov		r, eax
		mov		word ptr [bu.cbString], bx
		mov		word ptr [bu.nXStart], cx
		mov		word ptr [bu.pt.x], dx
		shr		edx, 16
		mov		word ptr [bu.pt.y], dx
		popa
	}
	return r;
}

void SetCurPos(POINT *p)
{
	_asm{
		pusha
		mov		ebx, dword ptr [p]
		mov		dx, word ptr [ebx+4]
		shl		edx, 16
		mov		dx, word ptr [ebx]
		call fword ptr [SetCurPos16]
		popa
	}
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
}



void CALLBACK TimerProc(HWND hwnd, UINT msg, UINT idTimer, DWORD dwTime)
{
	POINT p;
	HWND hwndPtIn;
	static POINT xp={-1, -1};
	WORD nP, nL;
    RECT rc;
	TRANS t;
	char tmp[0x1000];
	LPSTR lpSharedBuf;
	static int dull=0;

	if (!dull) lpSharedBuf = (LPSTR)CheckBuff();
	if (lpSharedBuf && !dull){
		bu.s = lpSharedBuf;
		nL = bu.nXStart;


		for (nP=0;
			 (nP<0x30) 	
			 && (bu.s[nL+nP])
			 && (IsCharAlpha(bu.s[nL+nP]) || bu.s[nL+nP]==' ');
			 nP++); // the string in buf only include Englih chars and blank

/* 	because in ECLIB.DAT the items' max-length is 41 chars
	here to cut the buf to search proper words or phrase		*/

        if (nP > 0x100) nP = 0x100;
		bu.s[nL + nP] = '\0';

		if (nL<bu.cbString){			

			int pl=0,			// how many words in buf
                i, j;
//////  add for debug //////////////////////////////////////			
			if (lstrlen(bu.s+nL)>0x1000){
//				MessageBeep(MB_ICONQUESTION);
//				MessageBox(NULL, "Warning: out of buffer in LTHOOK32.",
//					"Little Translator", MB_OK | MB_ICONEXCLAMATION);
				*(bu.s+nL+0x1000-1) = '\0';
			}
///////////////////////////////////////////////////////////////////////
			lstrcpy(tmp, bu.s+nL);
            fDoing = TRUE;

			makestr(tmp);
			for (nP=0; tmp[nP]; nP++)
				if (tmp[nP]==' ') pl++;
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
		lpSharedBuf[0] = '\0';
        return;
	}

	GetCursorPos(&p);
	if ((xp.x==p.x) && (xp.y==p.y)){
		if (dull<1000) dull++;
		if (dull==20) SetWindowPos(hMHWnd, (HWND)NULL,
						0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
		return;
	}
	else {
		dull = 0;
		xp.x = p.x;
		xp.y = p.y;
	}
	hwndPtIn = WindowFromPoint(p);
	if (hwndPtIn == hMHWnd)	return;

	pt.x = p.x;
	pt.y = p.y;                          
	SetCurPos(&pt);
	ScreenToClient(hwndPtIn, &p);
	SetRect(&rc, p.x-5, p.y-5, p.x+5, p.y+5);
	InvalidateRect(hwndPtIn, &rc, TRUE);
	if ((p.x<0) || (p.y<0))
		PostMessage(hwndPtIn, WM_NCPAINT, 1, 0);
	else
		PostMessage(hwndPtIn, WM_PAINT, 0, 0);
	SetWindowPos(hMHWnd, (HWND)NULL,
		0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
}


void SetMouseHook(void)
{
	lpfnTimerProc = (TIMERPROC) MakeProcInstance((FARPROC)TimerProc, hDLLInst);
	SetTimer(hMHWnd, ID_TIMER, 200, lpfnTimerProc);
	SetGDIHook();
}

void UnhookMouseHook(void)
{
	UnhookGDIHook();
	KillTimer(hMHWnd, ID_TIMER);
	FreeProcInstance((TIMERPROC)lpfnTimerProc);
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

    UnhookGDIHook();
	SetBkColor(hdc, RGB(192, 192, 192));
	SetTextColor(hdc, RGB(255, 0, 0));
	if (!DrawHZ(hdc, 2, 1, s, 1000, RGB(255, 0, 0)))
		DisplayErr(FALSE);
	s[0] = '\0';
	EndPaint(hwnd, &ps);
	SetGDIHook();
}


long DllExport CALLBACK MHWndProc(HWND hwnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	switch (uMsg){
		HANDLE_MSG(hwnd, WM_PAINT, MH_OnPaint);

		default:
			return (DefWindowProc(hwnd, uMsg, wParam, lParam));
	}
	return 0L;
}

BOOL DllExport WINAPI IsTracing(void)
{
	return (fMouseTrace);
}

void DllExport WINAPI SetTrace(void)
{
	SetMouseHook();
	fMouseTrace = TRUE;
	fDoing = FALSE;
}

void DllExport WINAPI StopTrace(void)
{
	fMouseTrace = FALSE;
	fDoing = FALSE;
	UnhookMouseHook();

	SetWindowPos(hMHWnd, (HWND)NULL,
		0, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE);
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
	wc.hbrBackground = GetStockObject(LTGRAY_BRUSH);
	wc.lpszClassName = "MHWndClass";
	if (!RegisterClass(&wc)){
		MessageBeep(MB_ICONQUESTION);
		MessageBox(NULL, "Application Initialized Error.",
			"Little Translator", MB_OK | MB_ICONEXCLAMATION);
		return 0;
	}

	hMHWnd = CreateWindowEx(
		WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE,
		"MHWndClass",
		NULL,
		WS_VISIBLE | WS_BORDER | WS_POPUP,
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
	DWORD	fn;

	switch (fdwReason){
	case DLL_PROCESS_ATTACH:
		hDLLInst = hInstance;

		fMouseTrace = FALSE;
		fDoing = FALSE;
///////////////////////////////////////
		hInstHook16 = LoadLibrary16("LTHOOK16.DLL");
		if ( hInstHook16 < (HINSTANCE)32 )
		{
			MessageBeep(MB_ICONQUESTION);
			MessageBox(NULL, "Error: cannot load LTHOOK16.DLL.",
				"Little Translator", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		
// Checkbuff16
		fn = (DWORD) GetProcAddress16(hInstHook16, "CHECKBUFF");
		if ( !fn )
		{
			MessageBeep(MB_ICONQUESTION);
			MessageBox(NULL, "Error: cannot get CheckBuff16 from LTHOOK16.DLL.",
				"Little Translator", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		else{
			CheckBuff16.seg = HIWORD(fn);
			CheckBuff16.ofs = (DWORD)LOWORD(fn);
		}

// SetGDIHook16
		fn = (DWORD) GetProcAddress16(hInstHook16, "SETGDIHOOK");
		if ( !fn )
		{
			MessageBeep(MB_ICONQUESTION);
			MessageBox(NULL, "Error: cannot get SetGDIHook16 from LTHOOK16.DLL.",
				"Little Translator", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		else{
			SetGDIHook16.seg = HIWORD(fn);
			SetGDIHook16.ofs = (DWORD)LOWORD(fn);
		}

// UnhookGDIHook16
		fn = (DWORD) GetProcAddress16(hInstHook16, "UNHOOKGDIHOOK");
		if ( !fn )
		{
			MessageBeep(MB_ICONQUESTION);
			MessageBox(NULL, "Error: cannot get UnhookGDIHook16 from LTHOOK16.DLL.",
				"Little Translator", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		else{
			UnhookGDIHook16.seg = HIWORD(fn);
			UnhookGDIHook16.ofs = (DWORD)LOWORD(fn);
		}

// SetCurPos16
		fn = (DWORD) GetProcAddress16(hInstHook16, "SETCURPOS");
		if ( !fn )
		{
			MessageBeep(MB_ICONQUESTION);
			MessageBox(NULL, "Error: cannot get SetCurPos from LTHOOK16.DLL.",
				"Little Translator", MB_OK | MB_ICONEXCLAMATION);
			return FALSE;
		}
		else{
			SetCurPos16.seg = HIWORD(fn);
			SetCurPos16.ofs = (DWORD)LOWORD(fn);
		}

///////////////////////////////////////////
		break;

	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		if (fMouseTrace){
			UnhookMouseHook();
		}
		FreeLibrary16( hInstHook16 ); 
		break;
	}

	return TRUE;
}

