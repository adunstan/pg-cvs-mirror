# Makefile for Microsoft Visual C++ 5.0 (or compat)

# Will build a Win32 static library libpq(d).lib
#        and a Win32 dynamic library libpq(d).dll with import library libpq(d)dll.lib
# USE_SSL=1 will compile with OpenSSL
# DEBUG=1 compiles with debugging symbols


!MESSAGE Building the Win32 static library...
!MESSAGE

!IFDEF DEBUG
OPT=/Od /Zi /MDd
LOPT=/debug
DEBUGDEF=/D _DEBUG
OUTFILENAME=libpqd
!ELSE
OPT=/O2 /MD
LOPT=
DEBUGDEF=/D NDEBUG
OUTFILENAME=libpq
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
RSC=rc.exe

!IFDEF DEBUG
OUTDIR=.\Debug
INTDIR=.\Debug
CPP_OBJS=.\Debug/
!ELSE
OUTDIR=.\Release
INTDIR=.\Release
CPP_OBJS=.\Release/
!ENDIF


ALL : "$(OUTDIR)\$(OUTFILENAME).lib" "$(OUTDIR)\$(OUTFILENAME).dll" 

CLEAN :
	-@erase "$(INTDIR)\getaddrinfo.obj"
	-@erase "$(INTDIR)\thread.obj"
	-@erase "$(INTDIR)\inet_aton.obj"
	-@erase "$(INTDIR)\crypt.obj"
	-@erase "$(INTDIR)\path.obj"
	-@erase "$(INTDIR)\dllist.obj"
	-@erase "$(INTDIR)\md5.obj"
	-@erase "$(INTDIR)\ip.obj"
	-@erase "$(INTDIR)\fe-auth.obj"
	-@erase "$(INTDIR)\fe-protocol2.obj"
	-@erase "$(INTDIR)\fe-protocol3.obj"
	-@erase "$(INTDIR)\fe-connect.obj"
	-@erase "$(INTDIR)\fe-exec.obj"
	-@erase "$(INTDIR)\fe-lobj.obj"
	-@erase "$(INTDIR)\fe-misc.obj"
	-@erase "$(INTDIR)\fe-print.obj"
	-@erase "$(INTDIR)\fe-secure.obj"
	-@erase "$(INTDIR)\pqexpbuffer.obj"
	-@erase "$(OUTDIR)\libpqdll.obj"
	-@erase "$(OUTDIR)\win32.obj"
	-@erase "$(OUTDIR)\$(OUTFILENAME).lib"
	-@erase "$(OUTDIR)\$(OUTFILENAME).dll"
	-@erase "$(OUTDIR)\libpq.res"
	-@erase "*.pch"
	-@erase "$(OUTDIR)\libpq.pch"
	-@erase "$(OUTDIR)\$(OUTFILENAME)dll.exp"
	-@erase "$(OUTDIR)\$(OUTFILENAME)dll.lib"
	-@erase "$(INTDIR)\wchar.obj"
	-@erase "$(INTDIR)\encnames.obj"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /W3 /GX $(OPT) /I "..\..\include" /D "FRONTEND" $(DEBUGDEF) /D\
 "WIN32" /D "_WINDOWS" /Fp"$(INTDIR)\libpq.pch" /YX\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c  /D "HAVE_VSNPRINTF" /D "HAVE_STRDUP"

!IFDEF USE_SSL
CPP_PROJ=$(CPP_PROJ) /D USE_SSL
SSL_LIBS=ssleay32.lib libeay32.lib gdi32.lib
!ENDIF

CPP_SBRS=.

