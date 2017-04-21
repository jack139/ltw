/////////////////////////////////////////////////////////////////////////
//
// findword.c
//
// Date   : 04/18/99
//
/////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include "stdio.h"
#include "string.h"
#include "findword.h"
#include "exports.h"
#include "public.h"
#include "dbgprint.h"

extern UINT BL_HASSTRING;

extern char  g_szTotalWord[BUFFERLENGTH];
extern RECT  g_TotalWordRect;
extern int   g_CharType;
extern int   g_nCurCaretPlaceInTotalWord;
extern int   g_bMouseInTotalWord;

extern int g_nWordsInPhrase ;
extern BOOL g_bPhraseNeeded ;
extern int g_nPhraseCharType ;

extern char szMemDCWordBuff[BUFFERLENGTH];
extern int  pnMemDCCharLeft[BUFFERLENGTH];
extern int  pnMemDCCharRight[BUFFERLENGTH];
extern WORDPARA WordBuffer[MEMDC_MAXNUM];
extern int nWordNum;

extern char  g_szCurWord[];
extern RECT  g_CurWordRect;
extern int   g_nCurCaretPlace;

extern BOOL  g_bAllowGetCurWord;
extern POINT g_CurMousePos;
extern HWND  g_hNotifyWnd;

extern UINT         g_nTextAlign;
extern POINT        g_dwDCOrg;
extern int          g_nExtra;
extern POINT        g_CurPos;
extern TEXTMETRIC   g_tm;

extern BOOL bRecAllRect;
extern RECT g_rcTotalRect;
extern RECT g_rcFirstWordRect;

#define RIGHT  1
#define LEFT  -1

#define IE4_CLIENT_CLASSNAME		"Internet Explorer_Server"// 定義取詞中 DC 所屬窗口的類名。
#define OUTLOOK_EDIT_CLASSNAME		"RichEdit20A"
#define MAX_CLASS_NAME 256

int  g_nWorkOnClassNum = 2;
char g_szWorkOnClassName[2][MAX_CLASS_NAME] = { 
									IE4_CLIENT_CLASSNAME,	// IE 4.0
									OUTLOOK_EDIT_CLASSNAME	// OutLook
								};

extern int g_nGetWordStyle;

/**************************************************************************
**************************************************************************/
__inline int  GetCharType(char ch)
{
	BYTE chitem = ch;

	if (ch < 0)
		return CHAR_TYPE_HZ;

	if (((ch >= 'a')&&(ch <= 'z'))||
	    ((ch >= 'A')&&(ch <= 'Z')))
	{
		return CHAR_TYPE_ASCII;
	}
	
	return CHAR_TYPE_OTHER;	
}

__inline int  FindAWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	//Modified by ZHHN on 2000.4
	int nResult = 0;

	switch(g_nGetWordStyle)
	{
	case GETWORD_D_ENABLE:
	case GETWORD_TW_ENABLE:
	case GETPHRASE_ENABLE:
	case GETWORD_D_TYPING_ENABLE:
		nResult = FindTWWord(lpString, nFromPlace, nLength);	//get word with '-'
		break;
	default:
		nResult = FindDWord(lpString, nFromPlace, nLength);		//get word without '-'
		break;
	}

	return nResult;
	//Modified end
}

__inline int  FindDWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_ASCII)
		{
			i++;
		}
		else
		{
			return i-1;
		}
	} 
	
	return nLength - 1;
}

__inline int  FindTWWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (lpString[i] == CHAR_LINK)
		{
			if (IsASCIIWord(lpString, nFromPlace, nLength, i + 1))
			{
				i++;
			}
			else
			{
				return i-1;
			}
		}
		else
		{
			if (IsASCIIWord(lpString, nFromPlace, nLength, i))
			{
				i++;
			}
			else
			{
				return i-1;
			}
		}
	} 
	
	return nLength - 1;
}

__inline BOOL IsASCIIWord(LPCSTR lpString, int nFromPlace, int nLength, int nCurCharNum)
{
	if (GetCharType(lpString[nCurCharNum]) == CHAR_TYPE_ASCII)
	{
		return TRUE;
	}

	return FALSE;
}

__inline int  FindHZWord(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;

	if ((BYTE)(lpString[nFromPlace]) >= 0xa1
		&& (BYTE)(lpString[nFromPlace]) <= 0xa9)
	{
		return nFromPlace + 1 ;
	}

	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_HZ)
		{
			if ((BYTE)(lpString[i]) >= 0xa1
				&& (BYTE)(lpString[i]) <= 0xa9)
			{
				return i - 1;
			}

			i = i + 2;
		}
		else
		{
			return i - 1;
		}
	} 
	
	return nLength - 1;
}

__inline int FindNextWordBegin(LPCSTR lpString, int nFromPlace, int nLength)
{
	int i = nFromPlace;
	while (i < nLength)
	{
		if (GetCharType(lpString[i]) == CHAR_TYPE_OTHER)
		{
			i++;
		}
		else
		{
			return i-1;
		}
	} 
	
	return i - 1;
}

__inline int GetCurWordEnd(LPCSTR lpString, int nFromPlace, int nLength, int nCharType)
{
	switch (nCharType)
	{                       
		case CHAR_TYPE_ASCII:
			 return FindAWord(lpString, nFromPlace, nLength);
			 break;
		case CHAR_TYPE_HZ:
			 return FindHZWord(lpString, nFromPlace, nLength);
			 break;
		case CHAR_TYPE_OTHER:
			 return FindNextWordBegin(lpString, nFromPlace, nLength);
			 break;
	}
	return FindAWord(lpString, nFromPlace, nLength);
}

__inline void CopyWord(LPSTR lpWord, LPCSTR lpString, int nBegin, int nEnd)
{
	int i;
	for ( i = nBegin; i <= nEnd; i++)
	{
		lpWord[i - nBegin] = lpString[i];
	}
	lpWord[nEnd - nBegin + 1] = '\0';
}

void GetStringTopBottom(HDC hDC, int y, RECT* lpStringRect)
{
	POINT  WndPos;

	WndPos.y = g_dwDCOrg.y;

    if (TA_UPDATECP & g_nTextAlign)
    {
    	y = g_CurPos.y;
    }
    
	switch ((TA_TOP | TA_BOTTOM)&g_nTextAlign)
	{
		case TA_BOTTOM:
			 lpStringRect->top    = y - g_tm.tmHeight + g_tm.tmInternalLeading;
			 lpStringRect->bottom = y;
			 break;
		case TA_BASELINE:
			 lpStringRect->top    = y - g_tm.tmAscent;
			 lpStringRect->bottom = y + g_tm.tmDescent;
			 break;
		case TA_TOP:
		default:
			 lpStringRect->top    = y;
			 lpStringRect->bottom = y + g_tm.tmHeight + g_tm.tmInternalLeading;
			 break;
	}
	
	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);

	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
}

