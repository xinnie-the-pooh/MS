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
*wcsncpy.c - copy at most n characters of wide-character string
*
*	Copyright (c) Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines wcsncpy() - copy at most n characters of wchar_t string
*
*Revision History:
*	09-09-91  ETC	Created from strncpy.c.
*	04-07-92  KRS	Updated and ripped out _INTL switches.
*	04-06-93  SKS	Replace _CRTAPI* with __cdecl
*       02-07-94  CFW   POSIXify.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <string.h>

#if defined _M_ARM
#pragma function(wcsncpy)
#endif

/***
*wchar_t *wcsncpy(dest, source, count) - copy at most n wide characters
*
*Purpose:
*	Copies count characters from the source string to the
*	destination.  If count is less than the length of source,
*	NO NULL CHARACTER is put onto the end of the copied string.
*	If count is greater than the length of sources, dest is padded
*	with null characters to length count (wide-characters).
*
*
*Entry:
*	wchar_t *dest - pointer to destination
*	wchar_t *source - source string for copy
*	size_t count - max number of characters to copy
*
*Exit:
*	returns dest
*
*Exceptions:
*
*******************************************************************************/

wchar_t * __cdecl wcsncpy (
	wchar_t * dest,
	const wchar_t * source,
	size_t count
	)
{
	wchar_t *start = dest;

	while (count && (*dest++ = *source++))	  /* copy string */
		count--;

	if (count)				/* pad out with zeroes */
		while (--count)
			*dest++ = L'\0';

	return(start);
}

#endif /* _POSIX_ */
