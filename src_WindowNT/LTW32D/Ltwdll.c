#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "windows.h"
#include "windowsx.h"
#include "res.h"
#include "text.h"
#include "ltwlex.h"

#define IDXOFF		(0x100)
#define	ITEMSIZE	(0x40)
#define INDEXSIZE	(0x40 * 0x80)
#define INDEXCOUNT	(1844)

#define DIRECTORY	""

typedef struct _trans{
			char eng[0x40];
			char chn[0x80];
            char pron[0x20];
		} TRANS, *PTRANS, *FPTRANS;		

typedef struct _rule{
			char tail[10];
			char n;
			char rep[10];
			int fail_off;
		} RULE, *PRULE;

char far errs[100];
char idx[INDEXCOUNT+1][11];
DWORD addr[INDEXCOUNT+1];
BOOL fFTHZ=FALSE, fListSet=FALSE, fLex=FALSE;
DWORD dwFTOff = 0x0;
HWND hwBMP, hList; // , hDlg;
HANDLE hDictFile, hDictMap;
LPSTR lpDictAddr;
HANDLE hHeap;
DllExport HWND hDlg;
DllExport BOOL fDoing;
DllExport TRANS trans = { "", "", ""};	///  use for dsiplay err msg
extern char engStr[0x40];
extern int engPtr;


RULE rule[32]={
		{"eld", 3, "old", 2},
		{"", 0, "", -1},

		{"ood", 3, "and", 2},
		{"", 0, "", -1},

		{"ound", 4, "ind", 2},
		{"", 0, "", -1},

		{"ilt", 3, "ild", 2},
		{"", 0, "", -1},

		{"ook", 3, "ake", 2},
		{"", 0, "", -1},

		{"s", 1, "", 6},
		{"e", 1, "", 1},
		{"i", 1, "y", 1},
		{"v", 1, "f", 1},
		{"f", 1, "fe", 1},
		{"", 0, "", -1},

		{"d", 1, "", 5},
		{"e", 1, "", 1},
		{"i", 1, "y", 1},
		{"??", 2, "?", 1},
		{"", 0, "", -1},

		{"ing", 3, "e", 6},
		{"e", 1, "", 1},
		{"??", 2, "?", 1},
		{"y", 1, "ie", 1},
		{"ick", 3, "ic", 1},
		{"", 0, "", -1},

		{"ly", 2, "l", 4},
		{"l", 1, "e", 1},
		{"e", 1, "", 1},
		{"i", 1, "y", 1},
		{"", 0, "", -1}		};


int strnicmp_tail(char far *s1, char far *s2, int n)
{
	int ls1 = lstrlen(s1);
	char *p = s1 + ls1 - n;

	return (_fstrnicmp(p, s2, n));
}


int DllExport WINAPI init(void)
{
	HFILE hh;
	WORD i;
	BYTE tmp[0x40];		// use this line special for isalpha()
	char r[]=" (c) by Guan Tao";

// check file DICT.DAT
	hh = _lopen(DIRECTORY"dict.dat", OF_READ);
	if (hh == HFILE_ERROR){
		 strcpy(errs, "Error: cannot open the data file DICT.DAT.");
		 return 0;
	}

	_llseek(hh, 0x30, SEEK_SET);
	_lread(hh, tmp, 0x11);
	tmp[0x10]='\0';
	if(strcmp(tmp, r)){
		strcpy(errs, "Error: the data file DICT.DAT is destroyed.");
		return 0;
	}
	_lclose(hh);


// load index
	hh = _lopen(DIRECTORY"dict.idx", OF_READ);
	if (hh == HFILE_ERROR){
		strcpy(errs, "Error: cannot open the index file DICT.IDX.");
		return 0;
	}

	_llseek(hh, 0x100, SEEK_SET);
	for (i=0; i<INDEXCOUNT; i++){
		_lread(hh, idx[i], 11);
		_lread(hh, &addr[i], sizeof(DWORD));
		addr[i] += 0x100;
	}
	strcpy(idx[INDEXCOUNT], "牋");
	_lclose(hh);

// alloc heap use for search
	hHeap = HeapCreate(HEAP_GENERATE_EXCEPTIONS, 0x4000, 0);
	if (hHeap==NULL){
		strcpy(errs, "Error: no enough memory.");
		return 0;
	}

// Open & Map file DICT.DAT
	hDictFile = CreateFile(DIRECTORY"dict.dat", GENERIC_READ,
		0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hDictFile == INVALID_HANDLE_VALUE){
		strcpy(errs, "Error: cannot open the data file DICT.DAT.");
		return 0;
	}

	hDictMap = CreateFileMapping(hDictFile, NULL, PAGE_READONLY,
		0, 0, NULL);
	if (hDictMap == NULL){
		strcpy(errs, "Error: cannot map the data file DICT.DAT.");
//		CloseHandle(hDictFile);
		return 0;
	}

	lpDictAddr = (LPSTR)MapViewOfFile(hDictMap, FILE_MAP_READ,
		0, 0, 0);
	if (lpDictAddr == NULL){
		strcpy(errs, "Error: cannot map the data file DICT.DAT.");
//		CloseHandle(hDictMap);
//		CloseHandle(hDictFile);
		return 0;
	}						

// the last addr of index
	addr[INDEXCOUNT] = GetFileSize(hDictFile, NULL);  

	return 1;
}