void GetStringLeftRight(HDC hDC, LPSTR szBuff, int cbLen, int x, RECT* lpStringRect, CONST INT *lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    int i;

	if (cbLen < 0)
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}
	
	GetTextExtentPoint32(hDC, szBuff, cbLen, &StringSize);	//Modified by ZHHN on 2000.1.14

	WndPos.x = g_dwDCOrg.x;
	
	if (lpDx != NULL)
	{
		StringSize.cx = 0;
		for (i = 0; i < cbLen; i++)
		{
			StringSize.cx += lpDx[i];
		}
	}

    if (TA_UPDATECP & g_nTextAlign)
    {
    	x = g_CurPos.x;
    }
    
	switch ((TA_LEFT | TA_CENTER | TA_RIGHT)&g_nTextAlign)
	{
		case TA_RIGHT:
			 if (!bRecAllRect)
			 {
				 lpStringRect->right = x;
				 lpStringRect->left  = x - StringSize.cx;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_CENTER:
			 if (!bRecAllRect)
			 {
				 lpStringRect->right = x + StringSize.cx / 2;
				 lpStringRect->left  = x - StringSize.cx / 2;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_LEFT:
		default:
			 lpStringRect->left  = x ;
			 lpStringRect->right = x + StringSize.cx;
			 break;
	}
	
	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);

	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

//Created due to fixing 
// Bug4: get big5 chinese word's position error from outlook98 in Windows 2000 
// Bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

void GetStringLeftRightW(HDC hDC, LPCWSTR lpWideCharStr, UINT cbWideChars, int x, RECT* lpStringRect, CONST INT *lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    UINT i;

	if (cbWideChars < 0)
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}
	
	WndPos.x = g_dwDCOrg.x;
	
	GetTextExtentPoint32W(hDC, lpWideCharStr, cbWideChars, &StringSize);

	if (lpDx != NULL)
	{
		StringSize.cx = 0;
		for (i = 0; i < cbWideChars; i++)
		{
			StringSize.cx += lpDx[i];
		}
	}

    if (TA_UPDATECP & g_nTextAlign)
    {
    	x = g_CurPos.x;
    }
    
	switch ((TA_LEFT | TA_CENTER | TA_RIGHT)&g_nTextAlign)
	{
		case TA_RIGHT:
			 if (!bRecAllRect)
			 {
				 lpStringRect->right = x;
				 lpStringRect->left  = x - StringSize.cx;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_CENTER:
			 if (!bRecAllRect)
			 {
				 lpStringRect->right = x + StringSize.cx / 2;
				 lpStringRect->left  = x - StringSize.cx / 2;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_LEFT:
		default:
			 lpStringRect->left  = x ;
			 lpStringRect->right = x + StringSize.cx;
			 break;
	}
	
	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);

	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

void GetStringRect(HDC hDC, LPSTR szBuff, int cbLen, int x, int y, RECT* lpStringRect, CONST INT *lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    int i;

	if (cbLen < 0)
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}

	GetTextExtentPoint32(hDC, szBuff, cbLen, &StringSize);

	WndPos.x = g_dwDCOrg.x;
	WndPos.y = g_dwDCOrg.y;
	
	if (lpDx != NULL)
	{
		StringSize.cx = 0;
		for (i = 0; i < cbLen; i++)
		{
			StringSize.cx += lpDx[i];
		}
	}

    if (TA_UPDATECP & g_nTextAlign)
    {
    	x = g_CurPos.x;
    	y = g_CurPos.y;
    }

	switch ((TA_LEFT | TA_CENTER | TA_RIGHT)&g_nTextAlign)
	{
		case TA_RIGHT:
			 if (bRecAllRect == FALSE)
			 {
				 lpStringRect->right = x;
				 lpStringRect->left  = x - StringSize.cx;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_CENTER:
			 if (bRecAllRect == FALSE)
			 {
				 lpStringRect->right = x + StringSize.cx / 2;
				 lpStringRect->left  = x - StringSize.cx / 2;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_LEFT:
		default:

			 lpStringRect->left  = x ;
			 lpStringRect->right = x + StringSize.cx;
			 break;
	}
	
	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE)&g_nTextAlign)
	{
		case TA_BOTTOM:
			 lpStringRect->top    = y - StringSize.cy;
			 lpStringRect->bottom = y;
			 break;
		case TA_BASELINE:
			 lpStringRect->top    = y - g_tm.tmAscent;
			 lpStringRect->bottom = y + g_tm.tmDescent;
			 break;
		case TA_TOP:
		default:
			 lpStringRect->top    = y;
			 lpStringRect->bottom = y + StringSize.cy;
			 break;
	}

	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);

	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

//Created due to fixing 
// Bug4: get big5 chinese word's position error from outlook98 in Windows 2000 
// Bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

void GetStringRectW(HDC hDC, LPCWSTR lpWideCharStr, UINT cbWideChars, int x, int y, RECT* lpStringRect, CONST INT *lpDx)
{
	SIZE   StringSize;
	POINT  WndPos;
    UINT i;

	if (cbWideChars < 0)
	{
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		return;
	}

	WndPos.x = g_dwDCOrg.x;
	WndPos.y = g_dwDCOrg.y;

	GetTextExtentPoint32W(hDC, lpWideCharStr, cbWideChars, &StringSize);
	
	if (lpDx != NULL)
	{
		StringSize.cx = 0;
		for (i = 0; i < cbWideChars; i++)
		{
			StringSize.cx += lpDx[i];
		}
	}

    if (TA_UPDATECP & g_nTextAlign)
    {
    	x = g_CurPos.x;
    	y = g_CurPos.y;
    }

	switch ((TA_LEFT | TA_CENTER | TA_RIGHT)&g_nTextAlign)
	{
		case TA_RIGHT:
			 if (bRecAllRect == FALSE)
			 {
				 lpStringRect->right = x;
				 lpStringRect->left  = x - StringSize.cx;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_CENTER:
			 if (bRecAllRect == FALSE)
			 {
				 lpStringRect->right = x + StringSize.cx / 2;
				 lpStringRect->left  = x - StringSize.cx / 2;
			 }
			 else
			 {
			 	lpStringRect->left = g_rcTotalRect.left;
			 	lpStringRect->right= g_rcTotalRect.left + StringSize.cx;
			 }
			 break;
		case TA_LEFT:
		default:

			 lpStringRect->left  = x ;
			 lpStringRect->right = x + StringSize.cx;
			 break;
	}
	
	switch ((TA_TOP | TA_BOTTOM | TA_BASELINE)&g_nTextAlign)
	{
		case TA_BOTTOM:
			 lpStringRect->top    = y - StringSize.cy;
			 lpStringRect->bottom = y;
			 break;
		case TA_BASELINE:
			 lpStringRect->top    = y - g_tm.tmAscent;
			 lpStringRect->bottom = y + g_tm.tmDescent;
			 break;
		case TA_TOP:
		default:
			 lpStringRect->top    = y;
			 lpStringRect->bottom = y + StringSize.cy;
			 break;
	}

	LPtoDP(hDC, (LPPOINT)lpStringRect, 2);

	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

DWORD GetCurMousePosWord(HDC   hDC, 
						 LPSTR szBuff, 
						 int   cbLen, 
						 int   x, 
						 int   y, 
						 CONST INT *lpDx)
{
	int   nCurrentWord, nPrevWord;
	RECT  StringRect;
	int   CharType;
	int   nLeft;
	DWORD dwReturn;

	DWORD dwResult = NO_CURMOUSEWORD;	//Added by ZHHN on 2000.4
/*
	if (cbLen != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "1........... GetCurMousePosWord : (%s) (%d) %d %d\n", szBuff, cbLen, x, y);
		OutputDebugString(cBuffer);
	}
*/
	GetStringTopBottom(hDC, y, &StringRect);

	if ((StringRect.top > g_CurMousePos.y) || (StringRect.bottom < g_CurMousePos.y))
	{
		return NO_CURMOUSEWORD;
	}

	GetStringRect(hDC, szBuff, cbLen, x, y, &StringRect, lpDx);
	nLeft = StringRect.left;

	nPrevWord = nCurrentWord = -1;
	while (nCurrentWord < cbLen)
	{
		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);
		dwReturn     = CheckMouseInCurWord(hDC, szBuff, cbLen, x, y, lpDx, &nLeft, nPrevWord + 1, nCurrentWord, CharType);
/*
		if (cbLen != 0)
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "2........... GetCurMousePosWord : %s %d %s %08x\n", 
				szBuff, cbLen, "HAS_CURMOUSEWORD", g_hNotifyWnd);
			OutputDebugString(cBuffer);
		}
*/
		if (dwReturn == HAS_CURMOUSEWORD)
		{
			if (CharType == CHAR_TYPE_OTHER)
			{
				PostMessage(g_hNotifyWnd, BL_HASSTRING, 0, 0l);				
			}
		}
		else
		{
			if (g_bMouseInTotalWord)
			{
				PostMessage(g_hNotifyWnd, BL_HASSTRING, 0, 0l);
			}
		}

		if (dwReturn == HAS_CURMOUSEWORD)
		{
			//return HAS_CURMOUSEWORD;
			dwResult = HAS_CURMOUSEWORD;//Modified by ZHHN on 2000.4 in order to get phrase
		}

		if (nCurrentWord >= cbLen - 1)
		{
			//return NO_CURMOUSEWORD;
			break;	//Modified by ZHHN on 2000.4
		}
	}
		
	//return NO_CURMOUSEWORD;
	return dwResult;	//Modified by ZHHN on 2000.4
}

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

DWORD GetCurMousePosWordW(HDC   hDC, 
						  LPCWSTR lpWideCharStr, 
						  UINT cbWideChars, 
						  int   x, 
						  int   y, 
						  CONST INT *lpDx)
{
	int   nCurrentWord, nPrevWord;
	RECT  StringRect;
	int   CharType;
	int   nLeft;
	DWORD dwReturn;

	DWORD dwResult = NO_CURMOUSEWORD;	//Added by ZHHN on 2000.4 in order to get phrase

	int cbLen;
	char szBuff[256];

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;

/*
	if (cbLen != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "1........... GetCurMousePosWord : (%s) (%d) %d %d\n", szBuff, cbLen, x, y);
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("GetCurMousePosWordW : szBuff(%s), cbLen(%d)\n", szBuff, cbLen);

	GetStringTopBottom(hDC, y, &StringRect);

	if ((StringRect.top > g_CurMousePos.y) || (StringRect.bottom < g_CurMousePos.y))
	{
		return NO_CURMOUSEWORD;
	}

	GetStringRectW(hDC, lpWideCharStr, cbWideChars, x, y, &StringRect, lpDx);
	nLeft = StringRect.left;

	nPrevWord = nCurrentWord = -1;
	while (nCurrentWord < cbLen)
	{
		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);
		dwReturn     = CheckMouseInCurWordW(hDC, lpWideCharStr, cbWideChars, x, y, lpDx, &nLeft, nPrevWord + 1, nCurrentWord, CharType);
/*
		if (cbLen != 0)
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "2........... GetCurMousePosWord : %s %d %s %08x\n", 
				szBuff, cbLen, "HAS_CURMOUSEWORD", g_hNotifyWnd);
			OutputDebugString(cBuffer);
		}
*/
		if (dwReturn == HAS_CURMOUSEWORD)
		{
			if (CharType == CHAR_TYPE_OTHER)
			{
				PostMessageW(g_hNotifyWnd, BL_HASSTRING, 0, 0l);				
			}
		}
		else
		{
			if (g_bMouseInTotalWord)
			{
				PostMessageW(g_hNotifyWnd, BL_HASSTRING, 0, 0l);				
			}
		}

		if (dwReturn == HAS_CURMOUSEWORD)
		{
			//return HAS_CURMOUSEWORD;
			dwResult = HAS_CURMOUSEWORD;	//Modified by ZHHN on 2000.4 in order to get phrase
		}

		if (nCurrentWord >= cbLen - 1)
		{
			//return NO_CURMOUSEWORD;
			break;	//Modified by ZHHN on 2000.4
		}
	}
		
	//return NO_CURMOUSEWORD;
	return dwResult;	//Modified by ZHHN on 2000.4
}

DWORD CheckMouseInCurWord(HDC   hDC, 
						  LPSTR szBuff, 
						  int   cbLen, 
						  int   x, 
						  int   y, 
						  CONST INT *lpDx,
						  int*  lpLeft,
						  int   nBegin,    // = nPrevWord + 1
						  int   nEnd,
						  int   nCharType)
{
	RECT  StringRect;

	GetStringRect(hDC, szBuff, nEnd + 1, x, y, &StringRect, lpDx);
	StringRect.left = *lpLeft;
	*lpLeft = StringRect.right;
/*
	if (cbLen != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "........... CheckMouseInCurWord : %s %d (%d,%d) (%d,%d,%d,%d)\n", 
			"start", nCharType, g_CurMousePos.x, g_CurMousePos.y, 
			StringRect.left, StringRect.top, StringRect.right, StringRect.bottom);
		OutputDebugString(cBuffer);
	}
*/
	if (  ((g_CurMousePos.x >= StringRect.left    ) && (g_CurMousePos.x <= StringRect.right))
	    || (g_CurMousePos.x == StringRect.left - 1)
	    || (g_CurMousePos.x == StringRect.right + 1))
	{
/*
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "........... CheckMouseInCurWord : %s %d\n", 
				"start", nCharType);
			OutputDebugString(cBuffer);
		}
*/
		switch (nCharType)
		{
			case CHAR_TYPE_HZ:
			case CHAR_TYPE_ASCII:
				 CopyWord(g_szCurWord, szBuff, nBegin, nEnd);
				 g_CurWordRect.left   = StringRect.left;
				 g_CurWordRect.right  = StringRect.right;
				 g_CurWordRect.top    = StringRect.top;
				 g_CurWordRect.bottom = StringRect.bottom;
/*				 
				{
					char cBuffer[0x100];
					wsprintf(cBuffer, "!!!........... CheckMouseInCurWord : %d %d %d %d\n", 
						g_CurWordRect.left, g_CurWordRect.top, g_CurWordRect.right, g_CurWordRect.bottom);
					OutputDebugString(cBuffer);
				}
*/
				 g_nCurCaretPlace = -1;
				 CalculateCaretPlace(hDC, 
									 szBuff, 
									 cbLen,
									 x,
									 y,
									 lpDx,
									 nBegin,
									 nEnd,
									 nCharType);
//				 g_bMouseInTotalWord = TRUE;
				 break;

			case CHAR_TYPE_OTHER:
				 break;
		}

		AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, TRUE);

		if (  (nCharType == CHAR_TYPE_OTHER)
		    &&(CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right)))
		{
			return HAS_CURMOUSEWORD;
		}

		return HAS_CURMOUSEWORD;
	}
	else
	{
//		g_bMouseInTotalWord = FALSE;
	}					

	AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, FALSE);

	return NO_CURMOUSEWORD;   
}

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

