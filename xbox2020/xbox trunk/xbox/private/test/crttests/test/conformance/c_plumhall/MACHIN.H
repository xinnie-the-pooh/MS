/* The Plum Hall Validation Suite for C
 * Unpublished copyright (c) 1986-1990, Chiron Systems Inc and Plum Hall Inc.
 * VERSION: 2.00
 * DATE: 90/04/05
 * The "ANSI" mode of this suite corresponds to official ANSI C, X3.159-1989.
 * As per your license agreement, your distribution is not to be moved or copied outside the Designated Site
 * without specific permission from Plum Hall Inc.
 */

#ifndef MACHINE_H
#define MACHINE_H

#include <limits.h>
#define SIGNED_CHAR     (CHAR_MIN < 0)      /* Does char sign-extend ... */
#define UNSIGNED_CHAR   (CHAR_MIN == 0)     /* ... or not? */

#define MAX_UCHAR       UCHAR_MAX       /* biggest unsigned character */
#define MAX_USHORT      USHRT_MAX       /* biggest unsigned short */
#define MAX_UINT        UINT_MAX        /* biggest unsigned int */
#define MAX_INT         INT_MAX         /* biggest int */
#define MAX_ULONG       ULONG_MAX       /* biggest unsigned long */
#define MAX_LONG        LONG_MAX        /* biggest long int */

#include <float.h>
#define FDIGITS_MAX     (FLT_DIG - 1)   /* number of significant digits for single precision comparison. */
#define DIGITS_MAX      (DBL_DIG - 1)   /* number of significant digits for double precision comparison. */
#define LDIGITS_MAX     (LDBL_DIG - 1)  /* number of significant digits for long double precision comparison. */
#endif  /* MACHINE_H */

