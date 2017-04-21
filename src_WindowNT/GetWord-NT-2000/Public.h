// public.h
#ifndef _INC_PUBLIC
#define _INC_PUBLIC

#define DLLEXPORT __declspec(dllexport)

#define MUTEXNAME "NobleHand"

typedef struct _tagWordRect
{
        long left;
        long right;
        long top;
        long bottom;
}WORDRECT, *LPWORDRECT;

#define BUFFERLENGTH            4096*4
#define MAXNUM                  1024
#define WORDMAXLEN              256

#define MEMDC_MAXNUM            256*4
#define MEMDC_WORDLEN           32

#define MSG_HASSTRINGNAME       "BL_HASSTRING"

#define BL_OK                   0
#define BL_BUFFERSMALL          1

/*
#define GETWORD_D_ENABLE		1000
#define GETWORD_TW_ENABLE		1001
#define GETWORD_DISABLE			1002
#define GETPHRASE_ENABLE		1003
#define GETPHRASE_DISABLE		1004
*/

#define _DICTING_

#ifdef _DICTING_
#define GETWORD_D_ENABLE		1000
#define GETWORD_TW_ENABLE		1001
#define GETWORD_DISABLE			1002
#ifndef GETPHRASE_ENABLE
#define GETPHRASE_ENABLE		1003
#define GETPHRASE_DISABLE		1004
#endif //GETPHRASE_ENABLE
#ifndef GETWORD_D_TYPING_ENABLE
#define GETWORD_D_TYPING_ENABLE		1005
#define GETWORD_D_TYPING_DISABLE	1006
#endif //GETPHRASE_ENABLE

////////////////////////////////////////////////////////////////////////////////
#else
#define GETWORD_ENABLE			1001
#endif

#define CHINESE_TAIWAN          0x0404  //Chinese (Taiwan)
#define CHINESE_PRC             0x0804  //Chinese (PRC)
#define ENGLISH_UNITED_STATES   0x0409  //English (United States)
#define PROCESS_DEFAULT         0x0400  //Process Default Language

#define CHAR_TYPE_ASCII         0
#define CHAR_TYPE_HZ            1
#define CHAR_TYPE_OTHER         2

#define MAX_WORDS_IN_PHRASE     3
#define MIN_WORDS_IN_PHRASE    -1

//#define GETWORDEND_EVENT_NAME __TEXT("NH_GetWordEnd")	//added by ZHHN 1999.12.30

#endif // _INC_PUBLIC