int MySearch(FPTRANS s)
{
	int k, low, high, mid, size;
	WORD i, j, l, cur;
	LPSTR m, lpAd;
	static WORD wIndex = 2000;
	BOOL fAddStr = TRUE;

	if (!isalpha(s->eng[0])){
		lstrcpy(errs, "出错：非法输入！");
		return 0;
	}


	low = 0;
	high = 1844;
	while (low <= high){
		mid = (low + high) / 2;
		if ((k=_fstricmp(s->eng, idx[mid])) < 0) high = mid - 1;
		else if (k > 0) low = mid + 1;
		else{
			j = mid;
			break;
		}
	}

	if (low > high){
		j = (low<high)?low:high;
		if (_fstricmp(s->eng, idx[j]) < 0) j--;
	}

// allo temp mem
	m = (LPSTR)HeapAlloc(hHeap, HEAP_ZERO_MEMORY, 0x4000);
	if (m==NULL){
		strcpy(errs, "出错：内存不足！");
		return 0;
	}

	size = addr[j+1]-addr[j]+0x80;
	memcpy(m, (LPSTR)lpDictAddr+addr[j], size);

// unlock
	lpAd = m;
	for (l=0; l<size; l++, lpAd++)
		_asm{
			push	eax
			push	ebx
			mov		ebx, [lpAd]
			mov		al, byte ptr [ebx]
			not		al
			xor		al, 76h
			rol		al, 1
			xor		al, 0f5h
			mov		byte ptr [ebx], al
			pop		ebx
			pop		eax
		}				

	if (j == wIndex) fAddStr = FALSE;
	else wIndex = j;

	if (fDoing) fAddStr = FALSE;

	i = j = 0;
    if (!fListSet){
		hList = GetDlgItem(hDlg, IDC_LIST);
		fListSet = TRUE;
	}
	if (fAddStr) ListBox_ResetContent(hList);

// add first string into list box
	j++;
	if (fAddStr){
		for (mid=0; *(m+mid)!=0x01; mid++);
		*(m+mid) = 0x0;
		ListBox_AddString(hList, m+i);
		*(m+mid) = 0x1;
	}

// finding
	while(k=_fstrnicmp(s->eng, m+i, _fstrlen(s->eng))>0){
		i += (_fstrlen(m+i) + 1);
		if (!isalpha(m[i])){
			lstrcpy(errs, "出错：单词越界！");
			HeapFree(hHeap, 0, (LPVOID)m);
			return 0;
		}
// add string into list box
		j++;
		if (fAddStr){
			for (mid=0; *(m+i+mid)!=0x01; mid++);
			*(m+i+mid) = 0x0;
			ListBox_AddString(hList, m+i);
			*(m+i+mid) = 0x1;
		}
	}

	cur = j - 1;
	_fstrcpy(s->chn, m+i);

// add rest string into list box
	if ((fAddStr) && (j < 0x40))
		for (k=j; k<0x40; k++){
			i += (_fstrlen(m+i) + 1);
			for (mid=0; *(m+i+mid)!=0x01; mid++);
			*(m+i+mid) = 0x0;
			ListBox_AddString(hList, m+i);
			*(m+i+mid) = 0x1;
		}		

	if (!fDoing) ListBox_SetCurSel(hList, cur);

	HeapFree(hHeap, 0, (LPVOID)m);

	return 1;
}