DWORD CheckMouseInCurWordW(HDC   hDC, 
						   LPCWSTR lpWideCharStr, 
						   UINT cbWideChars, 
						   int   x, 
						   int   y, 
						   CONST INT *lpDx,
						   int*  lpLeft,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   nCharType)
{
	RECT  StringRect;

	int cbLen;
	char szBuff[256];

	wchar_t lpTemp[256];
	int nTempLen;

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;

	nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, nEnd + 1, lpTemp, 256);
	GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &StringRect, lpDx);

	StringRect.left = *lpLeft;
	*lpLeft = StringRect.right;
/*
	if (cbLen != 0)
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "........... CheckMouseInCurWord : %s %d (%d,%d) (%d,%d,%d,%d)\n", 
			"start", nCharType, g_CurMousePos.x, g_CurMousePos.y, 
			StringRect.left, StringRect.top, StringRect.right, StringRect.bottom);
		OutputDebugString(cBuffer);
	}
*/
	if (  ((g_CurMousePos.x >= StringRect.left    ) && (g_CurMousePos.x <= StringRect.right))
	    || (g_CurMousePos.x == StringRect.left - 1)
	    || (g_CurMousePos.x == StringRect.right + 1))
	{
/*
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "........... CheckMouseInCurWord : %s %d\n", 
				"start", nCharType);
			OutputDebugString(cBuffer);
		}
*/
		switch (nCharType)
		{
			case CHAR_TYPE_HZ:
			case CHAR_TYPE_ASCII:
				 CopyWord(g_szCurWord, szBuff, nBegin, nEnd);
				 g_CurWordRect.left   = StringRect.left;
				 g_CurWordRect.right  = StringRect.right;
				 g_CurWordRect.top    = StringRect.top;
				 g_CurWordRect.bottom = StringRect.bottom;
/*				 
				{
					char cBuffer[0x100];
					wsprintf(cBuffer, "!!!........... CheckMouseInCurWord : %d %d %d %d\n", 
						g_CurWordRect.left, g_CurWordRect.top, g_CurWordRect.right, g_CurWordRect.bottom);
					OutputDebugString(cBuffer);
				}
*/
				 g_nCurCaretPlace = -1;
				 CalculateCaretPlaceW(hDC, 
									 lpWideCharStr, 
									 cbWideChars,
									 x,
									 y,
									 lpDx,
									 nBegin,
									 nEnd,
									 nCharType);
//				 g_bMouseInTotalWord = TRUE;
				 break;

			case CHAR_TYPE_OTHER:
				 break;
		}

		AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, TRUE);

		if (  (nCharType == CHAR_TYPE_OTHER)
		    &&(CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right)))
		{
			return HAS_CURMOUSEWORD;
		}

		return HAS_CURMOUSEWORD;
	}
	else
	{
//		g_bMouseInTotalWord = FALSE;
	}					

	AddToTotalWord(szBuff, cbLen, nBegin, nEnd, nCharType, StringRect, FALSE);

	return NO_CURMOUSEWORD;   
}

