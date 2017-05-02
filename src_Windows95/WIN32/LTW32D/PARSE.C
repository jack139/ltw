#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "text.h"
#include "ltwlex.h"

LPSTR gBuf;
char engStr[0x40];
int engPtr;

/*
	rule match should from complex one to simple one
*/


//////////////////////////////////////////////////////////////
//	np_parse()
//	parse a NP
//
ntsp_t DllExport WINAPI np_parse(DWORD want)
{
	WORD cat;
	nts_t *ntp, *ntp2;
	WORDHEAD *wh=(WORDHEAD *)gBuf;
	PROBODY *pb;

	Backup();
	ntp = NewNT();
	if (!(cat=Lex())) goto error;
	if (cat & PRO){		// NP-->PRO
		wh = GetByCat(wh, PRO);
		pb = (PROBODY *)((char *)wh + wh->offsBody);
		if ((pb->classVal & want) && !(pb->classVal & ~want)){
			ntp->c = np;
			lstrcpy(ntp->chn, GetOtherChn(wh));
			goto succ;
		}
		else goto error;
	}
	if (cat & ART){	// NP-->ART NPA
		ntp->c = np;
		lstrcpy(ntp->chn, GetOtherChn(wh));
		if (ntp2=npa_parse(want)){
			if (ntp2->info.np.num==pl) lstrcat(ntp->chn, "些");
			if (ntp2->info.np.one[0]) lstrcat(ntp->chn, ntp2->info.np.one);
			lstrcat(ntp->chn, ntp2->chn);
			free(ntp2);
			goto succ;
		}
		else goto error;
	}
	else{	// NP-->NPA
		Restore();
		if (ntp2=npa_parse(want)){
			ntp2->c = np;
/*			if (ntp2->info.np.num==pl){
				lstrcpy(ntp->chn, ntp2->chn);
				lstrcpy(ntp2->chn, "一些");
				if (ntp2->info.np.one[0]) lstrcat(ntp2->chn, ntp2->info.np.one);
				lstrcat(ntp2->chn, ntp->chn);
			}		*/
			free(ntp);
			ntp = ntp2;
			goto succ;
		}
		else goto error;
	}

succ:
	Pop();
	return ntp;

error:
	free(ntp);
	Restore();
	Pop();
	return NULL;
}


//////////////////////////////////////////////////////////////
//	npa_parse()
//	parse a NPA
//
ntsp_t DllExport WINAPI npa_parse(DWORD want)
{
	nts_t *ntp, *ntp2;
	WORDHEAD *wh=(WORDHEAD *)gBuf;
	WORD cat;

	Backup();
	ntp = NewNT();
	if (ntp2=adjp_parse()){			// NPA-->ADJP N
		if (cat=Lex()){
			if (cat & N){
				char *t, *t2;
				int i;

				wh = GetByCat(wh, N);
				ntp->c = npa;
				t = GetByClass(wh, want);
//				if (!t) t = GetByClass(wh, ALL);
				if (!t) goto error;			// strict match class
				for(i=0; t[i]!=' '; i++);  // per-word and Chinese is devided by a blank
				t[i] = '\0';
				t2 = t + i + 1;
				lstrcpy(ntp->chn, ntp2->chn);
				lstrcat(ntp->chn, t2);
				if (t[0]=='#') lstrcpy(ntp->info.np.one, ""); // no per-word
				else lstrcpy(ntp->info.np.one, t);
				if (wh->wordFeature & PL) ntp->info.np.num = pl;
				else ntp->info.np.num = sing;
				free(ntp2);
				goto succ;
			}
		}
	}
	Restore();
	if (cat=Lex()){		// NPA-->N
		if (cat & N){		
			char *t, *t2;
			int i;

			wh = GetByCat(wh, N);
			ntp->c = npa;
			t = GetByClass(wh, want); // t point to per-word
//			if (!t) t = GetByClass(wh, ALL);
			if (!t) goto error;		// strict match class
			for(i=0; t[i]!=' '; i++);
			t[i] = '\0';
			t2 = t + i + 1;		// t2 point to chinese
			lstrcpy(ntp->chn, t2);
			if (t[0]=='#') lstrcpy(ntp->info.np.one, ""); // no per-word
			else lstrcpy(ntp->info.np.one, t);
			if (wh->wordFeature & PL) ntp->info.np.num = pl;
			else ntp->info.np.num = sing;
			goto succ;
		}
	}

error:
	free(ntp);
	Restore();
	Pop();
	return NULL;

succ:
	Pop();
	return ntp;
}


