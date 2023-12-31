!IF 0

Copyright (c) 1989-1996 Microsoft Corporation

Module Name:

    sources.

Abstract:

    This file specifies the target component being built and the list of
    sources files needed to build that component.  Also specifies optional
    compiler switches and libraries that are unique for the component being
    built.


History:
    Common ccshell makefile and cleanup 15-Apr-96 Scott Hysom (scotth)
    Created 27-Sep-94 by Bob Day (bobday)
    from template created 12-Apr-1990 by Steve Wood (stevewo)

NOTE:   Commented description of this file is in \nt\public\oak\bin\sources.tpl

!ENDIF

CCSHELL_DIR     = $(BASEDIR)\private\shell

!include $(CCSHELL_DIR)\cpls\appwiz\appwiz.inc

SOURCES_USED    = $(SOURCES_USED) $(CCSHELL_DIR)\cpls\appwiz\appwiz.inc

NO_BROWSER_FILE = 1

TARGETNAME      = srcc
TARGETPATH      = obj
TARGETTYPE      = LIBRARY

PRECOMPILED_INCLUDE = ..\priv.h

SOURCES         = \
                  ..\applist.c  \
                  ..\appwiz.c   \
                  ..\browse.c   \
                  ..\confopt.c  \
                  ..\dump.c     \
                  ..\dllload.c  \
                  ..\folder.c   \
                  ..\gettitle.c \
                  ..\link.c     \
                  ..\msdos.c    \
                  ..\pickicon.c \
                  ..\setup.c    \
                  ..\uninstal.c \
                  ..\chgusr.c
  



