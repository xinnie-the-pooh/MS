/*

Copyright (c) 1990-2000  Microsoft Corporation

Module Name:

    ntsdexts.h

Abstract:

    This file contains procedure prototypes and structures
    needed to write old NTSD and KD debugger extensions.

    NOTE:  Newer defitions and interfaces are in wdbgexts.h

Environment:

    runs in the Win32 NTSD debug environment.

Revision History:

--*/

#ifndef _NTSDEXTNS_
#define _NTSDEXTNS_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef
VOID
(__cdecl *PNTSD_OUTPUT_ROUTINE)(
    char *,
    ...
    );

typedef
ULONG_PTR
(*PNTSD_GET_EXPRESSION)(
    char *
    );

typedef
VOID
(*PNTSD_GET_SYMBOL)(
    ULONG_PTR offset,
    PUCHAR pchBuffer,
    ULONG_PTR *pDisplacement
    );

typedef
DWORD
(*PNTSD_DISASM)(
    ULONG_PTR *lpOffset,
    LPSTR lpBuffer,
    ULONG fShowEfeectiveAddress
    );

typedef
BOOL
(*PNTSD_CHECK_CONTROL_C)(
    VOID
    );

typedef struct _NTSD_EXTENSION_APIS {
    DWORD nSize;
    PNTSD_OUTPUT_ROUTINE lpOutputRoutine;
    PNTSD_GET_EXPRESSION lpGetExpressionRoutine;
    PNTSD_GET_SYMBOL lpGetSymbolRoutine;
    PNTSD_DISASM lpDisasmRoutine;
    PNTSD_CHECK_CONTROL_C lpCheckControlCRoutine;
} NTSD_EXTENSION_APIS, *PNTSD_EXTENSION_APIS;

typedef
VOID
(*PNTSD_EXTENSION_ROUTINE)(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    );

#ifdef __cplusplus
}
#endif


#endif // _NTSDEXTNS_
