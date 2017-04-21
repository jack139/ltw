//********************************************************************************************
//    File Name    : 
//    Copyright 1996 by ITC/RD9.  All rights reserved.
//    Author       :  Shi Yongjun
//    Date         :  1996/11; 1997/1/21.
//    Description  :  
//    Side Effects :  
//    Class        :  
//    Function     :  
//    Notes        :
//    Update       :
//    Date            Name          Description
//	
//********************************************************************************************

#ifndef __DBGMSG_H__
#define __DBGMSG_H__


//向哪個窗口打印信息
#ifdef DBGMSG_ASST_WINDOW		//assistant window
#define DbgPrintf DbgAsstwndPrintf
#endif

#ifdef DBGMSG_MSGBOX_WINDOW		//message box window
#define  DbgPrintf DbgMsgboxPrintf
#endif

#ifdef DBGMSG_FILE_OUTPUT			//Output debug message to a file.
#define DbgPrintf DbgFilePrintf
#endif

#ifdef DBGMSG_MSVC_WINDOW			//MS-VC output window
#define DbgPrintf DbgMsVcPrintf
#endif

#ifndef DbgPrintf
#define DbgPrintf DbgMsVcPrintf
#endif




#define DoReturnVal(condition, return_val) \
	if (condition)\
	{						\
		V();\
		return return_val;	\
	}						

#define DoReturn(condition) \
	if (condition)\
	{						\
		V();\
		return;			\
	}

#define IsEmpty(p)	(p == NULL)
#define IsNull(p)	(p == NULL)
#define IsFalse(p)	(p == FALSE)
#define IsTrue(p)	(p == TRUE)

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

void DbgMsVcPrintf (char *fmt, ... );
void DbgMsgboxPrintf (char *fmt, ... );
void DbgAsstwndPrintf (char *fmt, ... );
void DbgFilePrintf (char *fmt, ... );
void DbgPopLastError(void);

void DbgWriteStrFile(LPSTR lpszData);

#ifdef __cplusplus
}
#endif //__cplusplus


#endif	//__DBGMSG_H__