DWORD CalculateCaretPlace(HDC   hDC, 
						  LPSTR szBuff, 
						  int   cbLen, 
						  int   x, 
						  int   y, 
						  CONST INT *lpDx,
						  int   nBegin,    // = nPrevWord + 1
						  int   nEnd,
						  int   nCharType)
{
	RECT  StringRect, BeginRect;
	RECT  CaretPrevRect, CaretNextRect;
	double itemWidth; 
	int   TempPlace;
	int   i;

	if ((nCharType == CHAR_TYPE_HZ) && (nBegin == nEnd))
	{
		g_nCurCaretPlace = -1;
		return 0L;
	}

	GetStringRect(hDC, szBuff, nBegin, x, y, &BeginRect, lpDx);
	
	GetStringRect(hDC, szBuff, nEnd + 1,   x, y, &StringRect, lpDx);
	StringRect.left = BeginRect.right;
    if (StringRect.left == StringRect.right)
    {
		g_nCurCaretPlace = -1;
		return 0L;
    }
	
	switch (nCharType)
	{
		case CHAR_TYPE_HZ:
			 itemWidth = ((double)StringRect.right - (double)StringRect.left + 1) / ((double)nEnd - (double)nBegin + 1);
			 for (i = 0; i <= (nEnd - nBegin + 1); i++)
			 {
			 	if (CalcCaretInThisPlace(g_CurMousePos.x, (double)((double)StringRect.left + (double)(itemWidth * i))))
			 	{
				 	g_nCurCaretPlace = i;
				 	i = nEnd - nBegin + 2;
			 	}
			 }
             break;
		case CHAR_TYPE_ASCII:
			 itemWidth = (StringRect.right - StringRect.left + 1) / (nEnd - nBegin + 1);
			 TempPlace = (g_CurMousePos.x - StringRect.left) * (nEnd - nBegin + 1) / (StringRect.right - StringRect.left);
			 GetStringRect(hDC, szBuff, TempPlace,     x, y, &CaretPrevRect, lpDx);
			 GetStringRect(hDC, szBuff, TempPlace + 1, x, y, &CaretNextRect, lpDx);
			 
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = TempPlace - nBegin;
			 	break;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = TempPlace - nBegin + 1;
			 	break;
			 }
			 if (g_CurMousePos.x > CaretNextRect.right)
			 {
				 GetEngLishCaretPlace(hDC, 
									  szBuff,
									  x,
									  y,
									  lpDx,
									  nBegin,    // = nPrevWord + 1
									  nEnd,
									  TempPlace,
									  RIGHT);
			 }
			 else
			 {
				 GetEngLishCaretPlace(hDC, 
									  szBuff,
									  x,
									  y,
									  lpDx,
									  nBegin,    // = nPrevWord + 1
									  nEnd,
									  TempPlace,
									  LEFT);
			 }
			 
			 break;
	}
	return 0L;
}

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

