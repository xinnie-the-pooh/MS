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
*inithelp.c - Contains the __getlocaleinfo helper routine
*
*       Copyright (c) Microsoft Corporation.  All rights reserved.
*
*Purpose:
*  Contains the __getlocaleinfo helper routine.
*
*Revision History:
*       12-28-92  CFW   Module created, _getlocaleinfo ported to Cuda tree.
*       12-29-92  CFW   Update for new GetLocaleInfoW, add LC_*_TYPE handling.
*       01-25-93  KRS   Change category argument to LCID.
*       02-02-93  CFW   Optimized INT case, update in STR case.
*       02-08-93  CFW   Optimized GetQualifiedLocale call, cast to remove warnings.
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       04-20-93  CFW   JonM's GetLocaleInfoW fixup, cast to avoid trashing memory.
*       05-24-93  CFW   Clean up file (brief is evil).
*       09-15-93  CFW   Use ANSI conformant "__" names.
*       09-22-93  CFW   Use __crtxxx internal NLS API wrapper.
*       09-22-93  CFW   NT merge.
*       11-09-93  CFW   Add code page for __crtxxx().
*       03-31-94  CFW   Include awint.h.
*       04-15-94  GJF   Made definition of wcbuffer conditional on
*                       DLL_FOR_WIN32S
*       09-06-94  CFW   Remove _INTL switch.
*       01-10-95  CFW   Debug CRT allocs.
*       02-02-95  BWT   Update POSIX support.
*       05-13-99  PML   Remove Win32s
*       06-04-04  SJ    VSW#314764 - Adding a _locale_t param to __getlocaleinfo
*	03-07-05  JL	VSW#465724 - Relocate definition of _initp_misc_purevirt
*       04-01-05  MSL   Integer overflow protection
*
*******************************************************************************/

#include <stdlib.h>
#include <cruntime.h>
#include <locale.h>
#include <setlocal.h>
#include <awint.h>
#include <dbgint.h>
#include <ctype.h>
#include <mtdll.h>
#include <internal.h>

/***
*__getlocaleinfo - return locale data
*
*Purpose:
*       Return locale data appropriate for the setlocale init functions.
*       In particular, wide locale strings can be converted to char strings
*       or numeric depending on the value of the first parameter.
*
*       Memory is allocated for the char version of the data, and the
*       calling function's pointer is set to it.  This pointer should later
*       be used to free the data.  The wide-char data is fetched using
*       GetLocaleInfo and converted to multibyte using WideCharToMultiByte.
*
*       *** For internal use by the __init_* functions only ***
*
*       *** Future optimization ***
*       When converting a large number of wide-strings to multibyte, do
*       not query the size of the result, but convert them one after
*       another into a large character buffer.  The entire buffer can
*       also be freed with one pointer.
*
*Entry:
*       int lc_type -
*           LC_INT_TYPE for numeric data
*           LC_STR_TYPE for string data
*           LC_WSTR_TYPE for wide string data
*       wchar_t* localename - Locale name based on category and lang or ctry
*       LCTYPE fieldtype - int or string value
*       void *address - cast to either char *, char **, or wchar_t **
*
*Exit:
*        0  success
*       -1  failure
*
*Exceptions:
*
*******************************************************************************/

#if NO_ERROR == -1 /*IFSTRIP=IGN*/
#error Need to use another error return code in __getlocaleinfo
#endif

#define STR_CHAR_CNT    128

int __cdecl __getlocaleinfo (
        _locale_t plocinfo,
        int lc_type,
        LPCWSTR localename,
        LCTYPE fieldtype,
        void *address
        )
{
#if !defined(_POSIX_)
        if (lc_type == LC_STR_TYPE)
        {
            char **straddress = (char **)address;
            unsigned char cbuffer[STR_CHAR_CNT];
            unsigned char *pcbuffer = cbuffer;
            int bufferused = 0; /* 1 indicates buffer points to malloc'ed memory */
            int buffersize = STR_CHAR_CNT;
            int outsize;

            if ((outsize = __crtGetLocaleInfoA(plocinfo, localename, fieldtype, pcbuffer, buffersize))
                == 0)
            {
                if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                    goto error;

                /* buffersize too small, get required size and malloc new buffer */

                if ((buffersize = __crtGetLocaleInfoA (plocinfo, localename, fieldtype, NULL, 0))
                    == 0)
                    goto error;

                if ((pcbuffer = (unsigned char *) _calloc_crt (buffersize, sizeof(unsigned char)))
                    == NULL)
                    goto error;

                bufferused = 1;

                if ((outsize = __crtGetLocaleInfoA (plocinfo, localename, fieldtype, pcbuffer, buffersize))
                    == 0)
                    goto error;
            }

            if ((*straddress = (char *) _calloc_crt (outsize, sizeof(char))) == NULL)
                goto error;

            _ERRCHECK(strncpy_s(*straddress, outsize, pcbuffer, outsize - 1));

            if (bufferused)
                _free_crt (pcbuffer);

            return 0;

error:
            if (bufferused)
                _free_crt (pcbuffer);
            return -1;

        } else if (lc_type == LC_WSTR_TYPE)
        {
            wchar_t **wstraddress = (wchar_t **)address;
            int buffersize;

            *wstraddress = NULL;

            if ((buffersize = __crtGetLocaleInfoEx(localename, fieldtype, NULL, 0)) == 0)
                goto werror;

            if ((*wstraddress = (wchar_t *) _calloc_crt(buffersize, sizeof(wchar_t))) == NULL)
                goto werror;

            if (__crtGetLocaleInfoEx(localename, fieldtype, *wstraddress, buffersize) == 0)
                goto werror;

            return 0;

werror:
            _free_crt(*wstraddress);
            *wstraddress = NULL;
            return -1;

        } else if (lc_type == LC_INT_TYPE)
        {
            DWORD dw = 0;

            if (__crtGetLocaleInfoEx (localename, fieldtype | LOCALE_RETURN_NUMBER, (LPWSTR)&dw, sizeof(dw) / sizeof(WCHAR)) == 0)
                return -1;

            *(unsigned char *)address = (unsigned char) dw;

            return 0;
        }
#endif  /* _POSIX_ */
        return -1;
}


#ifndef _POSIX_

_purecall_handler __pPurecall= NULL;

/***
*void _initp_misc_purevirt(void) -
*
*Purpose:
*       Initialize the __pPurecall function pointer
*
*Entry:
*	The per-process encoded value for the null pointer.
*
*Exit:
*	Never returns
*
*Exceptions:
*
*******************************************************************************/

//extern "C"
void __cdecl _initp_misc_purevirt(void* enull)
{
    __pPurecall = (_purecall_handler) enull;
}

#endif
