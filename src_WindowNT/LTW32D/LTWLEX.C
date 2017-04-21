#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "text.h"
#include "ltwlex.h"

#define MAXBUFSIZE 1024

typedef struct _rule{
		char tail[10];
		int n;
		char rep[10];
		int fail_off;
	} RULE;

typedef struct _bufbak{
		LPSTR buf;
		int engPtr;
		struct _bufbak *last;
	} BUFBAK;	// buf backup stack

HANDLE hLex, hLexMap;
LPSTR lpLex;
extern LPSTR gBuf;
extern char engStr[0x40];
extern int engPtr;
BUFBAK *gBufBak=NULL;
extern char far errs[100];


//////////////////////////////////////
//	LexInit()
//	Open & map LEX.DAT
//
int DllExport WINAPI LexInit()
{
	/// open and map LEX.DAT
	hLex = CreateFile("lex.dat", GENERIC_READ, 0, NULL, OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL, NULL);
	if (hLex==INVALID_HANDLE_VALUE){
		strcpy(errs, "Error: cannot open the data file DICT.DAT.");
		return 0;
	}
	hLexMap = CreateFileMapping(hLex, NULL, PAGE_READONLY, 0, 0, NULL);
	if (hLexMap==NULL){
//		CloseHandle(hLex);
		strcpy(errs, "Error: cannot map the data file DICT.DAT.");
		return 0;
	}
	lpLex = (LPSTR)MapViewOfFile(hLexMap, FILE_MAP_READ, 0, 0, 0);
	if (lpLex==NULL){
		strcpy(errs, "Error: cannot map the data file DICT.DAT.");
//		CloseHandle(hLexMap);
//		CloseHandle(hLex);
		return 0;
	}

	/// alloc global buffer
	if (!(gBuf=(LPSTR)malloc(MAXBUFSIZE))) return 0;
	gBufBak = NULL;
	
	return 1;
}

//////////////////////////////////////
//	LexExit()
//	Close LEX.DAT
//
void DllExport WINAPI LexExit()
{
	UnmapViewOfFile(lpLex);
	CloseHandle(hLexMap);
	CloseHandle(hLex);
	free(gBuf);
}

//////////////////////////////////////////
//	DoSearch()
//	return found string's CAT
//	trans inf gBuf
//
WORD DoSearch(char *s)
{
	WORD r=0;
	LPSTR p=lpLex+0x100;
	WORDHEAD *wh=(WORDHEAD *)p, whend={0};
	int size;
	
	while(wh->wordName!='ÿ' && lstrcmpi(&wh->wordName, s)){
		// not match
		p += wh->nextWord;
		wh = (WORDHEAD *)p;
	}
	if (wh->wordName=='ÿ') return 0;
	
	r = wh->wordCat;
	size = (int)wh->nextWord;
	if (wh->wordFeature & MUL){
		wh = (WORDHEAD *)((char *)wh + wh->nextWord);
		while(wh->wordName!='ÿ' && !lstrcmpi(&wh->wordName, s)){
			size += wh->nextWord;
			r |= wh->wordCat;
			wh = (WORDHEAD *)((char *)wh + wh->nextWord);
		}
	}
	memcpy(gBuf, p, size);
	memcpy(gBuf+size, &whend, sizeof(WORDHEAD));  // make end tag
	
	return r;
}

/////////////////////////////////////
// 	strnicmp_tail()
//
int strnicmp_tail(char far *s1, char far *s2, int n);
/*{
	int ls1=strlen(s1);
	LPSTR p=s1+ls1-n;

	return (strnicmp(p, s2, n));
}	*/

