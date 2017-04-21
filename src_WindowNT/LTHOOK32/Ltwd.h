#ifndef __LTWDH__
#define __LTWDH__

#define DllExport	__declspec(dllexport)
#define DllImport	__declspec(dllimport)

typedef struct _trans{
			char eng[0x40];
			char chn[0x80];
			char pron[0x20];
		} TRANS, *PTRANS, far *FPTRANS;


int DllExport WINAPI init(void);
int DllExport WINAPI translate(FPTRANS);
void DllExport WINAPI DisplayErr(BOOL);
UINT DllExport WINAPI DrawHZ(HDC, int, int, BYTE FAR *s, int, COLORREF);
BOOL DllExport WINAPI IsTracing(void);
BOOL DllExport WINAPI SetTrace(HINSTANCE);
void DllExport WINAPI StopTrace(void);
long DllExport CALLBACK MHWndProc(HWND, UINT,	WPARAM, LPARAM);
void DllExport WINAPI SetMHWndHandle(HWND);
void DllExport WINAPI ChangeHZStyle(void);
void DllExport WINAPI SetDlgHandle(HWND, HWND);
int DllExport WINAPI CreateMH(void);
void DllExport WINAPI DestoryMH(void);

#endif