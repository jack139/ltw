# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

!IF "$(CFG)" == ""
CFG=ltw32 - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to ltw32 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ltw32 - Win32 Release" && "$(CFG)" != "ltw32 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "ltw32.mak" CFG="ltw32 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ltw32 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "ltw32 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 
################################################################################
# Begin Project
# PROP Target_Last_Scanned "ltw32 - Win32 Debug"
MTL=mktyplib.exe
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ltw32 - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release

ALL : "$(OUTDIR)\ltw32.exe"

CLEAN : 
	-@erase ".\Release\ltw32.exe"
	-@erase ".\Release\Ltw.obj"
	-@erase ".\Release\ltw.res"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/ltw32.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x804 /d "NDEBUG"
RSC_PROJ=/l 0x804 /fo"$(INTDIR)/ltw.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ltw32.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib c:\user\gt\doc\win32\ltw32d\release\ltw32d.lib c:\user\gt\doc\win32\lthook32\release\lthook32.lib /nologo /subsystem:windows /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib\
 c:\user\gt\doc\win32\ltw32d\release\ltw32d.lib\
 c:\user\gt\doc\win32\lthook32\release\lthook32.lib /nologo /subsystem:windows\
 /incremental:no /pdb:"$(OUTDIR)/ltw32.pdb" /machine:I386\
 /out:"$(OUTDIR)/ltw32.exe" 
LINK32_OBJS= \
	"$(INTDIR)/Ltw.obj" \
	"$(INTDIR)/ltw.res"

"$(OUTDIR)\ltw32.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ltw32 - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug

ALL : "$(OUTDIR)\ltw32.exe"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\ltw32.exe"
	-@erase ".\Debug\Ltw.obj"
	-@erase ".\Debug\ltw.res"
	-@erase ".\Debug\ltw32.ilk"
	-@erase ".\Debug\ltw32.pdb"
	-@erase ".\Debug\ltw32.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/ltw32.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
RSC_PROJ=/l 0x804 /fo"$(INTDIR)/ltw.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/ltw32.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib c:\user\gt\doc\win32\ltw32d\debug\ltw32d.lib c:\user\gt\doc\win32\lthook32\debug\lthook32.lib /nologo /subsystem:windows /map /debug /machine:I386
# SUBTRACT LINK32 /profile /pdb:none
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib\
 c:\user\gt\doc\win32\ltw32d\debug\ltw32d.lib\
 c:\user\gt\doc\win32\lthook32\debug\lthook32.lib /nologo /subsystem:windows\
 /incremental:yes /pdb:"$(OUTDIR)/ltw32.pdb" /map:"$(INTDIR)/ltw32.map" /debug\
 /machine:I386 /out:"$(OUTDIR)/ltw32.exe" 
LINK32_OBJS= \
	"$(INTDIR)/Ltw.obj" \
	"$(INTDIR)/ltw.res"

"$(OUTDIR)\ltw32.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<  

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<  

################################################################################
# Begin Target

# Name "ltw32 - Win32 Release"
# Name "ltw32 - Win32 Debug"

!IF  "$(CFG)" == "ltw32 - Win32 Release"

!ELSEIF  "$(CFG)" == "ltw32 - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Ltw.c
DEP_CPP_LTW_C=\
	".\ltwd.h"\
	

"$(INTDIR)\Ltw.obj" : $(SOURCE) $(DEP_CPP_LTW_C) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ltw.rc
DEP_RSC_LTW_R=\
	".\Ltw.ico"\
	".\bitmap1.bmp"\
	".\bmp_eng.bmp"\
	".\bmp_fthz.bmp"\
	".\bmp_accu.bmp"\
	".\bmp_inte.bmp"\
	".\bmp_catc.bmp"\
	".\bmp00001.bmp"\
	".\bmp_quit.bmp"\
	".\lex.bmp"\
	

"$(INTDIR)\ltw.res" : $(SOURCE) $(DEP_RSC_LTW_R) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
# End Target
# End Project
################################################################################