DWORD CalculateCaretPlaceW(HDC   hDC, 
						   LPCWSTR lpWideCharStr, 
						   UINT cbWideChars, 
						   int   x, 
						   int   y, 
						   CONST INT *lpDx,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   nCharType)
{
	RECT  StringRect, BeginRect;
	RECT  CaretPrevRect, CaretNextRect;
	double itemWidth; 
	int   TempPlace;
	int   i;

	int cbLen;
	char szBuff[256];

	wchar_t lpTemp[256];
	int nTempLen;

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;

	if ((nCharType == CHAR_TYPE_HZ) && (nBegin == nEnd))
	{
		g_nCurCaretPlace = -1;
		return 0L;
	}

	nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, nBegin, lpTemp, 256);
	GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &BeginRect, lpDx);

	nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, nEnd + 1, lpTemp, 256);
	GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &StringRect, lpDx);

	StringRect.left = BeginRect.right;
    if (StringRect.left == StringRect.right)
    {
		g_nCurCaretPlace = -1;
		return 0L;
    }
	
	switch (nCharType)
	{
		case CHAR_TYPE_HZ:
			 itemWidth = ((double)StringRect.right - (double)StringRect.left + 1) / ((double)nEnd - (double)nBegin + 1);
			 for (i = 0; i <= (nEnd - nBegin + 1); i++)
			 {
			 	if (CalcCaretInThisPlace(g_CurMousePos.x, (double)((double)StringRect.left + (double)(itemWidth * i))))
			 	{
				 	g_nCurCaretPlace = i;
				 	i = nEnd - nBegin + 2;
			 	}
			 }
             break;
		case CHAR_TYPE_ASCII:
			 itemWidth = (StringRect.right - StringRect.left + 1) / (nEnd - nBegin + 1);
			 TempPlace = (g_CurMousePos.x - StringRect.left) * (nEnd - nBegin + 1) / (StringRect.right - StringRect.left);

			 nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, TempPlace, lpTemp, 256);
			 GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretPrevRect, lpDx);
			 nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, TempPlace + 1, lpTemp, 256);
			 GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretNextRect, lpDx);
			 
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = TempPlace - nBegin;
			 	break;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = TempPlace - nBegin + 1;
			 	break;
			 }
			 if (g_CurMousePos.x > CaretNextRect.right)
			 {
				 GetEngLishCaretPlaceW(hDC, 
									  lpWideCharStr,
									  cbWideChars,
									  x,
									  y,
									  lpDx,
									  nBegin,    // = nPrevWord + 1
									  nEnd,
									  TempPlace,
									  RIGHT);
			 }
			 else
			 {
				 GetEngLishCaretPlaceW(hDC, 
									  lpWideCharStr,
									  cbWideChars,
									  x,
									  y,
									  lpDx,
									  nBegin,    // = nPrevWord + 1
									  nEnd,
									  TempPlace,
									  LEFT);
			 }
			 
			 break;
	}
	return 0L;
}

DWORD GetEngLishCaretPlace(HDC   hDC, 
						   LPSTR szBuff,
						   int   x,
						   int   y,
						   CONST INT *lpDx,
						   int   nBegin,    // = nPrevWord + 1
						   int   nEnd,
						   int   TempPlace,
						   int   turnto)
{
	int i;
	RECT  CaretPrevRect, CaretNextRect;
	
	if (turnto == RIGHT)
	{
		i = TempPlace;
		GetStringRect(hDC, szBuff, i, x, y, &CaretPrevRect, lpDx);
		
		for (i = TempPlace; i <= nEnd; i++)
		{
			 GetStringRect(hDC, szBuff, i + 1, x, y, &CaretNextRect, lpDx);
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = i - nBegin;
			 	return 0;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = i - nBegin + 1;
			 	return 0;
			 }

			 CopyRect(&CaretPrevRect, &CaretNextRect);
		}
		g_nCurCaretPlace = nEnd - nBegin + 1;
	}
	else
	{
		i = TempPlace;
		GetStringRect(hDC, szBuff, i + 1, x, y, &CaretNextRect, lpDx);

		for (i = TempPlace; i >= nBegin; i--)
		{
			 GetStringRect(hDC, szBuff, i, x, y, &CaretPrevRect, lpDx);
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = i - nBegin;
			 	return 0;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = i - nBegin + 1;
			 	return 0;
			 }

			 CopyRect(&CaretNextRect, &CaretPrevRect);
		}
		g_nCurCaretPlace = nBegin - nBegin;
	}
	
	return 0;
}

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

DWORD GetEngLishCaretPlaceW(HDC   hDC, 
							LPCWSTR lpWideCharStr, 
							UINT cbWideChars, 
							int   x,
							int   y,
							CONST INT *lpDx,
							int   nBegin,    // = nPrevWord + 1
							int   nEnd,
							int   TempPlace,
							int   turnto)
{
	int i;
	RECT  CaretPrevRect, CaretNextRect;

	int cbLen;
	char szBuff[256];

	wchar_t lpTemp[256];
	int nTempLen;

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;
	
	if (turnto == RIGHT)
	{
		i = TempPlace;

		nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i, lpTemp, 256);
		GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretPrevRect, lpDx);
		
		for (i = TempPlace; i <= nEnd; i++)
		{
			 nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i + 1, lpTemp, 256);
			 GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretNextRect, lpDx);
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = i - nBegin;
			 	return 0;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = i - nBegin + 1;
			 	return 0;
			 }

			 CopyRect(&CaretPrevRect, &CaretNextRect);
		}
		g_nCurCaretPlace = nEnd - nBegin + 1;
	}
	else
	{
		i = TempPlace;

		nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i + 1, lpTemp, 256);
		GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretNextRect, lpDx);

		for (i = TempPlace; i >= nBegin; i--)
		{
 			 nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i, lpTemp, 256);
			 GetStringRectW(hDC, lpWideCharStr, nTempLen, x, y, &CaretPrevRect, lpDx);
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretPrevRect.right)) 
			 {
			 	g_nCurCaretPlace = i - nBegin;
			 	return 0;
			 }
			 if (CalcCaretInThisPlace(g_CurMousePos.x, CaretNextRect.right))
			 {
			 	g_nCurCaretPlace = i - nBegin + 1;
			 	return 0;
			 }

			 CopyRect(&CaretNextRect, &CaretPrevRect);
		}
		g_nCurCaretPlace = nBegin - nBegin;
	}
	
	return 0;
}

BOOL CalcCaretInThisPlace(int CaretX, double nPlace)
{
/*	if ((double)CaretX == nPlace)
	{
		return TRUE;
	}
*/	
	if (((double)CaretX >= nPlace - 3)&&((double)CaretX <= nPlace + 1))
	{
		return TRUE;
	}
	
	return FALSE;
}

