#include <stdlib.h>
#include <stdio.h>
#include <windows.h>
#include "text.h"
#include "ltwlex.h"

typedef nts_t *(*vpproc_t)(int);

extern LPSTR gBuf;
extern char engStr[0x40];
extern int engPtr;
vpproc_t vpn[17]={
			vp1_parse, vp2_parse, vp3_parse, vp4_parse,
			vp5_parse, vp6_parse, vp7_parse, vp8_parse,
			vp9_parse, vp10_parse, vp11_parse, vp12_parse,
			vp13_parse, vp14_parse, vp15_parse, vp16_parse,
			vp17_parse };

ntsp_t DllExport WINAPI vp_parse()
{
	WORD cat;
	nts_t *ntp;
	WORDHEAD *wh=(WORDHEAD *)gBuf;
	VERBBODY *vb;
	VERBITEM *vi;
	int i;
	
	Backup();
	if (cat=Lex()){
		if (cat & V){
			wh = GetByCat(wh, V);
			vb = (VERBBODY *)((char *)wh + wh->offsBody);
			if (vb->numPV){
				vi = (VERBITEM *)((char *)wh + vb->offsPV);
				for(i=0; i<vb->numPV; i++)
					if (ntp=(*vpn[vi[i].ruleNo-1])(i)) break;
				if (ntp) goto succ;
			}
			if (vb->numP){
				vi = (VERBITEM *)((char *)wh + vb->offsP);
				for(i=0; i<vb->numP; i++)
					if (ntp=(*vpn[vi[i].ruleNo-1])(i)) break;
				if (ntp) goto succ;
			}
			if (vb->numVP){
				vi = (VERBITEM *)((char *)wh + vb->offsVP);
				for(i=0; i<vb->numVP; i++)
					if (ntp=(*vpn[vi[i].ruleNo-1])(i)) break;
				if (ntp) goto succ;
			}
		}
	}

//error:
	Restore();
	Pop();
	return NULL;

succ:
	Pop();
	return ntp;
}

////////////////////////////////////////////////////////////////////////

nts_t *np_parse_vp(DWORD want)
{
	nts_t *ntp;

//	if (!(ntp = np_parse(want))) ntp = np_parse(ALL);
//	return ntp;		//  if not match wanted class, return ALL

	return (ntp = np_parse(want));  // this is for strict match class
}

nts_t *vp9_parse(int n)		// VP-->V
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp;

//	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
//	Pop();
	return ntp;
//	return NULL;
}

