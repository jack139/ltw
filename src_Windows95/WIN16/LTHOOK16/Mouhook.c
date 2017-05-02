#include <stdlib.h>
#include <dos.h>
#include <string.h>
#include <ctype.h>
#include "windows.h"
#include "windowsx.h"

#define GET_PROC(mod, func)	\
	GetProcAddress(GetModuleHandle(mod), func)

#define	THUNK_ENTER	\
	static char stack[8192];	\
	static WORD stack_seg;	\
	static WORD prev_seg;	\
	static DWORD prev_ofs;	\
	static WORD prev_ds;	\
	_asm{	\
		push ax;	\
		push bx;	\
		mov ax, ds;	\
		mov bx, seg dummy;	\
		mov ds, bx;	\
		mov stack_seg, bx;	\
		mov prev_ds, ax;	\
		pop bx;	\
		pop ax;	\
		mov prev_seg, ss;	\
		mov dword ptr prev_ofs, esp;	\
		mov ss, stack_seg;	\
		mov sp, offset stack;	\
		add sp, 8192;	\
	};

#define THUNK_LEAVE \
	_asm{	\
		mov ss, prev_seg;	\
		mov esp, dword ptr prev_ofs;	\
		mov ds, prev_ds;	\
		lea sp, word ptr [bp-2];	\
        pop ds;	\
		pop	bp;	\
        dec bp;	\
		db 66h;	\
		retf;	\
	};    	

int dummy;
DWORD ll;
HINSTANCE hDLLInst;
BOOL HookNow;
POINT pt, tmpPt;
BYTE far *lpTextOut;
BYTE far *lpExtTextOut;
BYTE OldCode[2][5],
	 NewCode[2][5];
WORD wGDISel, wGDICode;
WORD nP, nL, nT;
void far *lpfn0;
void far *lpfn1;
char tmp[0x100];
struct {
		POINT pt;
		HGLOBAL h, b;
		char far *s;
		int nXStart;
        UINT cbString;
	} bu;  


int _export WINAPI LibMain(HINSTANCE hInstance, WORD wDataSeg,
	WORD cbHeapSize, LPSTR lpszCmdLine)
{
	int i;
	WORD (FAR PASCAL *AllocCStoDSAlias)(WORD);

	hDLLInst = hInstance;
    HookNow = FALSE;

	AllocCStoDSAlias = GET_PROC("KERNEL", "ALLOCCSTODSALIAS");
	lpfn0 = GET_PROC("GDI", "TEXTOUT");
	lpfn1 = GET_PROC("GDI", "EXTTEXTOUT");
	wGDICode = FP_SEG(lpfn0);
    GlobalPageLock(wGDICode);
	wGDISel = AllocCStoDSAlias(wGDICode);
	lpTextOut = MK_FP(wGDISel, FP_OFF(lpfn0));
	lpExtTextOut = MK_FP(wGDISel, FP_OFF(lpfn1));
       
	for (i=0; i<5; i++){
		OldCode[0][i] = *(lpTextOut + i);
		OldCode[1][i] = *(lpExtTextOut + i);
	}
	NewCode[0][0] = NewCode[1][0] = 0x9a;		// far call instructor
	lpfn0 = MakeProcInstance(GET_PROC("LTHOOK16", "TEXTOUTHOOK"), hInstance);
	lpfn1 = MakeProcInstance(GET_PROC("LTHOOK16", "EXTTEXTOUTHOOK"), hInstance);
	*(DWORD far *)(NewCode[0] + 1) = (DWORD)lpfn0;
	*(DWORD far *)(NewCode[1] + 1) = (DWORD)lpfn1;

	if (!(bu.h = GlobalAlloc(GPTR, 2))){
		MessageBeep(MB_ICONQUESTION);
		MessageBox(NULL, "Error: no enough memory.",
		"Little Translator",
		MB_OK | MB_ICONEXCLAMATION);
		return (0);
    }

	bu.s = GlobalLock(bu.h);	 
	bu.s[0] = '\0';
	GlobalUnlock(bu.h);

	return (1);
}

////////////////////////// 32-bit DLL thunk here /////////////////////////////////
void _export WINAPI SetGDIHook(void)
{
	register int i;

	THUNK_ENTER
	HookNow = TRUE;
    for(i=0; i<5; i++){
		*(lpTextOut + i) = NewCode[0][i];
		*(lpExtTextOut + i) = NewCode[1][i];
	}
    THUNK_LEAVE
}

void _export WINAPI UnhookGDIHook(void)
{
	register int i;

	THUNK_ENTER
	HookNow = FALSE;
	for(i=0; i<5; i++){
		*(lpTextOut + i) = OldCode[0][i];
		*(lpExtTextOut + i) = OldCode[1][i];
	}
    THUNK_LEAVE
}