int GetHZBeginPlace(LPSTR lpszHzBuff, int nBegin, int nEnd, LPRECT lpStringRect)
{
	int itemWidth; 
	int nReturn;

	itemWidth = (lpStringRect->right - lpStringRect->left + 1) / (nEnd - nBegin + 1);
	nReturn = (g_CurMousePos.x - lpStringRect->left) * (nEnd - nBegin + 1) / (lpStringRect->right - lpStringRect->left);

	if (nReturn == 0)
		return nBegin;		
    
	if (nReturn % 2 == 1)
	{
    	lpStringRect->left = lpStringRect->left + (nReturn - 1) * itemWidth;
    	return nBegin + nReturn - 1;
    }
    
   	lpStringRect->left = lpStringRect->left + nReturn * itemWidth;
	return (nBegin + nReturn);
}

void AddToTotalWord(LPSTR szBuff, 
					int   cbLen, 
					int   nBegin,
					int   nEnd,
					int   nCharType,
					RECT  StringRect,
					BOOL  bInCurWord)
{
	int nPos = 0;
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "........... AddToTotalWord : %s\n", "start");
		OutputDebugString(cBuffer);
	}
*/
	if ((nCharType == CHAR_TYPE_OTHER) && (!g_bMouseInTotalWord))
	{
		g_szTotalWord[0] = 0x00;
		g_CharType = nCharType;
		g_bMouseInTotalWord = FALSE;
		return;
	}

	if (((BYTE)(szBuff[nBegin]) >= 0xa1 && (BYTE)(szBuff[nBegin]) <= 0xa9)
		&& (!g_bMouseInTotalWord))
	{
		g_szTotalWord[0] = 0x00;
		g_CharType = CHAR_TYPE_HZ;
		g_bMouseInTotalWord = FALSE;
		return;
	}
	
	if ((g_szTotalWord[0] == 0x00)&&(nCharType != CHAR_TYPE_OTHER))
	{
		CopyWord(g_szTotalWord, szBuff, nBegin, nEnd);
		g_TotalWordRect.left   = StringRect.left;
		g_TotalWordRect.right  = StringRect.right;
		g_TotalWordRect.top    = StringRect.top;
		g_TotalWordRect.bottom = StringRect.bottom;
		g_CharType = nCharType;
		if (bInCurWord)
		{
			g_bMouseInTotalWord = TRUE;
			g_nPhraseCharType = nCharType ;
			g_nWordsInPhrase++;

			g_rcFirstWordRect.left   = StringRect.left;
			g_rcFirstWordRect.right  = StringRect.right;
			g_rcFirstWordRect.top    = StringRect.top;
			g_rcFirstWordRect.bottom = StringRect.bottom;
		}
		else
		{
			g_bMouseInTotalWord = FALSE;
		}
		g_nCurCaretPlaceInTotalWord = -1;
		if (g_nCurCaretPlace != -1)
		{
			g_nCurCaretPlaceInTotalWord = g_nCurCaretPlace;
		}
		return;
	}

	if (!g_bMouseInTotalWord)
	{
		if (g_CharType != nCharType)
		{
			CopyWord(g_szTotalWord, szBuff, nBegin, nEnd);
			g_TotalWordRect.left   = StringRect.left;
			g_TotalWordRect.right  = StringRect.right;
			g_TotalWordRect.top    = StringRect.top;
			g_TotalWordRect.bottom = StringRect.bottom;
			g_CharType = nCharType;
			if (bInCurWord)
			{
				g_bMouseInTotalWord = TRUE;
				g_nPhraseCharType = nCharType ;
				g_nWordsInPhrase++;

				g_rcFirstWordRect.left   = StringRect.left;
				g_rcFirstWordRect.right  = StringRect.right;
				g_rcFirstWordRect.top    = StringRect.top;
				g_rcFirstWordRect.bottom = StringRect.bottom;
			}
			else
			{
				g_bMouseInTotalWord = FALSE;
			}
			g_nCurCaretPlaceInTotalWord = -1;
			if (g_nCurCaretPlace != -1)
			{
				g_nCurCaretPlaceInTotalWord = g_nCurCaretPlace;
			}
			return;
		}
	}

	if ((g_CharType != nCharType))
	{
		if ( ((nCharType == CHAR_TYPE_OTHER) && (szBuff[nBegin] == ' '))
			 || ((nCharType != CHAR_TYPE_OTHER)
				 && (g_nPhraseCharType == nCharType)) )
		{
			if (   g_bPhraseNeeded 
				   &&(g_nWordsInPhrase > MIN_WORDS_IN_PHRASE)
				   &&(g_nWordsInPhrase < MAX_WORDS_IN_PHRASE)
				   &&(!(g_szTotalWord[0] < 0 && szBuff[nBegin] == ' ')) )
			{
			}
			else
			{
				g_nWordsInPhrase = MAX_WORDS_IN_PHRASE + 1 ;
				return;
			}
		}
		else
		{
			g_nWordsInPhrase = MAX_WORDS_IN_PHRASE + 1 ;
			return;
		}
	}
	
	if (  ((UINT)(StringRect.left - g_TotalWordRect.right) <= (UINT)SEPERATOR)
		&&(abs((int)(StringRect.bottom - g_TotalWordRect.bottom)) <= SEPERATOR))
	{
		if ((BYTE)(szBuff[nBegin]) >= 0xa1 
			&& (BYTE)(szBuff[nBegin]) <= 0xa9)
			
		{
			return;
		}

		if (g_bMouseInTotalWord && g_bPhraseNeeded
			&& (g_nWordsInPhrase > MIN_WORDS_IN_PHRASE)
			&& (g_nWordsInPhrase < MAX_WORDS_IN_PHRASE)
			&& (szBuff[nBegin] == ' '))
		{
			nPos = nBegin ;
			while (szBuff[nPos] == ' ' && nPos <= nEnd)
			{
				nPos++;
			}      
			if (nPos <= nEnd || nPos - nBegin > 1)			
			{
				g_nWordsInPhrase = MAX_WORDS_IN_PHRASE + 1 ;
			}
			else
			{
				g_nWordsInPhrase++;
			}
			
		}

		if (g_nWordsInPhrase >= MAX_WORDS_IN_PHRASE)
		{
			return ;
		}

		if ((g_nCurCaretPlace != -1)&&(g_nCurCaretPlaceInTotalWord == -1))
		{
			g_nCurCaretPlaceInTotalWord = strlen(g_szTotalWord) + g_nCurCaretPlace;
		}
		CopyWord(g_szTotalWord + strlen(g_szTotalWord), szBuff, nBegin, nEnd);
		g_TotalWordRect.right  = StringRect.right;

		if (!strchr(g_szTotalWord, ' ') && (*(szBuff + nBegin) != ' '))
		{
			g_rcFirstWordRect.right  = StringRect.right;
		}

		if (bInCurWord)
		{
			g_bMouseInTotalWord = TRUE;			
			g_nPhraseCharType = nCharType ;
			g_nWordsInPhrase++;
			
			g_rcFirstWordRect.left   = StringRect.left;
			g_rcFirstWordRect.right  = StringRect.right;
			g_rcFirstWordRect.top    = StringRect.top;
			g_rcFirstWordRect.bottom = StringRect.bottom;
		}
	}
	else
	{
		if (!g_bMouseInTotalWord)
		{
			CopyWord(g_szTotalWord, szBuff, nBegin, nEnd);
			g_TotalWordRect.left   = StringRect.left;
			g_TotalWordRect.right  = StringRect.right;
			g_TotalWordRect.top    = StringRect.top;
			g_TotalWordRect.bottom = StringRect.bottom;
			g_CharType = nCharType;
			g_bMouseInTotalWord = FALSE;
			g_nCurCaretPlaceInTotalWord = -1;
			if (g_nCurCaretPlace != -1)
			{
				g_nCurCaretPlaceInTotalWord = g_nCurCaretPlace;
			}
			if (bInCurWord)
				g_bMouseInTotalWord = TRUE;
				g_nPhraseCharType = nCharType;
				g_nWordsInPhrase++;

				g_rcFirstWordRect.left   = StringRect.left;
				g_rcFirstWordRect.right  = StringRect.right;
				g_rcFirstWordRect.top    = StringRect.top;
				g_rcFirstWordRect.bottom = StringRect.bottom;
		}
	}
