####
#fp32.def - definitions for the fp32 libs build
#
#       Copyright (c) 1991-1994, Microsoft Corporation. All rights reserved.
#
#Purpose:
#       This file is included in the 'sources' files in this tree
#
#       Key to FP32 environment variables:
#
#       CRTLIBDEBUG = debug flag (define for debug libs)
#       CRTLIBTYPE = [dll/mt/st], dll = dynlink
#                                 mt = multithread,
#                                 st = singlethread
#
#Revision History:
#        9-26-91  GDP
#        2-07-92  GDP   DLL support (according to crt32.def)
#        3-04-92  GDP   Enabled 386 optimizations
#       05-22-93  SRW   Compile runtines with no debug info except globals.
#       06-03-93  SRW   Okay to allow FPO now, as crt32\startup\mlock.c has
#                       been fixed to explicitly disable FPO for itself.
#       10-18-93  SRW   Disable intrinsics on Alpha
#       11-19-93  GJF   Merged in NT SDK version.
#       01-04-94  GJF   Picked up MIPS_OPTIMIZATION from NT SDK version
#       01-11-94  GJF   Variant of fp32.def for NT SDK build.
#       09-06-94  CFW   Remove MTHREAD and _INTL definitions.
#
################################################################################

NTLEGO=1
LINKER_FLAGS=-ignore:4006
LIBRARIAN_FLAGS=-ignore:4006
MSC_WARNING_LEVEL=/W3 /WX

!if $(FREEBUILD)
DEBUG_CDEFINE = -DNDEBUG
!else
DEBUG_CDEFINE = -D_DEBUG
!endif

386_STDCALL=0
TARGETTYPE=LIBRARY
TARGETPATH=obj
INCLUDES=$(INCLUDES)

!if "$(CRTLIBTYPE)" == "MT"
TARGETNAMESUFFIX=mt
MTOPTION=-D_MT -D_BUILD_FOR_MT
C_DEFINES1=$(DEBUG_CDEFINE)
ASM_DEFINES1=
!elseif "$(CRTLIBTYPE)" == "ST"
TARGETNAMESUFFIX=
MTOPTION=
C_DEFINES1=$(DEBUG_CDEFINE)
ASM_DEFINES1=
!elseif "$(CRTLIBTYPE)" == "NT"
TARGETNAMESUFFIX=nt
MTOPTION=
C_DEFINES1=-D_NTSUBSET_=1 -D_NTSYSTEM_=1
ASM_DEFINES1=-D_NTSUBSET_=1 -D_NTSYSTEM_=1
!else
!ERROR Unsupported Library CRTLIBTYPE: $(CRTLIBTYPE)
!ENDIF

C_DEFINES1=$(C_DEFINES1) -DNT_BUILD -D_CRTBLD -D_XBOX
ASM_DEFINES1=$(C_DEFINES1) -DNT_BUILD -D_WIN32=1 -D_XBOX

C_DEFINES  =$(MTOPTION) $(C_DEFINES1)
ASM_DEFINES=$(MTOPTION) -DI386 -DFLAT386 $(ASM_DEFINES1)