//////////////////////////////////////
//	LexSearch()
//	return found string's CAT
//
WORD LexSearch(char *s)
{
	WORD r;
	int no=0, dob=0;	
	BOOL first=TRUE;
	WORDHEAD *wh, whend={0};
	static RULE rule[]=
		{ {"s", 1, "", 6},		// -s						0
		{"e", 1, "", 1},		// -es
		{"i", 1, "y", 1},		// -ies
		{"v", 1, "f", 1},		// -ves ---> -f
		{"f", 1, "fe", 1},		// -ves ---> -fe
		{"", 0, "", -1},

		{"d", 1, "", 5},		// -d						6
		{"e", 1, "", 1},		// -ed
		{"i", 1, "y", 1},		// -ied ---> -y
		{"??", 2, "?", 1},		// -XXed ---> -X
		{"", 0, "", -1},

		{"ing", 3, "e", 6},		// -ing						11
		{"e", 1, "", 1},		// -ing ---> -e
		{"??", 2, "?", 1},		// -XXing ---> -X
		{"y", 1, "ie", 1},		// -ying ---> -ie
		{"ick", 3, "ic", 1},	// -cking ---> -ic
		{"", 0, "", -1},

		{"ly", 2, "l", 4},		// -ly ---> -l				17
		{"l", 1, "e", 1},		// -ly
		{"e", 1, "", 1},		// -ly ---> -e
		{"i", 1, "y", 1},		// -ily ---> -y
		{"", 0, "", -1} };

search_again:
	if (r=DoSearch(s)){
		if (!first){
			wh = (WORDHEAD *)gBuf;
			if (no>=17 && (r & ADJ)){				///  ADJ+ly == ADV
				wh = GetByCat(wh, ADJ);
				memcpy(gBuf, (char *)wh, wh->nextWord);
				wh = (WORDHEAD *)gBuf;
				memcpy(gBuf+wh->nextWord, &whend, sizeof(WORDHEAD));
				wh->wordCat = ADV;
				r = ADV;
			}
			else if (no>=11 && (r & V)){
				wh = GetByCat(wh, V);		// V-ING
				wh->wordFeature |= ING;
				r = V;
			}
			else if (no>=6 && (r & V)){
				wh = GetByCat(wh, V);
				wh->wordFeature |= (PAST|PP);		// V-ED
				r = V;
			}
			else{
				WORD tr = r;

				r = 0;
				if (tr & V){
					wh = GetByCat((WORDHEAD *)gBuf, V);
					wh->wordFeature |= PRES;		// V-s
					r = V;
				}
				if (tr & N){
					wh = GetByCat((WORDHEAD *)gBuf, N);
					wh->wordFeature |= PL;		// N-s
					r |= N;
				}
				wh = (WORDHEAD *)gBuf;
			}
		}
		return r;
	}
	first = FALSE;

do_again:
	if (rule[no].fail_off==-1) return 0;	/// not match & not tail-do
	if (rule[no].rep[0]=='?'){
		int l=lstrlen(s);
		char p=s[l-1];

		rule[no].rep[0]=rule[no].tail[0]=rule[no].tail[1]=p;
		dob++;
	}
	if (!strnicmp_tail(s, rule[no].tail, rule[no].n)){
		int l=lstrlen(s);
		
		lstrcpy(s+l-rule[no].n, rule[no].rep);
		no++;
		if (dob){
		   rule[no].rep[0]=rule[no].tail[0]=rule[no].tail[1]='?';
		   dob--;
		}
		goto search_again;
	}
	else{
		no += rule[no].fail_off;
		if (dob){
		   rule[no].rep[0]=rule[no].tail[0]=rule[no].tail[1]='?';
		   dob--;
		}
		goto do_again;
	}
}	

/////////////////////////////////////////
//	Lex()
//	return word's CAT , trans in gBuf, word from engStr
//
WORD Lex()
{			
	int i, j;
	char s[20];
	WORD r, f;
	WORDHEAD *wh=(WORDHEAD *)gBuf;
	
	/// get word from engStr
	for(i=0; engStr[engPtr+i] && engStr[engPtr+i]==' '; i++);
	for(j=i; engStr[engPtr+j] && engStr[engPtr+j]!=' '; j++);
	memcpy(s, engStr+engPtr+i, j-i);
	s[j-i] = '\0';
	engPtr += j;

	if (!(r=LexSearch(s))) return r;
	if (wh->wordFeature & BT){		// deal with BT
		f = wh->wordFeature;		
		strcpy(s, (char *)wh+wh->offsBody);
		r = LexSearch(s);			// search root
		wh->wordFeature |= f;		// combin features of BT and root
	}

	return r;
}

////////////////////////////////////////
// 	Backup()
// 	backup gBuf & engPtr
//
int Backup()
{
	BUFBAK *bb;
	
	if (!(bb = (BUFBAK *)malloc(sizeof(BUFBAK)))) return 0;
	if (!(bb->buf = (LPSTR)malloc(MAXBUFSIZE))){
		free (bb);
		return 0;
	}
	memcpy(bb->buf, gBuf, MAXBUFSIZE);
	bb->engPtr = engPtr;
	bb->last = gBufBak;
	gBufBak = bb;
	
	return 1;
}