LIB32=link.exe -lib
LIB32_FLAGS=$(LOPT) /nologo /out:"$(OUTDIR)\$(OUTFILENAME).lib" 
LIB32_OBJS= \
	"$(INTDIR)\win32.obj" \
	"$(INTDIR)\getaddrinfo.obj" \
	"$(INTDIR)\thread.obj" \
	"$(INTDIR)\inet_aton.obj" \
        "$(INTDIR)\crypt.obj" \
	"$(INTDIR)\path.obj" \
	"$(INTDIR)\dllist.obj" \
	"$(INTDIR)\md5.obj" \
	"$(INTDIR)\ip.obj" \
	"$(INTDIR)\fe-auth.obj" \
	"$(INTDIR)\fe-protocol2.obj" \
	"$(INTDIR)\fe-protocol3.obj" \
	"$(INTDIR)\fe-connect.obj" \
	"$(INTDIR)\fe-exec.obj" \
	"$(INTDIR)\fe-lobj.obj" \
	"$(INTDIR)\fe-misc.obj" \
	"$(INTDIR)\fe-print.obj" \
	"$(INTDIR)\fe-secure.obj" \
	"$(INTDIR)\pqexpbuffer.obj" \
	"$(INTDIR)\wchar.obj" \
	"$(INTDIR)\encnames.obj"


RSC_PROJ=/l 0x409 /fo"$(INTDIR)\libpq.res"

LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib advapi32.lib wsock32.lib $(SSL_LIBS)  \
 /nologo /subsystem:windows /dll $(LOPT) /incremental:no\
 /pdb:"$(OUTDIR)\libpqdll.pdb" /machine:I386 /out:"$(OUTDIR)\$(OUTFILENAME).dll"\
 /implib:"$(OUTDIR)\$(OUTFILENAME)dll.lib"  /def:$(OUTFILENAME)dll.def
LINK32_OBJS= \
	"$(INTDIR)\libpqdll.obj" \
	"$(OUTDIR)\$(OUTFILENAME).lib" \
	"$(OUTDIR)\libpq.res"


"$(OUTDIR)\$(OUTFILENAME).lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

"$(INTDIR)\libpq.res" : "$(INTDIR)" libpq.rc
    $(RSC) $(RSC_PROJ) libpq.rc


"$(OUTDIR)\$(OUTFILENAME).dll" : "$(OUTDIR)" "$(OUTDIR)\libpqdll.obj" "$(INTDIR)\libpqdll.obj" "$(INTDIR)\libpq.res"
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

"$(INTDIR)\getaddrinfo.obj" : ..\..\port\getaddrinfo.c
    $(CPP) @<<
    $(CPP_PROJ) ..\..\port\getaddrinfo.c
<<

"$(INTDIR)\thread.obj" : ..\..\port\thread.c
    $(CPP) @<<
    $(CPP_PROJ) ..\..\port\thread.c
<<

"$(INTDIR)\inet_aton.obj" : ..\..\port\inet_aton.c
    $(CPP) @<<
    $(CPP_PROJ) ..\..\port\inet_aton.c
<<

"$(INTDIR)\crypt.obj" : ..\..\port\crypt.c
    $(CPP) @<<
    $(CPP_PROJ) ..\..\port\crypt.c
<<

"$(INTDIR)\path.obj" : ..\..\port\path.c
    $(CPP) @<<
    $(CPP_PROJ) ..\..\port\path.c
<<

"$(INTDIR)\dllist.obj" : ..\..\backend\lib\dllist.c
    $(CPP) @<<
    $(CPP_PROJ) ..\..\backend\lib\dllist.c
<<


"$(INTDIR)\md5.obj" : ..\..\backend\libpq\md5.c
    $(CPP) @<<
    $(CPP_PROJ) ..\..\backend\libpq\md5.c
<<

"$(INTDIR)\ip.obj" : ..\..\backend\libpq\ip.c
    $(CPP) @<<
    $(CPP_PROJ) ..\..\backend\libpq\ip.c
<<

"$(INTDIR)\wchar.obj" : ..\..\backend\utils\mb\wchar.c
    $(CPP) @<<
    $(CPP_PROJ) /I "." ..\..\backend\utils\mb\wchar.c
<<


"$(INTDIR)\encnames.obj" : ..\..\backend\utils\mb\encnames.c
    $(CPP) @<<
    $(CPP_PROJ) /I "." ..\..\backend\utils\mb\encnames.c
<<


.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<