void _export WINAPI CheckBuff(void)
{
	THUNK_ENTER
	bu.s = GlobalLock(bu.h);

	ll = 0;
	if (bu.s[0]) ll = GetSelectorBase(HIWORD((DWORD)bu.s));
	GlobalUnlock(bu.h);		
	_asm{
		mov eax, dword ptr ll
		mov bx, bu.cbString
		mov cx, bu.nXStart
		mov edx, dword ptr [bu]
	}
    
	THUNK_LEAVE
}

void _export WINAPI SetCurPos(void)
{
	THUNK_ENTER
	_asm mov dword ptr [pt], edx
	THUNK_LEAVE
}

/////////////////////////////////////////////////////////////////////////////////

void Priv_SetGDIHook(void)
{
	int i;

	HookNow = TRUE;
    for(i=0; i<5; i++){
		*(lpTextOut + i) = NewCode[0][i];
		*(lpExtTextOut + i) = NewCode[1][i];
	}
}

void Priv_UnhookGDIHook(void)
{
	int i;

	HookNow = FALSE;
	for(i=0; i<5; i++){
		*(lpTextOut + i) = OldCode[0][i];
		*(lpExtTextOut + i) = OldCode[1][i];
	}
}

BOOL _export CALLBACK TextOutHook(void)
{
	BOOL fResult;
	HDC hDC;
	int nXStart, nYStart, cbString;
	DWORD lpszString, dwTextSize, lpRet;
	WORD hi, lo;
	RECT rc;
	POINT ptx;
	DWORD xy;
	int x, y;

	_asm mov ax, [bp+20]
	_asm mov hDC, ax                // para hDC
	_asm mov ax, [bp+18]
	_asm mov nXStart, ax			// para nXStart
	_asm mov ax, [bp+16]
	_asm mov nYStart, ax			// para nYStart

	_asm mov ax, [bp+14]
	_asm mov hi, ax
	_asm mov ax, [bp+12]
	_asm mov lo, ax
	lpszString = MAKELONG(lo, hi);	// para lpszString

	_asm mov ax, [bp+10]
	_asm mov cbString, ax			// para cbString

	_asm mov ax, [bp+8]
	_asm mov [bp+20], ax
	_asm mov ax, [bp+6]
	_asm mov [bp+18], ax			// return address

	Priv_UnhookGDIHook();
	fResult = TextOut(hDC, nXStart, nYStart, (LPSTR)lpszString, cbString);
	xy = GetDCOrg(hDC);
	x = LOWORD(xy);
	y = HIWORD(xy);

//////////////

	dwTextSize = GetTextExtent(hDC, (LPSTR)lpszString, cbString);

	tmpPt.x = nXStart;
	tmpPt.y = nYStart;			//  now nXStart & nYStart is logical coordinate
	LPtoDP(hDC, (LPPOINT)&tmpPt, 1);
	nXStart = tmpPt.x;
	nYStart = tmpPt.y;          //  now nXStart & nYStart is device coordinate

	SetRect(&rc, nXStart, nYStart,
		nXStart + LOWORD(dwTextSize),
		nYStart + HIWORD(dwTextSize));			

	ptx.x = pt.x - x;
	ptx.y = pt.y - y;			// device coordinate


	if (PtInRect(&rc, ptx)){
			bu.b = GlobalAlloc(GPTR, lstrlen(lpszString)+2);
			if (!bu.b){
				MessageBeep(MB_ICONQUESTION);
				MessageBox(NULL, "Error: no enough memory.",
				"Little Translator",
				MB_OK | MB_ICONEXCLAMATION);
				bu.s = GlobalLock(bu.h);
				bu.s[0] = '\0';
				GlobalUnlock(bu.h);
			}
            else{
	            GlobalFree(bu.h);
				bu.h = bu.b;
				bu.s =GlobalLock(bu.h);
				lstrcpy(bu.s , lpszString);
///////////// add ////////////////////////////////////////
				if (bu.s[0]){
					nL = nT = 0;
					for (nP=0; (bu.s[nL+nP]) && (nL<cbString); nP++){
						if (!IsCharAlpha(bu.s[nL+nP])){
							nL += nP;
							nP = 0;
                            nXStart += nT;
						}
						_fstrncpy(tmp, bu.s+nL, nP+1);
                        tmp[nP+1] = '\0';
						nT = LOWORD(GetTextExtent(hDC, tmp, nP+1));
						if ((nXStart+nT) > ptx.x){
							nXStart += nT;
							break;
                        }
					}
					if (nP && (!IsCharAlpha(bu.s[nL]))) nL++;
					nXStart = nL;
				}

///////////////////////////////////////////////////////////
				GlobalUnlock(bu.h);
            }
			bu.pt.x = ptx.x;
			bu.pt.y = ptx.y;
			bu.nXStart = nXStart;
			bu.cbString = cbString;
	}

	Priv_SetGDIHook();

	_asm mov ax, fResult
	_asm lea sp, [bp-02]
	_asm pop ds
	_asm pop bp
	_asm dec bp
	_asm add sp, 16
	_asm retf

	return fResult;
}