////////////////////////////
//	Restore()
//	restore gBuf & engPtr	
//
int Restore()
{
	BUFBAK *bb=gBufBak;
	
	if (!gBufBak) return 0;
	memcpy(gBuf, bb->buf, MAXBUFSIZE);
	engPtr = bb->engPtr;
	
	return 1;
}

///////////////////////////////
//	Pop()
//	pop gBufBak
//
int Pop()
{
	BUFBAK *bb;
	
	if (!gBufBak) return 0;
	bb = gBufBak;
	gBufBak = bb->last;
	free(bb->buf);
	free(bb);
	
	return 1;
}	

/////////////////////////////////
//	NewNT()
//	return a empty NT struct
//
nts_t *NewNT()
{
	return (nts_t *)malloc(sizeof(nts_t));
}

/////////////////////////////////
//	GetByCat()
//	return trans by CAT (which in gBuf), if fail return NULL
//
WORDHEAD *GetByCat(WORDHEAD *w, WORD cat) // cat must is a uni-cat
{
	WORDHEAD *wh=w;

	if (wh->wordFeature & MUL){			/// MUL do
		while(wh->nextWord && !(wh->wordCat & cat))
			wh = (WORDHEAD *)((char *)wh + wh->nextWord);
		if (wh->nextWord) return wh;
		else return NULL;
	}
	else if (wh->wordCat & cat) return wh;	/// one CAT do
	else return NULL;
}

/////////////////////////////////
//	GetByClass()
//	return trans by N-Class (which in gBuf), if fail return NULL
//
char *GetByClass(WORDHEAD *w, DWORD c)	/// use for N & P
{
	NOUNPREPBODY *npb;
	NOUNPREPITEM *npi;
	int i, j;
	static char s[60], *s2;

	s[0]='\0';
	if (!(w->wordCat & (N | P))) return s; //not a noun or prep
	npb = (NOUNPREPBODY *)((char *)w + w->offsBody);
	npi = (NOUNPREPITEM *)((char *)w + npb->offsItem);
	if (c == ALL){		// return all chinese 
		s2 = (char *)w+npi[0].offsChn;		// format:
		strcpy(s, s2);						//    P  chn1(chn2,chn3,...,chnN)\0
		strcat(s, "[");						//    N  perWord chn1(chn2,chn3,...,chnN)\0
		for(i=1; i<npb->numItem; i++){		
			s2 = (char *)w+npi[i].offsChn;
			if (w->wordCat & N){
				for(j=0; s2[j]!=' '; j++);
				s2 = s2 + j + 1;
			}
			strcat(s, s2);
			strcat(s, ",");
		}
		if (npb->numItem>1){ s[strlen(s)-1]=']'; s[strlen(s)]='\0'; }
		else s[strlen(s)-1]='\0';		
		return s;
	}
	i = 0;
	while(i<npb->numItem && 
		!((npi[i].classVal & c) && !(npi[i].classVal & ~c))) i++;	// npi[i].classVal must be subclass of c
	if (i >= npb->numItem) return NULL;	// no certain class
	else return ((char *)w+npi[i].offsChn);
}

//////////////////////////////////
//	GetOtherChn()
//	return PRO, ADJ, ADV, ART, AUX chn
//
char *GetOtherChn(WORDHEAD *w)
{
	PROBODY *pb;

	if (w->wordCat & PRO){
		pb = (PROBODY *)((char *)w + w-> offsBody);
		return ((char *)w + pb->offsChn);
	}
	else return ((char *)w + w->offsBody);
}

//////////////////////////////////
//	InsertNP()
//	insert a chn-str into another in the place tag by a given tag
//	
int InsertNP(LPSTR s1, LPSTR s2, char c) 
{
	int m, n, i, j, l=lstrlen(s1);

	for(i=0; i<l; i++)
		if (s1[i]==c) break;
	if (i<l){
		m = l + lstrlen(s2) - 1 - 1;
		n = lstrlen(s1) -i -1;
		l--;
		s1[m+1]='\0';
		for(j=0; j<n; j++) s1[m-j]=s1[l-j]; /// move s1 tail
		for(j=0; j<lstrlen(s2); j++) s1[i+j]=s2[j]; /// insert
		return 1;
	}
	else return 0;
}	
	
////------------------------------------------------------------------

	
		