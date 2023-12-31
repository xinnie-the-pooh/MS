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
*_fptostr.c - workhorse routine for converting floating point to string
*
*   Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*   Workhorse routine for fcvt, ecvt.
*
*Revision History:
*   09-17-84  DFW   created
*   03-05-90  GJF   Fixed calling type, added #include <cruntime.h>,
*           removed #include <register.h>, fixed copyright. Also,
*           cleaned up the formatting a bit.
*   07-20-90  SBM   Compiles cleanly with -W3 (added #include <string.h>)
*   08-01-90  SBM   Renamed <struct.h> to <fltintrn.h>
*   09-27-90  GJF   New-style function declarator.
*   06-11-92  GDP   Update: Shorten string if leadig (overflow) digit is 1
*   10-09-92  GDP   Backed out last fix for ecvt (printf regressed)
*   04-06-93  SKS   Replace _CRTAPI* with _cdecl
*   04-27-94  CFW   Replace modified GDP 06-11-92 fix.
*   08-05-94  JWM   Backed out CFW's fix for ecvt (printf regression!).
*   08-19-03  AC    Validate input parameters
*   03-10-04  AC    Return ERANGE when buffer is too small
*   04-08-04  AC    Correct cast in the validation
*
*******************************************************************************/

#include <cruntime.h>
#include <string.h>
#include <stddef.h>
#include <fltintrn.h>
#include <internal.h>

/***
*errno_t _fptostr(buf, size, digits, pflt) - workhorse floating point conversion
*
*Purpose:
*   This is the workhorse routine for fcvt, ecvt. Here is where
*   all the digits are put into a buffer and the rounding is
*   performed and indicators of the decimal point position are set. Note,
*   this must not change the mantissa field of pflt since routines which
*   use this routine rely on this being unchanged.
*
*Entry:
*   char *buf - the buffer in which the digits are to be put
*   int digits - the number of digits which are to go into the buffer
*   STRFLT pflt - a pointer to a structure containing information on the
*       floating point value, including a string containing the
*       non-zero significant digits of the mantissa.
*
*Exit:
*   Changes the contents of the buffer and also may increment the decpt
*   field of the structure pointer to by the 'pflt' parameter if overflow
*   occurs during rounding (e.g. 9.999999... gets rounded to 10.000...).
*
*Exceptions:
*   Input parameters are validated. Refer to the validation section of the function.
*
*******************************************************************************/

errno_t __cdecl _fptostr (
    char *buf,
    size_t sizeInBytes,
    int digits,
    STRFLT pflt
    )
{
    char *pbuf = buf;
    char *mantissa = pflt->mantissa;

    /* validation section */
    _VALIDATE_RETURN_ERRCODE(buf != NULL, EINVAL);
    _VALIDATE_RETURN_ERRCODE(sizeInBytes > 0, EINVAL);
    buf[0] = '\0';
    /* the buffer will contains ndec decimal digits plus an optional
     * overflow digit for the rounding
     */
    _VALIDATE_RETURN_ERRCODE(sizeInBytes > (size_t)((digits > 0 ? digits : 0) + 1), ERANGE);
    _VALIDATE_RETURN_ERRCODE(pflt != NULL, EINVAL);

    /* initialize the first digit in the buffer to '0' (NOTE - NOT '\0')
     * and set the pointer to the second digit of the buffer.  The first
     * digit is used to handle overflow on rounding (e.g. 9.9999...
     * becomes 10.000...) which requires a carry into the first digit.
     */

    *pbuf++ = '0';

    /* Copy the digits of the value into the buffer (with 0 padding)
     * and insert the terminating null character.
     */

    while (digits > 0) {
        *pbuf++ = (*mantissa) ? *mantissa++ : (char)'0';
        digits--;
    }
    *pbuf = '\0';

    /* do any rounding which may be needed.  Note - if digits < 0 don't
     * do any rounding since in this case, the rounding occurs in  a digit
     * which will not be output beause of the precision requested
     */

    if (digits >= 0 && *mantissa >= '5') {
        pbuf--;
        while (*pbuf == '9')
            *pbuf-- = '0';
        *pbuf += 1;
    }

    if (*buf == '1') {
        /* the rounding caused overflow into the leading digit (e.g.
         * 9.999.. went to 10.000...), so increment the decpt position
         * by 1
         */
        pflt->decpt++;
    }
    else {
        /* move the entire string to the left one digit to remove the
         * unused overflow digit.
         */
        memmove(buf, buf+1, strlen(buf+1)+1);
    }

    return 0;
}
