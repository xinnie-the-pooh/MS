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
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
/***
*tell.c - find file position
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       contains _tell() - find file position
*
*Revision History:
*       09-02-83  RN    initial version
*       12-11-87  JCR   Added "_LOAD_DS" to declaration
*       03-13-90  GJF   Replaced _LOAD_DS with _CALLTYPE1, added #include
*                       <cruntime.h> and fixed the copyright. Also, cleaned
*                       up the formatting a bit.
*       10-01-90  GJF   New-style function declarator.
*       01-17-91  GJF   ANSI naming.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       02-15-95  GJF   Appended Mac version of source file (somewhat cleaned
*                       up), with appropriate #ifdef-s.
*       07-09-96  GJF   Replaced defined(_WIN32) with !defined(_MAC) and
*                       defined(_M_M68K) || defined(_M_MPPC) with 
*                       defined(_MAC). Also, detab-ed and cleaned up the 
*                       format a bit.
*       05-17-99  PML   Remove all Macintosh support.
*
*******************************************************************************/

#include <cruntime.h>
#include <io.h>

/***
*long _tell(filedes) - find file position
*
*Purpose:
*       Gets the current position of the file pointer (no adjustment
*       for buffering).
*
*Entry:
*       int filedes - file handle of file
*
*Exit:
*       returns file position or -1L (sets errno) if bad file descriptor or
*       pipe
*
*Exceptions:
*
*******************************************************************************/

long __cdecl _tell (
        int filedes
        )
{
        return(_lseek(filedes,0L,1));
}
