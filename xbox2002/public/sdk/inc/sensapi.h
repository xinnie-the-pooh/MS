/*++

Copyright (c) 1997-1999 Microsoft Corporation

Module Name:

    sensapi.h

Abstract:

    Public header file for the SENS Connectivity APIs.

Author:

    Gopal Parupudi    <GopalP>

[Notes:]

    optional-notes

Revision History:

    GopalP          10/12/1997         Start.

--*/


#ifndef __SENSAPI_H__
#define __SENSAPI_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif


#define NETWORK_ALIVE_LAN   0x00000001
#define NETWORK_ALIVE_WAN   0x00000002
#define NETWORK_ALIVE_AOL   0x00000004


typedef struct tagQOCINFO
{
    DWORD dwSize;
    DWORD dwFlags;
    DWORD dwInSpeed;
    DWORD dwOutSpeed;
} QOCINFO, *LPQOCINFO;



#ifdef UNICODE
#define  IsDestinationReachable  IsDestinationReachableW
#else
#define  IsDestinationReachable  IsDestinationReachableA
#endif // UNICODE


#if !defined(__midl)

BOOL APIENTRY
IsDestinationReachableA(
    LPCSTR lpszDestination,
    LPQOCINFO lpQOCInfo
    );

BOOL APIENTRY
IsDestinationReachableW(
    LPCWSTR lpszDestination,
    LPQOCINFO lpQOCInfo
    );

BOOL APIENTRY
IsNetworkAlive(
    LPDWORD lpdwFlags
    );

#endif // !defined(__midl)


#ifdef __cplusplus
}
#endif

#endif // __SENSAPI_H__