/*
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "........... AddToTotalWord : %s %d %d %d %d\n", 
			g_szTotalWord, g_CurWordRect.left, g_CurWordRect.top, g_CurWordRect.right, g_CurWordRect.bottom);
		OutputDebugString(cBuffer);
	}
*/
}

void AddToTextOutBuffer(HDC hMemDC, LPSTR szBuff, int cbLen, int x, int y, CONST INT *lpDx)
{
	int  nPrevWord, nCurrentWord, CharType;
	RECT PrevWordRect, NextWordRect;
	int  nLen, i;
/*	
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "4........... AddToTextOutBuffer : (%s) %d\n", szBuff, cbLen);
		OutputDebugString(cBuffer);
	}
*/
	if (cbLen >= (int)(BUFFERLENGTH - strlen(szMemDCWordBuff) - 2))
	{
		return;
	}

	nLen = strlen(szMemDCWordBuff);
	strncpy(szMemDCWordBuff + nLen, szBuff, cbLen);
	szMemDCWordBuff[nLen + cbLen    ] = ' ';
	szMemDCWordBuff[nLen + cbLen + 1] = 0x00;

	nPrevWord = nCurrentWord = -1;

	GetStringRect(hMemDC, szBuff, nPrevWord + 1, x, y, &PrevWordRect, lpDx);//zhhn

	while (nCurrentWord < cbLen)
	{
		if (nWordNum >= MEMDC_MAXNUM)
			break;

		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);

		//GetStringRect(hMemDC, szBuff, nPrevWord + 1, x, y, &PrevWordRect, lpDx);//modified by zhhn
		GetStringRect(hMemDC, szBuff, nCurrentWord + 1 , x, y, &NextWordRect, lpDx);
		
		WordBuffer[nWordNum].nBegin = nLen + nPrevWord + 1;
		WordBuffer[nWordNum].nEnd   = nLen + nCurrentWord;
		WordBuffer[nWordNum].hMemDC = hMemDC;
		WordBuffer[nWordNum].CharType = CharType;
		WordBuffer[nWordNum].wordRect.left   = PrevWordRect.right;
		WordBuffer[nWordNum].wordRect.right  = NextWordRect.right;
		WordBuffer[nWordNum].wordRect.top    = NextWordRect.top;
		WordBuffer[nWordNum].wordRect.bottom = NextWordRect.bottom;

		CopyRect(&PrevWordRect, &NextWordRect);//zhhn

		nWordNum++;

		if (nCurrentWord >= cbLen - 1)
			break;
	}
	
	GetStringLeftRight(hMemDC, szBuff, 0, x, &PrevWordRect, lpDx);
	for (i = 0; i < cbLen; i++)
	{
		GetStringLeftRight(hMemDC, szBuff, i+1, x, &NextWordRect, lpDx);
		pnMemDCCharLeft[nLen + i]  = PrevWordRect.right;
		pnMemDCCharRight[nLen + i] = NextWordRect.right;
	
		CopyRect(&PrevWordRect, &NextWordRect);
	}
}

//Created due to fixing bug5: get word position error sometimes
//Author: Zhang Haining
//Date: 01/19/2000

void AddToTextOutBufferW(HDC hMemDC, LPCWSTR lpWideCharStr, UINT cbWideChars, int x, int y, CONST INT *lpDx)
{
	int  nPrevWord, nCurrentWord, CharType;
	RECT PrevWordRect, NextWordRect;
	int  nLen, i;
	int cbLen;
	char szBuff[256];

	wchar_t lpTemp[256];
	int nTempLen;

	cbLen = WideCharToMultiByte(CP_ACP, 0, 
		lpWideCharStr, cbWideChars, 
		szBuff, 256, NULL, NULL);
	szBuff[cbLen] = 0x00;

/*	
	{
		char cBuffer[0x100];
		wsprintf(cBuffer, "4........... AddToTextOutBufferW : (%s) %d\n", szBuff, cbLen);
		OutputDebugString(cBuffer);
	}
*/
	//DbgFilePrintf("AddToTextOutBufferW : szBuff(%s), cbLen(%d)\n", szBuff, cbLen);

	if (cbLen >= (int)(BUFFERLENGTH - strlen(szMemDCWordBuff) - 2))
	{
		return;
	}

	nLen = strlen(szMemDCWordBuff);
	strncpy(szMemDCWordBuff + nLen, szBuff, cbLen);
	szMemDCWordBuff[nLen + cbLen    ] = ' ';
	szMemDCWordBuff[nLen + cbLen + 1] = 0x00;

	nPrevWord = nCurrentWord = -1;

	GetStringRectW(hMemDC, lpWideCharStr, 0, x, y, &PrevWordRect, lpDx);//zhhn

	while (nCurrentWord < cbLen)
	{
		if (nWordNum >= MEMDC_MAXNUM)
			break;

		CharType     = GetCharType(szBuff[nCurrentWord + 1]);
		nPrevWord    = nCurrentWord;
		nCurrentWord = GetCurWordEnd(szBuff, nPrevWord + 1, cbLen, CharType);

		nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, nCurrentWord + 1, lpTemp, 256);
		GetStringRectW(hMemDC, lpWideCharStr, nTempLen , x, y, &NextWordRect, lpDx);
		
		WordBuffer[nWordNum].nBegin = nLen + nPrevWord + 1;
		WordBuffer[nWordNum].nEnd   = nLen + nCurrentWord;
		WordBuffer[nWordNum].hMemDC = hMemDC;
		WordBuffer[nWordNum].CharType = CharType;
		WordBuffer[nWordNum].wordRect.left   = PrevWordRect.right;
		WordBuffer[nWordNum].wordRect.right  = NextWordRect.right;
		WordBuffer[nWordNum].wordRect.top    = NextWordRect.top;
		WordBuffer[nWordNum].wordRect.bottom = NextWordRect.bottom;

		CopyRect(&PrevWordRect, &NextWordRect);//zhhn

		nWordNum++;

		if (nCurrentWord >= cbLen - 1)
			break;
	}
	
	GetStringLeftRight(hMemDC, szBuff, 0, x, &PrevWordRect, lpDx);
	for (i = 0; i < cbLen; i++)
	{
		nTempLen = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, szBuff, i + 1, lpTemp, 256);
		GetStringLeftRightW(hMemDC, lpWideCharStr, nTempLen, x, &NextWordRect, lpDx);

		pnMemDCCharLeft[nLen + i]  = PrevWordRect.right;
		pnMemDCCharRight[nLen + i] = NextWordRect.right;
	
		CopyRect(&PrevWordRect, &NextWordRect);
	}
}

