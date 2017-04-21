#ifndef __GETWORDS__
#define __GETWORDS__

#include <windows.h>

#define NHD_MAX_TEXTLEN		1024
#define GWMSG_GETWORDOK		"BL_HASSTRING"
#define GETWORD_ENABLE		1000
#define GETWORD_DISABLE		1002
#define NHD_WIN_INITPOSX	-1
#define NHD_WIN_INITPOSY	-1
#define NHD_FLYWIN_WIDTH	1
#define NHD_FLYWIN_HEIGHT	1
#define NHD_CLASSNAME_LEN	64
#define NHD_GETWORD_TIMER	2
#define NHD_GW_WAITING_TIME	200   //get word waiting time;
#define NHD_WM_GETWORD_OK	WM_USER + 1011

HWND NHD_InitGetWords(HINSTANCE hInst, HWND hwnd);
BOOL NHD_ExitGetWords(void);
void NHD_BeginGetWord(POINT *g_ptMousePos);
BOOL NHD_CopyWordsTo(char *szBuffer, int nBufferSize);

#endif //__GETWORDS__     