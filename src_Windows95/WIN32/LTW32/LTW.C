#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <string.h>
#include "resource.h"
#include "ltwd.h"
  
#define BMP_WIDTH	210
#define BMP_HEIGHT	160
#define BMP_LEFT	20
#define BMP_TOP		70
 
HINSTANCE ghInst;
HWND hwBMP;
int nCaseSen = 0;
int refresh=0;
BOOL fAuto = TRUE;
HBITMAP hbmpChn,
		hbmpEng,
		hbmpTran,
		hbmpAccurate,
		hbmpInteli,
		hbmpTrace,
		hbmpFthz,
		hbmpQuit,
		hbmpLex;
DllImport TRANS trans;
char word_list_filename[100];


void BMP_OnPaint(HWND hwnd)
{
	HDC hDC;
	PAINTSTRUCT ps;
	RECT rc;
	int h;
	COLORREF col;

	hDC = BeginPaint(hwnd, &ps);
	SetRect(&rc, 3, 3, BMP_WIDTH-3 , BMP_HEIGHT-3);
	col = SetTextColor(hDC, RGB(0, 0, 255));
	h = DrawText(hDC, trans.eng, strlen(trans.eng),	&rc,
			DT_LEFT | DT_EXTERNALLEADING | DT_NOCLIP
			| DT_NOPREFIX | DT_WORDBREAK);
	SetTextColor(hDC, col);
	if (!DrawHZ(hDC, 3, h+10, trans.chn, BMP_WIDTH, RGB(0, 0, 255)))
		DisplayErr(FALSE);
	EndPaint(hwnd, &ps);
}


long CALLBACK BMPWndProc(HWND hwnd, UINT uMsg,
	WPARAM wParam, LPARAM lParam)
{
	switch (uMsg){
		HANDLE_MSG(hwnd, WM_PAINT, BMP_OnPaint);
		default:
			return (DefWindowProc(hwnd, uMsg, wParam, lParam));
	}
	return 0L;
}


BOOL Dlg_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
{
	WNDCLASS wc;
	
	if (!init() || !LexInit()){
		DisplayErr(FALSE);
		MessageBeep(MB_ICONQUESTION);
		MessageBox(NULL, 
			"Application Initialized Error.\n"
			"Little Translator(TM) has to be STOPed!",
			"Little Translator",
			MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
		EndDialog(hwnd, 0);
	}

	SetClassLong(hwnd, GCL_HICON, 
		(LONG)LoadIcon(ghInst, MAKEINTRESOURCE(ICO_LTW)));

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = BMPWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = ghInst;
	wc.hIcon = NULL;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "BMPWndClass";
	if (!RegisterClass(&wc)){
		MessageBeep(MB_ICONQUESTION);
		MessageBox(NULL, 
			"Application Initialized Error.\n"
			"Little Translator(TM) has to be STOPed!",
			"Little Translator",
			MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
		EndDialog(hwnd, 0);
	}
	hwBMP = CreateWindowEx(WS_EX_CLIENTEDGE,
		"BMPWndClass",
		NULL,
		WS_CHILD | WS_VISIBLE | WS_BORDER,
		BMP_LEFT, BMP_TOP, BMP_WIDTH, BMP_HEIGHT,
		hwnd,
		NULL,
		ghInst,
		NULL);

//	CreateMH();
	SetDlgHandle(hwnd, hwBMP);
//	CreateMH();

	hbmpChn = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_CHN));
	hbmpEng = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_ENG));
//	hbmpTran = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_TRAN));
	hbmpAccurate = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_ACCURATE));
	hbmpInteli = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_INTELI));
	hbmpTrace = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_TRACE));
	hbmpFthz = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_FTHZ));
//	hbmpQuit = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_QUIT));
	hbmpLex = LoadBitmap(ghInst, MAKEINTRESOURCE(BMP_LEX));

	if (fAuto) CheckDlgButton(hwnd, IDC_AUTO, 1);

	GetPrivateProfileString("SET", "WORD_LIST",
		"WORD_LST.TXT", word_list_filename, 30, ".\\CONFIG.INI"); 

	return TRUE;
}


void Dlg_OnSysCommand(HWND hwnd, UINT cmd, int x, int y)
{
	if (cmd == SC_CLOSE) EndDialog(hwnd, 0);
}
    	

