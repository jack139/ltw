// findword.h
#ifndef _INC_FINDWORD
#define _INC_FINDWORD

#include <windows.h>
#include "public.h"

#define HAS_CURMOUSEWORD  1
#define NO_CURMOUSEWORD   0

#define CHAR_TYPE_ASCII 0           // the current character is a...z or A...Z
#define CHAR_TYPE_HZ    1           // the currnet character is chinese.
#define CHAR_TYPE_OTHER 2           // other character.
#define CHAR_LINK       '-'
#define CHAR_WILDCHAR1 '*'
#define CHAR_WILDCHAR2 '?'

#define SEPERATOR       2

#define MAX_CHAR_HEIGHT 40

int  GetCharType(char ch);
int  FindAWord(LPCSTR lpString, int nFromPlace, int nLength);
int  FindDWord(LPCSTR lpString, int nFromPlace, int nLength);
int  FindTWWord(LPCSTR lpString, int nFromPlace, int nLength);
BOOL IsASCIIWord(LPCSTR lpString, int nFromPlace, int nLength, int nCurCharNum);
int  FindHZWord(LPCSTR lpString, int nFromPlace, int nLength);
int  FindNextWordBegin(LPCSTR lpString, int nFromPlace, int nLength);
int  GetCurWordEnd(LPCSTR lpString, int nFromPlace, int nLength, int nCharType);
void CopyWord(LPSTR lpWord, LPCSTR lpString, int nBegin, int nEnd);

void GetStringTopBottom(HDC hDC, int y, RECT* lpStringRect);
void GetStringLeftRight(HDC hDC, LPSTR szBuff, int cbLen, int x, RECT* lpStringRect, CONST INT *lpDx);
void GetStringRect(HDC hDC, LPSTR szBuff, int cbLen, int x, int y, RECT* lpStringRect, CONST INT *lpDx);

DWORD GetCurMousePosWord(HDC   hDC, 
						 LPSTR szBuff, 
						 int   cbLen, 
						 int   x, 
						 int   y, 
						 CONST INT *lpDx);

DWORD CheckMouseInCurWord(HDC   hDC, 
						 LPSTR szBuff, 
						 int   cbLen, 
						 int   x, 
						 int   y, 
						 CONST INT *lpDx,
						 int*  lpLeft,
						 int   nBegin,    // = nPrevWord + 1
						 int   nEnd,
						 int   nCharType);
DWORD CalculateCaretPlace(HDC   hDC, 
						   LPSTR szBuff, 
						   int   cbLen, 
						   int   x, 
						   int   y, 
						   CONST INT *lpDx,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   nCharType);
DWORD GetEngLishCaretPlace(HDC   hDC, 
						   LPSTR szBuff,
							int   x,
							int   y,
							CONST INT *lpDx,
							int   nBegin,    // = nPrevWord + 1
							int   nEnd,
							int   TempPlace,
							int   turnto);
int GetHZBeginPlace(LPSTR lpszHzBuff, int nBegin, int nEnd, LPRECT StringRect);
void AddToTotalWord(LPSTR szBuff, 
					int   cbLen, 
					int   nBegin,
					int   nEnd,
					int   nCharType,
					RECT  StringRect,
					BOOL  bInCurWord);

BOOL CalcCaretInThisPlace(int CaretX, double nPlace);

#define MEMDC_TOTALWORD -1

void AddToTextOutBuffer(HDC hMemDC, LPSTR szBuff, int cbLen, int x, int y, CONST INT *lpDx);
void GetMemWordStringRect(int nWordCode, int nOffset, LPRECT lpStringRect);
void CheckMemDCWordBuffer(HDC hdcDest, HDC hdcSrc);
DWORD CheckMouseInMemDCWord(int nWordCode);
DWORD CalculateCaretPlaceInMemDCWord(int nWordCode);
BOOL CheckDCWndClassName(HDC hDC);

void GetStringRectW(HDC hDC, LPCWSTR lpWideCharStr, UINT cbWideChars, int x, int y, RECT* lpStringRect, CONST INT *lpDx);
void AddToTextOutBufferW(HDC hMemDC, LPCWSTR lpWideCharStr, UINT cbWideChars, int x, int y, CONST INT *lpDx);
DWORD GetCurMousePosWordW(HDC   hDC, 
						 LPCWSTR lpWideCharStr, 
						 UINT cbWideChars, 
						 int   x, 
						 int   y, 
						 CONST INT *lpDx);
DWORD CheckMouseInCurWordW(HDC   hDC, 
						   LPCWSTR lpWideCharStr, 
						   UINT cbWideChars, 
						   int   x, 
						   int   y, 
						   CONST INT *lpDx,
						   int*  lpLeft,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   nCharType);
DWORD CalculateCaretPlaceW(HDC   hDC, 
						   LPCWSTR lpWideCharStr, 
						   UINT cbWideChars, 
						   int   x, 
						   int   y, 
						   CONST INT *lpDx,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   nCharType);
DWORD GetEngLishCaretPlaceW(HDC   hDC, 
							LPCWSTR lpWideCharStr, 
							UINT cbWideChars, 
							int   x,
							int   y,
							CONST INT *lpDx,
							int   nBegin,    // = nPrevWord + 1
							int   nEnd,
							int   TempPlace,
							int   turnto);

#endif // _INC_FINDWORD