int search_plus(FPTRANS s)
{
	int k;
	char eg[0x40];
	int no=0, dob=0, first=1;
	TRANS t;

	lstrcpy(t.eng, s->eng);

search_again:

	if (!MySearch((FPTRANS)&t)) return 0;

	if (first){
		lstrcpy(s->chn, t.chn);
		first--;
	}

	/* get English */
	for(k=0; t.chn[k]!=0x01; k++, eg[k-1]=t.chn[k-1]);
	eg[k] = '\0';

	if (!_fstricmp(t.eng, eg)){
		lstrcpy(s->chn, t.chn);
		return 1;
	}

do_again:

	if (rule[no].fail_off == -1) return 1;

	if (rule[no].rep[0] == '?'){
		int l = lstrlen(t.eng);
		char p = t.eng[l-1];

		rule[no].rep[0] = rule[no].tail[0] = rule[no].tail[1] = p;
		dob++;
	}
	if (!strnicmp_tail(t.eng, rule[no].tail, rule[no].n)){
		int l = lstrlen(t.eng);

		lstrcpy(t.eng+l-rule[no].n, rule[no].rep);
		no++;

		if (dob){
			rule[no].rep[0] = rule[no].tail[0] = rule[no].tail[1] = '?';
			dob--;
		}

		goto search_again;
	}
	else{
		no += rule[no].fail_off;

		if (dob){
			rule[no].rep[0] = rule[no].tail[0] = rule[no].tail[1] = '?';
			dob--;
		}

		goto do_again;
	}
}


void DllExport WINAPI makestr(char far *s)			// also called by LTHOOK32.DLL
{
	int i, j, k, l, m;

	l = _fstrlen(s);
	for (i=0; (s[i]==' ') && (s[i]!='\0'); i++);
	if (i == l){				// s is string of blank
		s[1]='\0';
		return;
	}
	for (j=0; j<=l-i; s[j]=s[i+j], j++);	// cut the blanks at the head
	if (!isalpha(s[0])) return;
	l -= i;
	k = 0;
	while (k<l){	// cut the additional blanks within the string
		for (m=k; (s[m]!=' ') && (s[m]!='\0'); m++);
		if (m == l) break;
		for (i=0; (s[m+i]==' ') && (s[m+i]!='\0'); i++);
		if ((m+i) == l){
			s[m]='\0';	// cut the blanks at the tail
			break;
		}
		if (i>1) for (j=0; j<=l-m-i; s[m+j+1]=s[m+i+j], j++);
		k = m + 2;
		i--;
		l -= i;
	}
}

int DllExport WINAPI translate(FPTRANS t)
{
	int i, j, k, max=0;
	ntsp_t n, v, p, a, m=NULL;
	char tmpp[0x40];

	makestr(t->eng); // ready the input string
	if (lstrlen(t->eng)==0) return 0;
	lstrcpy(tmpp, t->eng);	
	if (fLex){	// use lex_parser to translate
		lstrcpy(engStr, t->eng);
		engPtr = 0;
		n = np_parse(ALL);
		if (n && engPtr>max)
			{ m = n; max = engPtr; strcpy(t->chn,"NP. "); }
		engPtr = 0;
		a = adjp_parse();
		if (a && engPtr>max)
			{ m = a; max = engPtr; strcpy(t->chn,"ADJP. "); }
		engPtr = 0;
		p = pp_parse();
		if (p && engPtr>max)
			{ m = p; max = engPtr; strcpy(t->chn,"PP. "); }
		engPtr = 0;
		v = vp_parse();
		if (v && engPtr>max)
			{ m = v; max = engPtr; strcpy(t->chn,"VP. "); }
		
		if (m){
			engStr[max] = '\0';
			strcpy(t->eng, engStr);
			strcat(t->chn, m->chn);
			t->pron[0]='\0';
			return 1;
		}
	}

	lstrcpy(t->eng, tmpp);		

	if (!search_plus(t)) return 0;

	/* get English */
	for(k=0; t->chn[k]!=0x01; k++, t->eng[k-1]=t->chn[k-1]);
	t->eng[k] = '\0';

	i = lstrlen(t->chn) - 1;
	while ((t->chn[i] != 0x01) && (t->chn[i] !=0x02)) i--;
	/* get pronance */
	if (k == i)	t->pron[0] = '\0';
	else{
		for(j=k; t->chn[j]!=0x02; t->pron[j-k]=t->chn[j+1], j++);
		t->pron[j-k-1] = '\0';
	}
	/* get Chinese */
	for(j=i; t->chn[j]; t->chn[j-i]=t->chn[j+1], j++);

	return 1;
}


