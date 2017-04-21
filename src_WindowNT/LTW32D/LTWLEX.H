#ifndef __LTWLEX__
#define __LTWLEX__

#define DllExport	__declspec(dllexport)
#define DllImport	__declspec(dllimport)

typedef enum _cat{ np, vp, pp, adjp, advp, npa, vp_ing, vp_root, to } cat_t;
typedef enum _num{ sing, pl } num_t;
//typedef enum _the{ the, aan} the_t;
//typedef enum _tense{ root, pres, past, pp, ing } tense_t;
//typedef enum _bool{ true, false } bool_t;

typedef struct _nps{
		num_t num;
//		the_t a;
		char one[11];
//		DWORD classVal;
	} nps_t;

typedef struct _vps{
		WORD tense;
		char prep[20];
	} vps_t;

typedef struct _nts{
		cat_t c;
		union { 
			nps_t np;
			vps_t vp;
		} info;
		char chn[100];
	} nts_t, *ntsp_t;


int DllExport WINAPI LexInit(void);
void DllExport WINAPI LexExit(void);
WORD DoSearch(char *);
int strnicmp_tail(LPSTR , LPSTR , int);
WORD LexSearch(char *);
WORD Lex(void);
int Backup(void);
int Restore(void);
int Pop(void);
nts_t *NewNT(void);
WORDHEAD *GetByCat(WORDHEAD *, WORD);
char *GetByClass(WORDHEAD *, DWORD);
char *GetOtherChn(WORDHEAD *);
int InsertNP(LPSTR, LPSTR, char);

ntsp_t DllExport WINAPI np_parse(DWORD);
ntsp_t DllExport WINAPI npa_parse(DWORD);
ntsp_t DllExport WINAPI adjp_parse(void);
ntsp_t DllExport WINAPI advp_parse(void);
ntsp_t DllExport WINAPI pp_parse(void);

ntsp_t DllExport WINAPI vp_parse(void);
nts_t *vp1_parse(int);
nts_t *vp2_parse(int);
nts_t *vp3_parse(int);
nts_t *vp4_parse(int);
nts_t *vp5_parse(int);
nts_t *vp6_parse(int);
nts_t *vp7_parse(int);
nts_t *vp8_parse(int);
nts_t *vp9_parse(int);
nts_t *vp10_parse(int);
nts_t *vp11_parse(int);
nts_t *vp12_parse(int);
nts_t *vp13_parse(int);
nts_t *vp14_parse(int);
nts_t *vp15_parse(int);
nts_t *vp16_parse(int);
nts_t *vp17_parse(int);

#endif