void Dlg_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify)
{
	char t[0x40];
    static char te[0x40]={0};
	int i;
    HWND hTemp;

	switch (id){
		case IDC_TRAN:
			if (codeNotify == BN_CLICKED){
				GetDlgItemText(hwnd, IDC_ENGLISH, t, 0x40);
                if (!strlen(t)) break;
                strcpy(trans.eng, t);

				// search here
				if (!translate(&trans)){
					DisplayErr(TRUE);
					break;
				}

				if (strcmp(te, trans.eng)!=0){
					strcpy(te, trans.eng);

					refresh = 1;
					if (nCaseSen){
						if (stricmp(t, trans.eng) != 0){
							strcpy(trans.chn, "本词典无此词条！");
							trans.eng[0] = '\0';
							trans.pron[0] = '\0';
							refresh = 0;
						}
	                }

					InvalidateRect(hwBMP, NULL, TRUE);
					UpdateWindow(hwBMP);
                }

				if (hwndCtl){
					SetDlgItemText(hwnd, IDC_ENGLISH, trans.eng);
					hTemp = GetDlgItem(hwnd, IDC_ENGLISH);
					SetFocus(hTemp);
					Edit_SetSel(hTemp, 0, -1);
				}
			}
			break;

		case IDC_ENGLISH:
			switch (codeNotify){
				case EN_CHANGE:
					if (!fAuto) break;
					SendMessage(hwnd, WM_COMMAND, IDC_TRAN,
						MAKELONG(0, BN_CLICKED));
					break;

				case EN_SETFOCUS:
					Edit_SetSel(hwndCtl, 0, -1);
					break;
			}
            break;

		case IDC_TRACE:
			if (codeNotify == BN_CLICKED){
				if (IsTracing()){ DestoryMH(); StopTrace(); }
				else { CreateMH(); SetTrace(); }
			}
			break;

		case IDC_FTHZ:
			if (codeNotify == BN_CLICKED)
				ChangeHZStyle();
			break;
		case IDC_LEX:
			if (codeNotify == BN_CLICKED)
				ChangeLexStatus();
			break;


		case IDC_CASE:
			nCaseSen = !nCaseSen;
			break;

		case IDC_AUTO:
			if (fAuto == TRUE) fAuto = FALSE;
			else fAuto = TRUE;
            break;

		case IDC_LIST:
			switch(codeNotify){
				case LBN_SELCHANGE:
					i = ListBox_GetCurSel(hwndCtl);
					ListBox_GetText(hwndCtl, i, t);
					SetDlgItemText(hwnd, IDC_ENGLISH, t);
					SendMessage(hwnd, WM_COMMAND, IDC_TRAN,
						MAKELONG(0, BN_CLICKED));
                	break;
			}
			break;

		case IDC_EXIT:
			if (codeNotify == BN_CLICKED) EndDialog(hwnd, 0);
			break;

		case IDC_ABOUT:
			if (codeNotify == BN_CLICKED){
				MessageBox(hwnd,
					"Little Translator(TM) for Windows 95/98\n"
					"Release 0.99\n\n"
					"Copyright (C) by Guan Tao, 1996-1998",
					"About me ...",
					MB_OK | MB_ICONINFORMATION | MB_APPLMODAL);
				if (hwndCtl){
					hTemp = GetDlgItem(hwnd, IDC_ENGLISH);
					SetFocus(hTemp);
					Edit_SetSel(hTemp, 0, -1);
				}
			}
			break;

		case IDC_SAVE:
			if (codeNotify == BN_CLICKED){
				FILE *fp;

				fp = fopen(word_list_filename, "a");
				if (!fp){
					MessageBox(hwnd,
						"Cannot open word list file",
						"Open file fail",
						MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
				}
				else{
					if (refresh) fprintf(fp, "%-25s%s\n", trans.eng, trans.chn);
					fclose(fp);
					refresh = 0;
				}
				if (hwndCtl){
					SetDlgItemText(hwnd, IDC_ENGLISH, trans.eng);
					hTemp = GetDlgItem(hwnd, IDC_ENGLISH);
					SetFocus(hTemp);
					Edit_SetSel(hTemp, 0, -1);
				}
			}
			break;
	}
}

void Dlg_OnDestroy(HWND hwnd)
{
	if (IsTracing()) DestoryMH();
	DestroyWindow(hwBMP);
	UnregisterClass("BMPWndClass", ghInst);
}

void Dlg_OnPaint(HWND hwnd)
{
	HDC hDC, hdcMem;
	PAINTSTRUCT ps;
	HBITMAP hOldBit;
	COLORREF col;
	
	hDC = BeginPaint(hwnd, &ps);

	hdcMem = CreateCompatibleDC(hDC);
	col = SetBkColor(hDC, RGB(192, 192, 192));

	hOldBit = SelectObject(hdcMem, hbmpChn);
	BitBlt(hDC, 15, 45, 71, 18, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hbmpEng);
	BitBlt(hDC, 15, 10, 71, 18, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hbmpTrace);
	BitBlt(hDC, 60, 240, 65, 18, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hbmpFthz);
	BitBlt(hDC, 60, 270, 65, 18, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hbmpAccurate);
	BitBlt(hDC, 170, 240, 65, 18, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hbmpInteli);
	BitBlt(hDC, 170, 270, 65, 18, hdcMem, 0, 0, SRCCOPY);
	SelectObject(hdcMem, hbmpLex);
	BitBlt(hDC, 60, 300, 129, 17, hdcMem, 0, 0, SRCCOPY);

	SetBkColor(hDC, col);
	SelectObject(hdcMem, hOldBit);
	DeleteDC(hdcMem);

	EndPaint(hwnd, &ps);
}


BOOL CALLBACK DlgProc(HWND hwnd, UINT uMsg,	WPARAM wParam, LPARAM lParam)
{
	BOOL fProcessed = TRUE;

	switch (uMsg){
		HANDLE_MSG(hwnd, WM_INITDIALOG, Dlg_OnInitDialog);
		HANDLE_MSG(hwnd, WM_PAINT, Dlg_OnPaint);
		HANDLE_MSG(hwnd, WM_COMMAND, Dlg_OnCommand);
		HANDLE_MSG(hwnd, WM_SYSCOMMAND, Dlg_OnSysCommand);
		HANDLE_MSG(hwnd, WM_DESTROY, Dlg_OnDestroy);
		default:
			fProcessed = FALSE;
	}
	return (fProcessed);
}



int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpszCmdLine, int nCmdShow)
{
	FARPROC lpfnDlgProc;

	if (FindWindow(NULL,"Little Translator")){
		MessageBeep(MB_ICONQUESTION);
		MessageBox(NULL, 
			"Another instance of Little Translator(TM)\n"
			"is already running.",
			"Little Translator",
			MB_OK | MB_ICONEXCLAMATION | MB_SYSTEMMODAL);
		return (0);
	}
	ghInst = hInstance;

	lpfnDlgProc = MakeProcInstance(DlgProc, hInstance);
	DialogBox(hInstance, MAKEINTRESOURCE(DLG_LTW), NULL, lpfnDlgProc);
	FreeProcInstance(lpfnDlgProc);
	LexExit();

	return (0);
}

