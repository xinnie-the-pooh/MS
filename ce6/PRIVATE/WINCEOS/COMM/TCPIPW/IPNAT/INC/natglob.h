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
/*++


Module Name:

    natglob.h

Abstract:

    Contains global variable declarations and global constants for the IPNAT component



    David Kanz (davidka) 11-June-1999

Revision History:

--*/

#ifndef _NATGLOB_H_
#define _NATGLOB_H_

#include <rtinfo.h>             // for RTR_INFO_BLOCK_HEADER
#include <ipinfoid.h>           // for IP_GENERAL_INFO_BASE

#ifdef __cplusplus
extern "C" {
#endif

#ifdef UNDER_CE

// Afd function tables
extern PFNVOID *v_apSocketFns;
extern PFNVOID *v_apAfdFns;

// Use 3 day default lease
#define DHCP_DEFAULT_LEASE_TIME                  (3 * 24 * 60)

extern const TCHAR ICSRegServersKey[];
extern const TCHAR ICSRegAddressKey[];


// FILETIME (100-ns intervals) to minutes (10 x 1000 x 1000 x 60)
#define FILETIME_TO_MINUTES ((__int64)600000000L)
extern ULONG            v_DhcpLeaseTime;
extern unsigned __int64 v_DhcpStoreAddressTime;


// 
// Registry settings for connection sharing
//
#define ICSRegKey                  TEXT("COMM\\ConnectionSharing")
#define ICSRegApplicationsKey      TEXT("COMM\\ConnectionSharing\\Applications")
#define ICSRegServersKey           TEXT("COMM\\ConnectionSharing\\Servers")
#define ICSRegIEHKey               TEXT("COMM\\ConnectionSharing\\InternalExposedHost")
#define ICSRegAddressKey           TEXT("COMM\\ConnectionSharing\\Addresses")
#define ICSRegPublicInterface      TEXT("PublicInterface")  // REG_SZ
#define ICSRegPrivateInterface     TEXT("PrivateInterface") // REG_MULTI_SZ
#define ICSRegDisabledOnBoot       TEXT("DisabledOnBoot")            // DWORD
#define ICSRegSetLastDhcpAddrToIEH TEXT("SetLastDhcpAddrToIEH")     // DWORD
#define ICSRegEnableNAT            TEXT("EnableAddressTranslation") // DWORD
#define ICSRegEnableDhcp           TEXT("EnableDhcpAllocator")      // DWORD
#define ICSRegEnableDns            TEXT("EnableDnsProxy")           // DWORD
#define ICSRegEnableWins           TEXT("EnableWinsProxy")          // DWORD
#define ICSRegEnableFirewall       TEXT("EnablePacketFiltering")    // DWORD
#define ICSRegDhcpLeaseTime        TEXT("DhcpLeaseTime")
#define ICSRegInternalExposedHost  TEXT("InternalExposedHost")
#define ICSRegDhcpAllocatorDomain  TEXT("DhcpAllocatorDomain")
#define ICSRegDomain               TEXT("Domain")
#define ICSRegDhcpBeginRange       TEXT("DhcpAllocationStartRange")
#define ICSRegDhcpEndRange         TEXT("DhcpAllocationEndRange")
#define ICSRegDhcpGateway          TEXT("DhcpDefaultGatewayInfo")

#define ICSLogRegKey                TEXT("Comm\\IPNat")
#define ICSLogDLLValue              TEXT("FirewallLoggerDLL")
#define ICSLogDropFunction          TEXT("LogDroppedPacket")
#define ICSLogCreationFunction      TEXT("LogConnectionCreation")
#define ICSLogDeletionFunction      TEXT("LogConnectionDeletion")
#define ICSLogInitFunction          TEXT("LogInit")

#define ICSRegProtoTCP              TEXT("TCP")
#define ICSRegProtoUDP              TEXT("UDP")
#define ICS_MAX_PROTO_STRING        4

#endif // UNDER_CE


//  Functions called by NAT init/deinit routines.
BOOLEAN
DhcpInitializeModule(
    VOID
    );

ULONG
DhcpRmAddInterface(
#ifdef UNDER_CE
    WCHAR *IFName,
#endif    
    ULONG Index,
    IN ULONG IPAddr,
    IN ULONG IPMask
    );

ULONG
DhcpRmDeleteInterface(
    ULONG Index
    );


BOOLEAN
DnsInitializeModule(
    VOID
    );

ULONG
APIENTRY
DnsRmAddPrivateInterface(
    ULONG Index,
    IN ULONG IPAddr,
	IN ULONG IPMask,
	ULONG EnableDNS,
	ULONG EnableWINS
    );

ULONG
APIENTRY
DnsRmDeleteInterface(
    ULONG Index
    );





#ifdef __cplusplus
}
#endif

#endif // _NATGLOB_H_
