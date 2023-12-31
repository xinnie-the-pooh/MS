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
*_toupper.c - convert character to uppercase
*
*       Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines _Toupper()
*
*Revision History:.
*       01-XX-96  PJP   Created from toupper.c January 1996 by P.J. Plauger
*       04-17-96  GJF   Updated for current locale locking. Also, reformatted
*                       and made several cosmetic changes.
*       03-17-97  RDK   Added error flag to __crtLCMapStringA.
*       05-17-99  PML   Remove all Macintosh support.
*       01-29-01  GB    Added _func function version of data variable used in
*                       msvcprt.lib to work with STATIC_CPPLIB
*       03-12-01  PML   Use supplied locale to check case VS7#190902
*       04-03-01  PML   Reverse lead/trail bytes in composed char (vs7#232853)
*       04-26-02  GB    Fixed problem with operator precedence. problem was
*                       !ploc->_Table[c]&_LOWER
*       04-29-02  GB    Added try-finally arounds lock-unlock.
*       05-06-05  PML   Use passed in codepage, not _cpp_leadbyte (VSW#2495)
*       05-22-05  AC    Moved some function in the import lib for clr:pure
*                       VSW#417363
*       05-23-05  PML   _lock_locale has been useless for years (vsw#497047)
*
*******************************************************************************/

#include <cruntime.h>
#include <ctype.h>
#include <stddef.h>
#include <xlocinfo.h>
#include <locale.h>
#include <mtdll.h>
#include <setlocal.h>
#include <awint.h>
#include <yvals.h>

/* remove macro definitions of _toupper() and toupper()
 */
#undef  _toupper
#undef  toupper

/***
*int _Toupper(c) - convert character to uppercase
*
*Purpose:
*       _Toupper() is a version of toupper with a locale argument.
*
*Entry:
*       c - int value of character to be converted
*       const _Ctypevec * = pointer to locale info
*
*Exit:
*       returns int value of uppercase representation of c
*
*Exceptions:
*
*******************************************************************************/

_CRTIMP2_PURE int __CLRCALL_PURE_OR_CDECL _Toupper (
        int c,
        const _Ctypevec *ploc
        )
{
        int size;
        unsigned char inbuffer[3];
        unsigned char outbuffer[3];

        const wchar_t *locale_name;
        UINT codepage;

        if (ploc == 0)
        {
            locale_name = ___lc_locale_name_func()[LC_CTYPE];
            codepage = ___lc_codepage_func();
        }
        else
        {
            locale_name = ploc->_LocaleName;
            codepage = ploc->_Page;
        }

        if (locale_name == NULL)
        {
            if ( (c >= 'a') && (c <= 'z') )
                c = c - ('a' - 'A');
            return c;
        }

        /* if checking case of c does not require API call, do it */
        if ((unsigned)c < 256)
        {
            if (ploc == 0)
            {
                if (!islower(c))
                {
                    return c;
                }
            }
            else
            {
                if (!(ploc->_Table[c] & _LOWER))
                {
                    return c;
                }
            }
        }

        /* convert int c to multibyte string */
        if (ploc == 0 ? _cpp_isleadbyte((c >> 8) & 0xff)
                      : (ploc->_Table[(c >> 8) & 0xff] & _LEADBYTE) != 0)
        {
            inbuffer[0] = (c >> 8 & 0xff);
            inbuffer[1] = (unsigned char)c;
            inbuffer[2] = 0;
            size = 2;
        } else {
            inbuffer[0] = (unsigned char)c;
            inbuffer[1] = 0;
            size = 1;
        }

        /* convert wide char to uppercase */
        if (0 == (size = __crtLCMapStringA(NULL, locale_name, LCMAP_UPPERCASE,
            (const char *)inbuffer, size, (char *)outbuffer, 3, codepage, TRUE)))
        {
            return c;
        }

        /* construct integer return value */
        if (size == 1)
            return ((int)outbuffer[0]);
        else
            return ((int)outbuffer[1] | ((int)outbuffer[0] << 8));

}
