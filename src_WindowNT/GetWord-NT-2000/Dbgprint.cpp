//********************************************************************************************
//    File Name    :  DbgMsg.CPP
//    Copyright 1996 by ITC/RD9.  All rights reserved.
//    Author       :  Shi Yongjun
//    Date         :  1996,11
//    Description  : 
//    Side Effects : 
//    Class        : 
//    Function     :  
//    Notes        :
//    Update       :
//    Date            Name          Description
//********************************************************************************************

#include "stdafx.h"
#include "DbgPrint.h"

#define DBG_STR_MAX_LEN 256



#define OUTPUT_FILE_NAME "C:\\OUTPUT.LOG"

HWND g_hwndDbgEdit = NULL;

void DbgMsVcPrintf (char *fmt, ... )
{
  va_list argptr;				/* Argument list pointer	*/
  char str[DBG_STR_MAX_LEN];	/* Buffer to build sting into	*/

  va_start (argptr, fmt);		/* Initialize va_ functions	*/
  wvsprintf (str, fmt, argptr);	/* prints string to buffer	*/
  OutputDebugString(str);
  va_end (argptr);				/* Close va_ functions		*/
}

void DbgMsgboxPrintf (char *fmt, ... )
{
  va_list argptr;				/* Argument list pointer	*/
  char str[DBG_STR_MAX_LEN];	/* Buffer to build sting into	*/

  va_start (argptr, fmt);		/* Initialize va_ functions	*/
  wvsprintf (str, fmt, argptr);	/* prints string to buffer	*/
  MessageBox (NULL, str, "Debug Message(DbgMsgboxPrintf)", 
			MB_ICONINFORMATION | MB_OK);
  va_end (argptr);				/* Close va_ functions		*/
}

void DbgFilePrintf (char *fmt, ... )
{
  FILE *output;    
  va_list argptr;				/* Argument list pointer	*/
  char str[DBG_STR_MAX_LEN];	/* Buffer to build sting into	*/

  va_start (argptr, fmt);		/* Initialize va_ functions	*/
  wvsprintf (str, fmt, argptr);	/* prints string to buffer	*/
  va_end (argptr);				/* Close va_ functions		*/

  output = fopen(OUTPUT_FILE_NAME, "a");
  if( output==NULL )    return;
  fprintf(output, "%s", str);
  fclose(output);
}

void DbgAsstwndPrintf (char *fmt, ... )
{
  va_list argptr;				/* Argument list pointer	*/
  char str[DBG_STR_MAX_LEN];	/* Buffer to build sting into	*/

  va_start (argptr, fmt);		/* Initialize va_ functions	*/
  wvsprintf (str, fmt, argptr);	/* prints string to buffer	*/
  if (g_hwndDbgEdit != NULL) 
  {
	  SetWindowText(g_hwndDbgEdit, str);
  }
  va_end (argptr);				/* Close va_ functions		*/
}

void DbgPopLastError(void)
{
	LPVOID lpMsgBuf;
	FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | 
					FORMAT_MESSAGE_FROM_SYSTEM,
					NULL,
					GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
					(LPTSTR) &lpMsgBuf,
					0,
					NULL );
	//DbgPrintf (lpMsgBuf);
	MessageBox (NULL, (LPCSTR)lpMsgBuf, "Debug Message(DbgPopLastError)", 
					MB_ICONINFORMATION | MB_OK);
	LocalFree( lpMsgBuf );	
}



void DbgWriteStrFile(LPSTR lpszData)
{
	FILE *output;    

	output = fopen(OUTPUT_FILE_NAME, "a");
	if( output == NULL )   
		return;
	while (*lpszData)
	{
		fwrite (lpszData++, sizeof(char), 1, output);
	}
	fclose(output);
}