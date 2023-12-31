#ifndef _VERSION_H_INCL
#include "version.h"                   /* SLM maintained version file */
#endif

#if defined(_WIN32) || defined(WIN32)
#include <winver.h>
#else   /* !WIN32 */
#include <ver.h>
#endif  /* !WIN32 */

#ifndef rpt
#define rpt 0
#endif

#if     (rmm < 10)
#define rmmpad "0"
#else
#define rmmpad
#endif

#if     (rpt == 0)
#define VERSION_STR1(a,b,c,d)       #a "." rmmpad #b "." ruppad #c
#else
#define VERSION_STR1(a,b,c,d)       #a "." rmmpad #b "." ruppad #c "." #d
#endif

#if     (rup < 10)
#define ruppad "000"
#elif   (rup < 100)
#define ruppad "00"
#elif   (rup < 1000)
#define ruppad "0"
#else
#define ruppad
#endif

#define VERSION_STR2(a,b,c,d)       VERSION_STR1(a,b,c,d)
#define VER_PRODUCTVERSION_STR      VERSION_STR2(rmj,rmm,rup,rpt)
#define VER_PRODUCTVERSION          rmj,rmm,rup,rpt

/*--------------------------------------------------------------*/
/* the following section defines values used in the version     */
/* data structure for all files, and which do not change.       */
/*--------------------------------------------------------------*/

// DEBUG flag is set for debug build, not set for retail build
#if defined(DEBUG) || defined(_DEBUG)
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

// PRIVATEBUILD flag is set if not built by build lab
#if defined(_SHIP)
#define VER_PRIVATEBUILD            0
#else
#define VER_PRIVATEBUILD            VS_FF_PRIVATEBUILD
#endif

// PRERELEASE flag is always set unless building SHIP version
#if defined(_SHIP)
#define VER_PRERELEASE              0
#else
#define VER_PRERELEASE              VS_FF_PRERELEASE
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#if defined(_WIN32) || defined(WIN32)
#define VER_FILEOS                  VOS__WINDOWS32
#else
#define VER_FILEOS                  VOS_DOS_WINDOWS16
#endif
#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "Microsoft Corporation"