nts_t *vp5_parse(int n)		// VP-->V NP
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
	if (ntp2=np_parse_vp(vi[n].classVal[0])){
		if (ntp->chn[0]=='$'){
			lstrcpy(ntp->chn, ntp->chn+1);
			InsertNP(ntp->chn, ntp2->chn, '1');
		}
		else lstrcat(ntp->chn, ntp2->chn);
		free(ntp2);
		goto succ;
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

nts_t *vp1_parse(int n)		// VP-->V NP NP
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;
	DWORD class1, class2;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
	class1 = vi[n].classVal[0];
	class2 = vi[n].classVal[1];
	if (ntp2=np_parse_vp(class1)){	// NP1
		if (ntp->chn[0]=='$') InsertNP(ntp->chn, ntp2->chn, '1');
		else lstrcat(ntp->chn, ntp2->chn);
		free(ntp2);
		if (ntp2=np_parse_vp(class2)){	// NP2
			if (ntp->chn[0]=='$'){
				InsertNP(ntp->chn, ntp2->chn, '2');
				lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
			}
			else lstrcat(ntp->chn, ntp2->chn);
			free(ntp2);
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

nts_t *vp6_parse(int n)		// VP-->V TO VP
{
	WORD cat;
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
	if (cat=Lex()){		// if is TO
		wh = (WORDHEAD *)gBuf;
		if (!lstrcmpi(&wh->wordName, "to")){
			if (ntp2=vp_parse()){	// VP
				if (ntp->chn[0]=='$'){
					InsertNP(ntp->chn, ntp2->chn, '1');
					lstrcpy(ntp->chn, ntp->chn+1);
				}
				else lstrcat(ntp->chn, ntp2->chn);
				free(ntp2);
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

nts_t *vp2_parse(int n)		// VP-->V NP TO VP
{
	WORD cat;
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
	if (ntp2=np_parse_vp(vi[n].classVal[0])){	// if is NP
		if (ntp->chn[0]=='$') InsertNP(ntp->chn, ntp2->chn, '1');
		else lstrcat(ntp->chn, ntp2->chn);
		free(ntp2);
		if (cat=Lex()){		// if is TO
			wh = (WORDHEAD *)gBuf;
			if (!lstrcmpi(&wh->wordName, "to")){
				if (ntp2=vp_parse()){	// VP
					if (ntp->chn[0]=='$'){
						InsertNP(ntp->chn, ntp2->chn, '2');
						lstrcpy(ntp->chn, ntp->chn+1);
					}
					else lstrcat(ntp->chn, ntp2->chn);
					free(ntp2);
					goto succ;
				}
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

nts_t *vp7_parse(int n)		// VP-->V VP-ING
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
	if (ntp2=vp_parse()){	// VP
		if (ntp2->info.vp.tense & ING){		// if is ING
			if (ntp->chn[0]=='$'){
				InsertNP(ntp->chn, ntp2->chn, '1');
				lstrcpy(ntp->chn, ntp->chn+1);
			}
			else lstrcat(ntp->chn, ntp2->chn);
			free(ntp2);
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

nts_t *vp8_parse(int n)		// VP-->ADJP
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
	if (ntp2=adjp_parse()){	// ADJP
		if (ntp->chn[0]=='$'){
			InsertNP(ntp->chn, ntp2->chn, '1');
			lstrcpy(ntp->chn, ntp->chn+1);
		}
		else lstrcat(ntp->chn, ntp2->chn);
		free(ntp2);
		goto succ;
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

nts_t *vp3_parse(int n)		// VP-->V NP ADJP
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
	if (ntp2=np_parse_vp(vi[n].classVal[0])){	// NP
		if (ntp->chn[0]=='$') InsertNP(ntp->chn, ntp2->chn, '1');
		else lstrcat(ntp->chn, ntp2->chn);
		free(ntp2);
		if (ntp2=adjp_parse()){	// ADJP
			if (ntp->chn[0]=='$'){
				InsertNP(ntp->chn, ntp2->chn, '2');
				lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
			}
			else lstrcat(ntp->chn, ntp2->chn);
			free(ntp2);
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

nts_t *vp4_parse(int n)		// VP-->V NP VP-ROOT
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsVP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn);
	if (ntp2=np_parse_vp(vi[n].classVal[0])){	// NP
		if (ntp->chn[0]=='$') InsertNP(ntp->chn, ntp2->chn, '1');
		else lstrcat(ntp->chn, ntp2->chn);
		free(ntp2);
		if (ntp2=vp_parse()){	// VP
//			WORD x2=(ROOT|PRES|PAST|PP|ING),
//				x=ntp->info.vp.tense&(ROOT|PRES|PAST|PP|ING);

			if ((ntp2->info.vp.tense&(ROOT|PRES|PAST|PP|ING))==ROOT){ // if is ROOT
				if (ntp->chn[0]=='$'){
					InsertNP(ntp->chn, ntp2->chn, '2');
					lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
				}
				else lstrcat(ntp->chn, ntp2->chn);
				free(ntp2);
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

////--------------- P -------------------

nts_t *vp12_parse(int n)	// VP-->V P
{
	WORD cat;
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	lstrcpy(ntp->info.vp.prep, (char *)wh+vi[n].offsChn);	// copy prep
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn+lstrlen(ntp->info.vp.prep)+1); // copy chinese
	if (cat=Lex()){		// 
		if (cat & P){
			wh = GetByCat((WORDHEAD *)gBuf, P);
			if (!lstrcmpi(ntp->info.vp.prep, &wh->wordName))	goto succ;
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

nts_t *vp10_parse(int n)	// VP-->V P NP
{
	WORD cat;
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;
	DWORD class1;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	lstrcpy(ntp->info.vp.prep, (char *)wh+vi[n].offsChn);	// copy prep
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn+lstrlen(ntp->info.vp.prep)+1); // copy chinese
	class1 = vi[n].classVal[0];
	if (cat=Lex()){		// 
		if (cat & P){		// P
			wh = GetByCat((WORDHEAD *)gBuf, P);
			if (!lstrcmpi(ntp->info.vp.prep, &wh->wordName)){
				if (ntp2=np_parse_vp(class1)){		// NP
					if (ntp->chn[0]=='$'){
						InsertNP(ntp->chn, ntp2->chn, '1');
						lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
					}
					else lstrcat(ntp->chn, ntp2->chn);
					free(ntp2);
					goto succ;
				}
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

nts_t *vp13_parse(int n)	// VP-->P NP P NP
{
	WORD cat;
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;
	DWORD class2;
	
	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	lstrcpy(ntp->info.vp.prep, (char *)wh+vi[n].offsChn);	// copy prep
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn+lstrlen(ntp->info.vp.prep)+1); // copy chinese
	class2 = vi[n].classVal[1];
	if (ntp2=np_parse_vp(vi[n].classVal[0])){		// NP1
		if (ntp->chn[0]=='$') InsertNP(ntp->chn, ntp2->chn, '1');
		else lstrcat(ntp->chn, ntp2->chn);
		free(ntp2);
		if (cat=Lex()){		// 
			if (cat & P){		// P
				wh = GetByCat((WORDHEAD *)gBuf, P);
				if (!lstrcmpi(ntp->info.vp.prep, &wh->wordName)){
					if (ntp2=np_parse_vp(class2)){		// NP2
						if (ntp->chn[0]=='$'){
							InsertNP(ntp->chn, ntp2->chn, '2');
							lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
						}	
						else lstrcat(ntp->chn, ntp2->chn);
						free(ntp2);
						goto succ;
					}
				}
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

nts_t *vp14_parse(int n)	// VP-->V NP P
{
	WORD cat;
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;
	
	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	lstrcpy(ntp->info.vp.prep, (char *)wh+vi[n].offsChn);	// copy prep
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn+lstrlen(ntp->info.vp.prep)+1); // copy chinese
	if (ntp2=np_parse_vp(vi[n].classVal[0])){		// NP
		if (ntp->chn[0]=='$'){
			InsertNP(ntp->chn, ntp2->chn, '1');
			lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
		}
		else lstrcat(ntp->chn, ntp2->chn);
		free(ntp2);
		if (cat=Lex()){		// 
			if (cat & P){		// P
				wh = GetByCat((WORDHEAD *)gBuf, P);
				if (!lstrcmpi(ntp->info.vp.prep, &wh->wordName)) goto succ;
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

nts_t *vp11_parse(int n)	// VP-->V P VP-ING
{
	WORD cat;
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;
	
	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsP);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	lstrcpy(ntp->info.vp.prep, (char *)wh+vi[n].offsChn);	// copy prep
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn+lstrlen(ntp->info.vp.prep)+1); // copy chinese
	if (cat=Lex()){		// 
		if (cat & P){		// P
			wh = GetByCat((WORDHEAD *)gBuf, P);
			if (!lstrcmpi(ntp->info.vp.prep, &wh->wordName)){
				if (ntp2=vp_parse()){		// VP
					if (ntp2->info.vp.tense & ING){	// if is ING
						if (ntp->chn[0]=='$'){
							InsertNP(ntp->chn, ntp2->chn, '1');
							lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
						}
						else lstrcat(ntp->chn, ntp2->chn);
						free(ntp2);
						goto succ;
					}
				}
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

////---------------------- PV -------------------------

nts_t *vp17_parse(int n)	// VP-->PV
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp;
	int l, i;
	

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsPV);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	lstrcpy(ntp->info.vp.prep, (char *)wh+vi[n].offsChn);	// copy idom
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn+lstrlen(ntp->info.vp.prep)+1); // copy chinese
	l = lstrlen(ntp->info.vp.prep);
	for (i=engPtr; engStr[i]==' '; i++);
	engPtr = i;
	if (!strnicmp(ntp->info.vp.prep, engStr+engPtr, l)){		// match idom
		engPtr += l;
		goto succ;
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

nts_t *vp15_parse(int n)	// VP-->PV NP
{
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;
	int l, i;
	

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsPV);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	lstrcpy(ntp->info.vp.prep, (char *)wh+vi[n].offsChn);	// copy idom
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn+lstrlen(ntp->info.vp.prep)+1); // copy chinese
	l = lstrlen(ntp->info.vp.prep);
	for (i=engPtr; engStr[i]==' '; i++);
	engPtr = i;
	if (!strnicmp(ntp->info.vp.prep, engStr+engPtr, l)){		// match idom
		engPtr += l;
		if (ntp2=np_parse_vp(vi[n].classVal[0])){	// NP
			if (ntp->chn[0]=='$'){
				InsertNP(ntp->chn, ntp2->chn, '1');
				lstrcpy(ntp->chn, ntp->chn+1);	// cut '$'
			}
			else lstrcat(ntp->chn, ntp2->chn);
			free(ntp2);
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

nts_t *vp16_parse(int n)	// VP-->PV TO VP
{
	WORD cat;
	WORDHEAD *wh;
	VERBBODY *vb;
	VERBITEM *vi;
	nts_t *ntp, *ntp2;
	int l, i;

	Backup();	
	ntp = NewNT();	// verb have read in gBuf
	wh = GetByCat((WORDHEAD *)gBuf, V);
	vb = (VERBBODY *)((char *)wh + wh->offsBody);
	vi = (VERBITEM *)((char *)wh + vb->offsPV);
	ntp->c = vp;
	ntp->info.vp.tense = wh->wordFeature;
	lstrcpy(ntp->info.vp.prep, (char *)wh+vi[n].offsChn);	// copy idom
	ntp->chn[0]='\0';
//	if (ntp->info.vp.tense & ING) lstrcpy(ntp->chn, "正在");
	lstrcat(ntp->chn, (char *)wh+vi[n].offsChn+lstrlen(ntp->info.vp.prep)+1); // copy chinese
	l = lstrlen(ntp->info.vp.prep);
	for (i=engPtr; engStr[i]==' '; i++);
	engPtr = i;
	if (!strnicmp(ntp->info.vp.prep, engStr+engPtr, l)){		// match idom
		engPtr += l;
		if (cat=Lex()){		// if is TO
			wh = (WORDHEAD *)gBuf;
			if (!lstrcmpi(&wh->wordName, "to")){
				if (ntp2=vp_parse()){	// VP
					if (ntp->chn[0]=='$'){
						InsertNP(ntp->chn, ntp2->chn, '1');
						lstrcpy(ntp->chn, ntp->chn+1);
					}
					else lstrcat(ntp->chn, ntp2->chn);
					free(ntp2);
					goto succ;
				}
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

