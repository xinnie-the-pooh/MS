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
*strstrea.h - definitions/declarations for strstreambuf, strstream
*
*       Copyright (c) 1991-2001, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       This file defines the classes, values, macros, and functions
*       used by the strstream and strstreambuf classes.
*       [AT&T C++]
*
*       [Public]
*
*Revision History:
*       01-23-92  KRS   Ported from 16-bit version.
*       02-23-93  SKS   Update copyright to 1993
*       10-12-93  GJF   Support NT and Cuda versions. Enclose #pragma-s in
*                       #ifdef _MSC_VER
*       08-12-94  GJF   Disable warning 4514 instead of 4505.
*       11-03-94  GJF   Changed pack pragma to 8 byte alignment.
*       02-11-95  CFW   Add _CRTBLD to avoid users getting wrong headers.
*       02-14-95  CFW   Clean up Mac merge.
*       05-11-95  CFW   Only for use by C++ programs.
*       12-14-95  JWM   Add "#pragma once".
*       04-09-96  SKS   Change _CRTIMP to _CRTIMP1 for special iostream build
*       04-15-96  JWM   Remove _OLD_IOSTREAMS, add '#pragma comment(lib,"cirt")'.
*       04-16-96  JWM   '#include useoldio.h' replaces '#pragma comment(...)'.
*       02-24-97  GJF   Cleaned out obsolete support for _NTSDK. Also, 
*                       detab-ed.
*       05-17-99  PML   Remove all Macintosh support.
*	08-18-06  KRS	Fix SAL on strstreambuf ctors.
*
****/

#if     _MSC_VER > 1000 /*IFSTRIP=IGN*/
#pragma once
#endif

#ifdef  __cplusplus

#ifndef _INC_STRSTREAM
#define _INC_STRSTREAM

#if     !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#ifndef _CRTBLD
/* This version of the header files is NOT for user programs.
 * It is intended for use when building the C runtimes ONLY.
 * The version intended for public use will not have this message.
 */
#error ERROR: Use of C runtime library internal header file.
#endif  /* _CRTBLD */

#ifdef  _MSC_VER

// Currently, all MS C compilers for Win32 platforms default to 8 byte
// alignment.
#pragma pack(push,8)

#endif  // _MSC_VER

#include <crtdefs.h>
#include <useoldio.h>
#include <iostream.h>

#ifdef  _MSC_VER
#pragma warning(disable:4514)           // disable unwanted /W4 warning
// #pragma warning(default:4514)        // use this to reenable, if necessary
#endif  // _MSC_VER

class _CRTIMP1 strstreambuf : public streambuf  {
public:
                strstreambuf();
                strstreambuf(int);
                strstreambuf(_In_reads_(_Inexpressible_(size)) char *, int size, _In_opt_ char * = 0);
                strstreambuf(_In_reads_(_Inexpressible_(size)) unsigned char *, int size, _In_opt_ unsigned char * = 0);
                strstreambuf(_In_reads_(_Inexpressible_(size)) signed char *, int size, _In_opt_ signed char * = 0);
                strstreambuf(void * (*a)(long), void (*f) (void *));
                ~strstreambuf();

        void    freeze(int =1);
        char * str();

virtual int     overflow(int);
virtual int     underflow();
virtual streambuf* setbuf(_In_ char *, int);
virtual streampos seekoff(streamoff, ios::seek_dir, int);
virtual int     sync();         // not in spec.

protected:
virtual int     doallocate();
private:
        void    _init(_In_reads_(_Inexpressible_(size)) char *, int size, _In_opt_ char *);
        int     x_dynamic;
        int     x_bufmin;
        int     _fAlloc;
        int     x_static;
        void * (* x_alloc)(long);
        void    (* x_free)(void *);
};

class _CRTIMP1 istrstream : public istream {
public:
                istrstream(_In_ char *);
                istrstream(_In_ char *, int);
                ~istrstream();

inline  strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline  char * str() { return rdbuf()->str(); }
};

class _CRTIMP1 ostrstream : public ostream {
public:
                ostrstream();
                ostrstream(_In_ char *, int, int = ios::out);
                ~ostrstream();

inline  int     pcount() const { return rdbuf()->out_waiting(); }
inline  strstreambuf* rdbuf() const { return (strstreambuf*) ios::rdbuf(); }
inline  char *  str() { return rdbuf()->str(); }
};

class _CRTIMP1 strstream : public iostream {    // strstreambase ???
public:
                strstream();
                strstream(_In_ char *, int, int);
                ~strstream();

inline  int     pcount() const { return rdbuf()->out_waiting(); } // not in spec.
inline  strstreambuf* rdbuf() const { return (strstreambuf*) ostream::rdbuf(); }
inline  char * str() { return rdbuf()->str(); }
};

#ifdef  _MSC_VER
// Restore previous packing
#pragma pack(pop)
#endif  // _MSC_VER

#endif  // _INC_STRSTREAM

#endif /* __cplusplus */