//////////////////////////////////////////////////////////////
//	adjp_parse()
//	parse a ADJP
//
ntsp_t DllExport WINAPI adjp_parse()
{
	WORD cat;
	nts_t *ntp, *ntp2;
	WORDHEAD *wh=(WORDHEAD *)gBuf;

	Backup();
	ntp = NewNT();
	if (ntp2=advp_parse()){		// ADJP-->ADVP ADJ
		ntp2->c = adjp;
		if (cat=Lex()){
			if (cat & ADJ){
				wh = GetByCat(wh, ADJ);
				lstrcat(ntp2->chn, GetOtherChn(wh));
//				lstrcat(ntp2->chn, "的");
				free(ntp);
				ntp = ntp2;
				goto succ;
			}
		}
	}
	Restore();
	if (cat=Lex()){		// ADJP-->ADJ
		if (cat & ADJ){		
			wh = GetByCat(wh, ADJ);
			ntp->c = adjp;
			lstrcpy(ntp->chn, GetOtherChn(wh));
//			lstrcat(ntp->chn, "的");
			goto succ;
		}
	}

//error:
	free(ntp);
	Restore();
	Pop();
	return NULL;

succ:
	Pop();
	return ntp;
}

//////////////////////////////////////////////////////////////
//	advp_parse()
//	parse a ADVP
//
ntsp_t DllExport WINAPI advp_parse()
{
	WORD cat;
	nts_t *ntp;
	WORDHEAD *wh=(WORDHEAD *)gBuf;

	Backup();
	ntp = NewNT();
	if (cat=Lex()){
		if (cat & ADV){		// ADVP-->ADV
			wh = GetByCat(wh, ADV);
			ntp->c = advp;
			lstrcpy(ntp->chn, GetOtherChn(wh));
//			lstrcat(ntp->chn, "地");
			Pop();
			Backup();
			if (cat = Lex()){
				if (cat & ADV){		// ADVP-->ADV ADV
					wh = GetByCat((WORDHEAD *)gBuf, ADV);
					lstrcat(ntp->chn, GetOtherChn(wh));
//					lstrcat(ntp->chn, "地");
					goto succ;
				}
			}
			Restore();
			goto succ;
		}
	}

//error:
	free(ntp);
	Restore();
	Pop();
	return NULL;

succ:
	Pop();
	return ntp;
}

//////////////////////////////////////////////////////////////
//	pp_parse()
//	parse a PP
//
ntsp_t DllExport WINAPI pp_parse()
{
	WORD cat;
	nts_t *ntp, *ntp2;
	WORDHEAD *wh=(WORDHEAD *)gBuf;
	NOUNPREPBODY *npb;
	NOUNPREPITEM *npi;
	int i;

	Backup();
	ntp = NewNT();
	if (cat=Lex()){		//	PP-->P NP
		if (cat & P){
//			wh = GetByCat(wh, P);
			ntp->c = pp;
			npb = (NOUNPREPBODY *)((char *)wh + wh->offsBody);
			npi = (NOUNPREPITEM *)((char *)wh + npb->offsItem);
			Pop();
			Backup();
			for(i=0; i<npb->numItem; i++){
				if (ntp2=np_parse(npi[i].classVal)) break;
				Restore();
			}
			if (!ntp2){ ntp2=np_parse(ALL); i=0; }
			if (ntp2){
				int tt=engPtr;

				Restore();
				lstrcpy(ntp->chn, (char *)wh+npi[i].offsChn);
				if (ntp->chn[0]=='$'){
					InsertNP(ntp->chn, ntp2->chn, '1');
					lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
				}
				else lstrcat(ntp->chn, ntp2->chn);
//				lstrcat(ntp->chn, ntp2->chn);
				free(ntp2);
				engPtr = tt;
				goto succ;
			}
		}
	}

//error:
	free(ntp);
	Restore();
	Pop();
	return NULL;

succ:
	Pop();
	return ntp;
}

/////----------------------------------------------------------------

