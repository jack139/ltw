#include <windows.h>

/////// cat ///////////
#define AUX	0x0080
#define PRO	0x0040
#define ADJ	0x0020
#define ADV	0x0010
#define V	0x0008
#define N	0x0004
#define ART	0x0002
#define P	0x0001

#define BT	0x4000		// 
#define MUL	0x8000		// multi-sense

/////// feature ////////////
#define SING	0x0001
#define PL		0x0002
#define PRES	0x0004
#define PAST	0x0008
#define PP		0x0010
#define ING		0x0020
#define ROOT	0x0040
#define SELF	0x0080

///////  classfication ///////////////////////////////

#define NIL			0x00000000
#define ALL			0xffffffff
	#define INDIVIDUAL	0xfffffc00
		#define PHYSOBJ		0xfffc0000
			#define NON_LIVING	0xffc00000
				#define MACHINE		0x80000000
				#define MONEY		0x40000000
				#define BOOK		0x20000000
				#define CLOTHES		0x10000000
				#define PARTS		0x08000000
				#define INSTRMT		0x04000000
			#define LIVING		0x003c0000
				#define ANIMATE		0x00300000
					#define PERSON		0x00200000
					#define ANIMAL		0x00100000
				#define VEGETATIVE	0x000c0000
		#define ABSTRACTOBJ	0x0003fc00
			#define MENTALOBJ	0x00020000
			#define DISEASE		0x00010000
	#define ENVIROMENT	0x000003c0
		#define LOCATION	0x00000200
		#define SITUATION	0x00000100
		#define CONDITION	0x00000080
		#define TIME		0x00000040
	#define ACTION		0x0000003f
		#define PHYSACTION	0x00000038
			#define SOCIAL		0x00000020
			#define SPORT		0x00000010
		#define ABSTRCTACTION 0x00000007

////// type define ////////////////

typedef struct _wordhead{
		WORD wordCat;
		WORD wordFeature;
		WORD nextWord;
		BYTE offsBody;
		char wordName;
} WORDHEAD;			

typedef struct _verbbody{
		WORD offsVP;
		BYTE numVP;
		WORD offsPV;
		BYTE numPV;
		WORD offsP;
		BYTE numP;
		WORD strPV;		// can cut off
		WORD strP;		// can cut off
} VERBBODY;

typedef struct _nounprepbody{
		WORD offsItem;
		BYTE numItem;
//		char perWord;
} NOUNPREPBODY;

/*
typedef struct _prepbody{
		WORD offsItem;
		BYTE numItem;
} PREPBODY;
*/
typedef struct _probody{
		DWORD classVal;
		WORD offsChn;
} PROBODY;

typedef struct _plusbody{
		char rootWord;
} PLUSBODY;

typedef struct _verbitem{
		BYTE ruleNo;
		DWORD classVal[2];
		WORD offsChn;
} VERBITEM;

typedef struct _nounprepitem{
		DWORD classVal;
		WORD offsChn;
} NOUNPREPITEM;

		