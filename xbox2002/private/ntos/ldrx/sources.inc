!IF 0

Copyright (c) 1989-2001  Microsoft Corporation

!ENDIF

MAJORCOMP=ntos
MINORCOMP=ldrx

TARGETPATH=$(BASEDIR)\private\ntos\obj
TARGETTYPE=LIBRARY

INCLUDES=$(BASEDIR)\private\ntos\inc

MSC_WARNING_LEVEL=/W4 /WX

C_DEFINES=$(C_DEFINES) -D_NTSYSTEM_

!if defined(ARCADE)
C_DEFINES=$(C_DEFINES) -DARCADE
!endif

!IF DEFINED(INTERIMPUBKEY)
C_DEFINES=$(C_DEFINES) -DINTERIMPUBKEY=1
!ENDIF

SOURCES=..\loader.c