UINT DllExport WINAPI DrawHZ(HDC hDC, int nX, int nY,
	BYTE FAR *buf, int MaxWidth, COLORREF color)
{
	BYTE bits[0x20] = { 0 };
	HBITMAP hBitmap, hOldBitmap;
	int nP, i, w;
	HDC hMemDC;
	DWORD dwPos;
	HFILE hFile;
    COLORREF col;

	nP = 0;
	if (!buf[nP]) return 1;

	hFile = _lopen(DIRECTORY"ec.fnt", OF_READ);
	if (hFile == HFILE_ERROR){
		strcpy(errs, "Error: cannot open the font file EC.FNT.");
		return 0;
	}

	hBitmap = CreateBitmap(16, 16, 1, 1, bits);
    hMemDC = CreateCompatibleDC(hDC);
	hOldBitmap = SelectObject(hMemDC, hBitmap);

    col = GetTextColor(hDC);
	while (buf[nP]){
		if (buf[nP]=='\r'){
			nY += 18;
			nX = 3;
			nP++;
			if (buf[nP]=='\n') nP++;
			SetTextColor(hDC, color);
			continue;
		}
		if (buf[nP] & 0x80){
			dwPos = (DWORD)((buf[nP] - 0xa1) * 0x5e + (buf[nP+1] - 0xa1)) * 0x20;
			dwPos += dwFTOff;
			nP += 2;
			_llseek(hFile, dwPos, 0);
			_lread(hFile, bits, 0x20);

			for (i=0; i<0x20; i++) bits[i] = ~bits[i];

			SetBitmapBits(hBitmap, 0x20, bits);
			SelectObject(hMemDC, hBitmap);
			if (nX + 18 >= MaxWidth){
				nX = 3;
				nY += 18;
			}
			BitBlt(hDC, nX, nY, 16, 16, hMemDC, 0, 0, SRCCOPY);
			nX += 18;
		}
		else{
			SIZE size;

			for (i=0;
				(!(buf[nP+i] & 0x80)) && (buf[nP+i]) && (buf[nP+i]!='\r');
				i++);
			GetTextExtentPoint32(hDC, buf+nP, i, (LPSIZE)&size);
			w = size.cx;
			if (nX + w> MaxWidth){
				nX = 3;
				nY += 18;
			}
//			if (!nP) SetTextColor(hDC, color);
			TextOut(hDC, nX, nY, buf+nP, i);
			nX += w;
			nP += i;
//			SetTextColor(hDC, col);
		}
	}

	SelectObject(hMemDC, hOldBitmap);
	DeleteObject(hBitmap);
	DeleteDC(hMemDC);
	_lclose(hFile);
	SetTextColor(hDC, col);

	return	1;
}


void DllExport WINAPI DisplayErr(BOOL HZ)
{
	HWND hTemp;

	MessageBeep(MB_ICONQUESTION);
	if (!HZ){
		MessageBox(NULL, errs,
			"Little Translator",
			MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
	}
	else{
		strcpy(trans.chn, errs);
		trans.eng[0] = '\0';
		trans.pron[0] = '\0';
		InvalidateRect(hwBMP, NULL, TRUE);
		UpdateWindow(hwBMP);
		hTemp = GetDlgItem(hDlg, IDC_ENGLISH);
		SetFocus(hTemp);
		Edit_SetSel(hTemp, 0, -1);
	}
}


void DllExport WINAPI ChangeHZStyle(void)
{
	if (fFTHZ){
		fFTHZ = FALSE;
		dwFTOff = 0x0;
	}
	else{
		fFTHZ = TRUE;
		dwFTOff = 0x40000;
	}
}

void DllExport WINAPI ChangeLexStatus(void)
{
	if (fLex) fLex = FALSE;
	else fLex = TRUE;
}

void DllExport WINAPI SetDlgHandle(HWND hwndDlg, HWND hwndBMP)
{
	hDlg = hwndDlg;
	hwBMP = hwndBMP;
}

void DllExport WINAPI SetListHandle(HWND hwndList)
{
	hList = hwndList;
    fListSet = TRUE;
}

BOOL WINAPI DllMain(HANDLE hInstance, DWORD fdwReason, LPVOID lpvResrved)
{
	switch (fdwReason){
	case DLL_PROCESS_ATTACH:
		hDictFile = INVALID_HANDLE_VALUE;
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		if (hDictFile!=INVALID_HANDLE_VALUE){
			UnmapViewOfFile(lpDictAddr);
			CloseHandle(hDictMap);
			CloseHandle(hDictFile);
		}
		break;
	}

	return TRUE;
}

