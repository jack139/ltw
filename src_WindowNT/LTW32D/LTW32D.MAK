# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=ltw32d - Win32 Debug
!MESSAGE No configuration specified.  Defaulting to ltw32d - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "ltw32d - Win32 Release" && "$(CFG)" != "ltw32d - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "LTW32D.MAK" CFG="ltw32d - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ltw32d - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "ltw32d - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
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
# PROP Target_Last_Scanned "ltw32d - Win32 Debug"
CPP=cl.exe
RSC=rc.exe
MTL=mktyplib.exe

!IF  "$(CFG)" == "ltw32d - Win32 Release"

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

ALL : "$(OUTDIR)\LTW32D.dll"

CLEAN : 
	-@erase ".\Release\LTW32D.dll"
	-@erase ".\Release\Parse.obj"
	-@erase ".\Release\Ltwdll.obj"
	-@erase ".\Release\Ltwlex.obj"
	-@erase ".\Release\ltw32d.res"
	-@erase ".\Release\vpparse.obj"
	-@erase ".\Release\LTW32D.lib"
	-@erase ".\Release\LTW32D.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /Ob2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/LTW32D.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32 
# ADD BASE RSC /l 0x804 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/ltw32d.res" /d "NDEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/LTW32D.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib /nologo /subsystem:windows /dll /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)/LTW32D.pdb"\
 /machine:I386 /out:"$(OUTDIR)/LTW32D.dll" /implib:"$(OUTDIR)/LTW32D.lib" 
LINK32_OBJS= \
	"$(INTDIR)/Parse.obj" \
	"$(INTDIR)/Ltwdll.obj" \
	"$(INTDIR)/Ltwlex.obj" \
	"$(INTDIR)/vpparse.obj" \
	"$(INTDIR)/ltw32d.res"

"$(OUTDIR)\LTW32D.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "ltw32d - Win32 Debug"

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

ALL : "$(OUTDIR)\LTW32D.dll"

CLEAN : 
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\Debug\LTW32D.dll"
	-@erase ".\Debug\Ltwlex.obj"
	-@erase ".\Debug\Parse.obj"
	-@erase ".\Debug\Ltwdll.obj"
	-@erase ".\Debug\ltw32d.res"
	-@erase ".\Debug\vpparse.obj"
	-@erase ".\Debug\LTW32D.ilk"
	-@erase ".\Debug\LTW32D.lib"
	-@erase ".\Debug\LTW32D.exp"
	-@erase ".\Debug\LTW32D.pdb"
	-@erase ".\Debug\LTW32D.map"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS"\
 /Fp"$(INTDIR)/LTW32D.pch" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c 
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32 
# ADD BASE RSC /l 0x804 /d "_DEBUG"
# ADD RSC /l 0x804 /d "_DEBUG"
RSC_PROJ=/l 0x804 /fo"$(INTDIR)/ltw32d.res" /d "_DEBUG" 
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/LTW32D.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib /nologo /subsystem:windows /dll /map /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib uuid.lib /nologo\
 /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)/LTW32D.pdb"\
 /map:"$(INTDIR)/LTW32D.map" /debug /machine:I386 /out:"$(OUTDIR)/LTW32D.dll"\
 /implib:"$(OUTDIR)/LTW32D.lib" 
LINK32_OBJS= \
	"$(INTDIR)/Ltwlex.obj" \
	"$(INTDIR)/Parse.obj" \
	"$(INTDIR)/Ltwdll.obj" \
	"$(INTDIR)/vpparse.obj" \
	"$(INTDIR)/ltw32d.res"

"$(OUTDIR)\LTW32D.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "ltw32d - Win32 Release"
# Name "ltw32d - Win32 Debug"

!IF  "$(CFG)" == "ltw32d - Win32 Release"

!ELSEIF  "$(CFG)" == "ltw32d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=.\Parse.c
DEP_CPP_PARSE=\
	".\text.h"\
	".\ltwlex.h"\
	

"$(INTDIR)\Parse.obj" : $(SOURCE) $(DEP_CPP_PARSE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Ltwlex.c
DEP_CPP_LTWLE=\
	".\text.h"\
	".\ltwlex.h"\
	

"$(INTDIR)\Ltwlex.obj" : $(SOURCE) $(DEP_CPP_LTWLE) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\ltw32d.rc

"$(INTDIR)\ltw32d.res" : $(SOURCE) "$(INTDIR)"
   $(RSC) $(RSC_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=.\Ltwdll.c
DEP_CPP_LTWDL=\
	".\Res.h"\
	".\text.h"\
	".\ltwlex.h"\
	

"$(INTDIR)\Ltwdll.obj" : $(SOURCE) $(DEP_CPP_LTWDL) "$(INTDIR)"


# End Source File
################################################################################
# Begin Source File

SOURCE=.\vpparse.c
DEP_CPP_VPPAR=\
	".\text.h"\
	".\ltwlex.h"\
	

"$(INTDIR)\vpparse.obj" : $(SOURCE) $(DEP_CPP_VPPAR) "$(INTDIR)"


# End Source File
# End Target
# End Project
################################################################################