BOOL _export CALLBACK ExtTextOutHook(void)
{
	BOOL fResult;
	HDC hDC;
	int nXStart, nYStart;
	UINT fuOptions, cbString;
	DWORD lprc, lpszString, lpDx, lpRet, dwTextSize;
	WORD lo, hi;
	RECT rc;
	POINT ptx;
	DWORD xy;
	int x, y;

	    	
	_asm mov ax, [bp+30]
	_asm mov hDC, ax                // para hDC
	_asm mov ax, [bp+28]
	_asm mov nXStart, ax			// para nXStart
	_asm mov ax, [bp+26]
	_asm mov nYStart, ax			// para nYStart
	_asm mov ax, [bp+24]
	_asm mov fuOptions, ax			// para fuOptinons

	_asm mov ax, [bp+22]
	_asm mov hi, ax
	_asm mov ax, [bp+20]
	_asm mov lo, ax
	lprc = MAKELONG(lo, hi);		// para lprc

	_asm mov ax, [bp+18]
	_asm mov hi, ax
	_asm mov ax, [bp+16]
	_asm mov lo, ax
	lpszString = MAKELONG(lo, hi);	// para lpszString


	_asm mov ax, [bp+14]
	_asm mov cbString, ax			// para cdString

	_asm mov ax, [bp+12]
	_asm mov hi, ax
	_asm mov ax, [bp+10]
	_asm mov lo, ax
	lpDx = MAKELONG(lo, hi);		// para lpDx

	_asm mov ax, [bp+8]
	_asm mov [bp+30], ax
	_asm mov ax, [bp+6]
	_asm mov [bp+28], ax			// return address

	Priv_UnhookGDIHook();
	fResult = ExtTextOut(hDC, nXStart, nYStart, fuOptions, (RECT FAR *)lprc,
		 (LPSTR)lpszString, cbString, (int FAR *)lpDx);
	xy = GetDCOrg(hDC);
	x = LOWORD(xy);
	y = HIWORD(xy);

//////////////////

	tmpPt.x = nXStart;
	tmpPt.y = nYStart;		//  now nXStart & nYStart is logical coordinate
	LPtoDP(hDC, (LPPOINT)&tmpPt, 1);
	nXStart = tmpPt.x;
	nYStart = tmpPt.y;		//  now nXStart & nYStart is device coordinate
    
	if ((!fuOptions) && (lprc==NULL)){
		dwTextSize = GetTextExtent(hDC, (LPSTR)lpszString, cbString);
		SetRect(&rc, nXStart, nYStart,
			nXStart + LOWORD(dwTextSize),
			nYStart + HIWORD(dwTextSize));
    }
	else{
		CopyRect(&rc, lprc);
		nXStart = rc.left;
	}

	ptx.x = pt.x - x;
	ptx.y = pt.y - y;		// device coordinate

	if (PtInRect(&rc, ptx)){
			bu.b = GlobalAlloc(GPTR, lstrlen(lpszString)+2);
			if (!bu.b){
				MessageBeep(MB_ICONQUESTION);
				MessageBox(NULL, "Error: no enough memory.",
				"Little Translator",
				MB_OK | MB_ICONEXCLAMATION);
				bu.s = GlobalLock(bu.h);
				bu.s[0] = '\0';
				GlobalUnlock(bu.h);
			}
			else{
	            GlobalFree(bu.h);
				bu.h = bu.b;
				bu.s =GlobalLock(bu.h);
				lstrcpy(bu.s , lpszString);
///////////// add ////////////////////////////////////////
				if (bu.s[0]){
					nL = nT = 0;
					for (nP=0; (bu.s[nL+nP]) && (nL<cbString); nP++){
						if (!IsCharAlpha(bu.s[nL+nP])){
							nL += nP;
							nP = 0;
                            nXStart += nT;
						}
						_fstrncpy(tmp, bu.s+nL, nP+1);
                        tmp[nP+1] = '\0';
						nT = LOWORD(GetTextExtent(hDC, tmp, nP+1));
						if ((nXStart+nT) > ptx.x){
							nXStart += nT;
							break;
                        }
					}
					if (nP && (!IsCharAlpha(bu.s[nL]))) nL++;
					nXStart = nL;
				}

///////////////////////////////////////////////////////////
				GlobalUnlock(bu.h);
            }
			bu.pt.x = ptx.x;
			bu.pt.y = ptx.y;
			bu.nXStart = nXStart;
			bu.cbString = cbString;
	}

	Priv_SetGDIHook();

	_asm mov ax, fResult
	_asm lea sp, [bp-02]
	_asm pop ds
	_asm pop bp
	_asm dec bp
	_asm add sp, 26
	_asm retf

	return fResult;
}


int _export WINAPI WEP(int bSystemExit)
{
	if (HookNow) Priv_UnhookGDIHook();

	FreeSelector(wGDISel);
	FreeProcInstance((FARPROC)lpfn0);
	FreeProcInstance((FARPROC)lpfn1);
	GlobalPageUnlock(wGDICode);
	GlobalFree(bu.h);
	return (1);
}