void GetMemWordStringRect(int nWordCode, int nOffset, LPRECT lpStringRect)
{
	POINT  WndPos;
	int    nNum;

	if (nWordCode >= nWordNum)
	{
		lpStringRect->left   = 0;
		lpStringRect->right  = 0;
		lpStringRect->top    = 0;
		lpStringRect->bottom = 0;
		
		return;
	}
	
	CopyRect(lpStringRect, &(WordBuffer[nWordCode].wordRect));
	if (nOffset != MEMDC_TOTALWORD)
	{
		nNum = WordBuffer[nWordCode].nBegin + nOffset;
		lpStringRect->left = pnMemDCCharLeft[nNum];
		lpStringRect->right = pnMemDCCharRight[nNum];
	}
	
	WndPos.x = g_dwDCOrg.x;
	WndPos.y = g_dwDCOrg.y;
	
	lpStringRect->top    = lpStringRect->top    + WndPos.y;
	lpStringRect->bottom = lpStringRect->bottom + WndPos.y;
	lpStringRect->left   = lpStringRect->left   + WndPos.x;
	lpStringRect->right  = lpStringRect->right  + WndPos.x;
}

void CheckMemDCWordBuffer(HDC hdcdest, HDC hdcSrc)
{
	int i;
	DWORD dwReturn;
	
	for (i = 0; i < nWordNum; i++)
	{
		if (WordBuffer[i].hMemDC == hdcSrc)
		{
			dwReturn = CheckMouseInMemDCWord(i);
		}
		else
		{
			if (CheckDCWndClassName(hdcdest))
			{
				dwReturn = CheckMouseInMemDCWord(i);
			}
		}

		//added by zhhn on 2000.2.2

		if (dwReturn == HAS_CURMOUSEWORD || g_bMouseInTotalWord)
		{
			PostMessage(g_hNotifyWnd, BL_HASSTRING, 0, 0l);
			//break;	//Modified by ZHHN on 2000.4 in order to get phrase
		}

		//added end
	}                               
}

__inline BOOL CheckDCWndClassName(HDC hDC)
{
	HWND hWndFromDC;
	char szClassName[MAX_CLASS_NAME];
	int i;

	hWndFromDC = WindowFromDC(hDC);
	GetClassName(hWndFromDC, szClassName, MAX_CLASS_NAME);

	for (i = 0; i < g_nWorkOnClassNum; i++)
	{
		if (lstrcmp(szClassName, (LPSTR)g_szWorkOnClassName[i]) == 0)
		{
			return TRUE;
		}
    }
	return FALSE;
}

DWORD CheckMouseInMemDCWord(int nWordCode)
{
	RECT  StringRect;

	GetMemWordStringRect(nWordCode, MEMDC_TOTALWORD, &StringRect);

	if (  (StringRect.left   <= g_CurMousePos.x)
		&&(StringRect.right  >= g_CurMousePos.x)
		&&(StringRect.top    <= g_CurMousePos.y)
		&&(StringRect.bottom >= g_CurMousePos.y))
	{
		switch (WordBuffer[nWordCode].CharType)
		{
			case CHAR_TYPE_HZ:
			case CHAR_TYPE_ASCII:
				 CopyWord(g_szCurWord, szMemDCWordBuff, WordBuffer[nWordCode].nBegin, WordBuffer[nWordCode].nEnd);
				 g_CurWordRect.left   = StringRect.left;
				 g_CurWordRect.right  = StringRect.right;
				 g_CurWordRect.top    = StringRect.top;
				 g_CurWordRect.bottom = StringRect.bottom;
				 
				 g_nCurCaretPlace = -1;
				 CalculateCaretPlaceInMemDCWord(nWordCode);
				 
				 break;
			case CHAR_TYPE_OTHER:
				 break;
		}
		AddToTotalWord(szMemDCWordBuff, 
						0,  // Ignor
						WordBuffer[nWordCode].nBegin, 
						WordBuffer[nWordCode].nEnd, 
						WordBuffer[nWordCode].CharType, 
						StringRect, 
						TRUE);

/*
		{
			char cBuffer[0x100];
			wsprintf(cBuffer, "We got the word here\n");
			OutputDebugString(cBuffer);
		}
*/

		if (  (WordBuffer[nWordCode].CharType == CHAR_TYPE_OTHER)
		    &&(g_CurMousePos.x == StringRect.right))
		{
			return NO_CURMOUSEWORD;
		}
		return HAS_CURMOUSEWORD;
	}
	else
	{
	}

	AddToTotalWord(szMemDCWordBuff, 
				   0,  // Ignor
				   WordBuffer[nWordCode].nBegin, 
				   WordBuffer[nWordCode].nEnd, 
				   WordBuffer[nWordCode].CharType, 
				   StringRect, 
				   FALSE);

	return NO_CURMOUSEWORD;   
}

DWORD CalculateCaretPlaceInMemDCWord(int nWordCode)
{
	RECT  StringRect;
	int   i;

	if (  (WordBuffer[nWordCode].CharType == CHAR_TYPE_HZ) 
	    &&(WordBuffer[nWordCode].nBegin    == WordBuffer[nWordCode].nEnd))
	{
		g_nCurCaretPlace = -1;
		return 0L;
	}

	GetMemWordStringRect(nWordCode, MEMDC_TOTALWORD, &StringRect);
	
	if (CalcCaretInThisPlace(g_CurMousePos.x, StringRect.left))
	{
		g_nCurCaretPlace = 0;
		return 0l;
	}

	if (CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right))
	{
		g_nCurCaretPlace = WordBuffer[nWordCode].nEnd - WordBuffer[nWordCode].nBegin + 1;
		return 0l;
	}
	
	for (i = WordBuffer[nWordCode].nBegin; i <= WordBuffer[nWordCode].nEnd; i++)
	{
		GetMemWordStringRect(nWordCode, i - WordBuffer[nWordCode].nBegin, &StringRect);
		if (CalcCaretInThisPlace(g_CurMousePos.x, StringRect.right))
		{
			g_nCurCaretPlace = i - WordBuffer[nWordCode].nBegin + 1;
			return 0l;
		}
	}
	
	return 0L;
}