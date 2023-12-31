//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft shared
// source or premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license agreement,
// you are not authorized to use this source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the SOURCE.RTF on your install media or the root of your tools installation.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
#ifndef __CMN_PRINT_H__
#define __CMN_PRINT_H__

#include <windows.h>

#ifndef UNDER_CE
#include <tchar.h>
#endif

// Types of print functions, if you want to define your own, #define MAX_PRINT_TYPE before this header
// The types are 0-indexed, see below
#ifndef MAX_PRINT_TYPE
#define PT_LOG				0
#define PT_WARN1			1
#define PT_WARN2			2
#define PT_FAIL				3
#define PT_VERBOSE			4
#define MAX_PRINT_TYPE			4
#endif

#define PT_ALL				(ULONG)(-1)

#ifndef MAX_PRINT_CHARS
#define MAX_PRINT_CHARS			(1024-64)					// Maximum characters that CmnPrint can output
#endif

typedef void (*LPF_PRINT)(LPCTSTR szText);			// For simple print functions like OutputDebugString
typedef void (*LPF_PRINTEX)(LPCTSTR format, ...);	// For more complex print functions like printf

#ifdef __cplusplus
extern "C" {
#endif
void CmnPrint_Initialize();
void CmnPrint_Cleanup();
BOOL CmnPrint_RegisterPrintFunction(DWORD dwType, LPF_PRINT lpfPrint, BOOL fAddEOL);
BOOL CmnPrint_RegisterPrintFunctionEx(DWORD dwType, LPF_PRINTEX lpfPrint, BOOL fAddEOL);
void CmnPrint(DWORD dwType, const TCHAR *pFormat, ...);
#ifdef __cplusplus
}
#endif

#endif // __CMN_PRINT_H__