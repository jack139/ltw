// exports.h
#ifndef _INC_EXPORTS
#define _INC_EXPORTS

#include <windows.h>
#include "public.h"

//extern BOOL CheckDCWndClassName(HDC hDC);

typedef struct _tagWordPara
{
	HDC  hMemDC;
	int  nBegin;
	int  nEnd;
	int  CharType;
	RECT wordRect;
}WORDPARA, *LPWORDPARA;

DLLEXPORT DWORD WINAPI NHHookWin32Api();
DLLEXPORT DWORD WINAPI NHUnHookWin32Api();
void HookAllTextOut();
void UnHookAllTextOut();

//
BOOL IsParentOrSelf(HWND hParent, HWND hChild);
DLLEXPORT DWORD WINAPI BL_SetGetWordStyle(int nGetWordStyle);
DLLEXPORT DWORD WINAPI BL_GetBuffer16(LPSTR lpszBuffer,
									  short nBufferSize,
									  LPWORDRECT lpWr);
DLLEXPORT DWORD WINAPI BL_GetText32(LPSTR lpszCurWord,
									int nBufferSize,
									LPRECT lpWordRect);
DLLEXPORT DWORD WINAPI BL_SetFlag32(UINT nFlag,
									HWND hNotifyWnd,
									int MouseX,
									int MouseY);

// hook api
DLLEXPORT BOOL WINAPI NHBitBlt(HDC hdcDest,
						       int nXDest,
						       int nYDest,
						       int nWidth,
						       int nHeight,
						       HDC hdcSrc,
						       int nXSrc,
						       int nYSrc,
						       DWORD dwRop);

DLLEXPORT BOOL WINAPI NHTextOutA(HDC hdc,
							     int nXStart,
							     int nYStart,
							     LPCTSTR lpString,
							     int cbString);

DLLEXPORT BOOL WINAPI NHTextOutW(HDC hdc,
							     int nXStart,
							     int nYStart,							     
								 LPCWSTR lpString,
							     int cbString);

DLLEXPORT BOOL WINAPI NHExtTextOutA(HDC hdc,
								    int X,
								    int Y,
								    UINT fuOptions,
								    CONST RECT *lprc,
								    LPCTSTR lpString,
								    UINT cbCount,
								    CONST INT *lpDx);

DLLEXPORT BOOL WINAPI NHExtTextOutW(HDC hdc,
								    int X,
								    int Y,
								    UINT fuOptions,
								    CONST RECT *lprc,
									LPCWSTR lpString,
								    UINT cbCount,
								    CONST INT *lpDx);
#endif // _INC_EXPORTS
