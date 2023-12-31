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
//******************************************************************************
//
//
// This source code is licensed under Microsoft Shared Source License
// Version 1.0 for Windows CE.
// For a copy of the license visit http://go.microsoft.com/fwlink/?LinkId=3223.
//******************************************************************************

/*++
Module Name:  
	pc32bitwr.h

Abstract:

    Definition of 32bit PCCard API wrappers

--*/


#ifndef __PCMCIA_WRAPPER_
#define __PCMCIA_WRAPPER_

    BOOL PC32bitWR_Init(LPCTSTR lpDllName) ;
    VOID PC32bitWR_DeInit();
    //Access Function
    CARD_CLIENT_HANDLE CardRegisterClient(CLIENT_CALLBACK CallBackFn, PCARD_REGISTER_PARMS pParms);
    STATUS CardDeregisterClient(CARD_CLIENT_HANDLE hCardClient);

    STATUS CardGetFirstTuple(PCARD_TUPLE_PARMS pGetTupleParms);
    STATUS CardGetNextTuple(PCARD_TUPLE_PARMS pGetTupleParms);
    STATUS CardGetTupleData(PCARD_DATA_PARMS pGetTupleData);

    STATUS CardRequestExclusive(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSocket);
    STATUS CardReleaseExclusive(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSocket);
    STATUS CardRequestDisable(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSocket);
    STATUS CardResetFunction(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSock);

    CARD_WINDOW_HANDLE CardRequestWindow(CARD_CLIENT_HANDLE hCardClient, PCARD_WINDOW_PARMS pCardWinParms);
    STATUS CardReleaseWindow(CARD_WINDOW_HANDLE hCardWin);   
    PVOID CardMapWindow(CARD_WINDOW_HANDLE hCardWindow, UINT32 uCardAddress, UINT32 uSize, PUINT32 pGranularity);
    STATUS CardMapWindowPhysical(CARD_WINDOW_HANDLE hCardWindow, PCARD_WINDOW_ADDRESS pCardWindowAddr) ;
        
    STATUS CardGetStatus(PCARD_STATUS pStatus);

    STATUS CardRequestConfiguration(CARD_CLIENT_HANDLE hCardClient, PCARD_CONFIG_INFO pParms);
    STATUS CardModifyConfiguration(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSock,
                          PUINT16 fAttributes);
    STATUS CardReleaseConfiguration(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSock);
    STATUS CardRequestConfigRegisterPhAddr(CARD_CLIENT_HANDLE hCardClient,CARD_SOCKET_HANDLE hSock,PCARD_WINDOW_ADDRESS pCardWindowAddr,PDWORD pOffset);
    STATUS CardAccessConfigurationRegister(CARD_CLIENT_HANDLE hCardClient,
                                           CARD_SOCKET_HANDLE hSock,UINT8 rw_flag,
                                           UINT8 offset,UINT8 *pValue);
    // Only Support By Lagacy Driver
    STATUS CardReleaseIRQ(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSocket);
    STATUS CardRequestIRQLine(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSocket, UINT16 uSupportedIrqBit, PDWORD pdwIrqNumber, PDWORD pdwSysIrqNumber);
    STATUS CardRequestIRQ(CARD_CLIENT_HANDLE hCardClient, CARD_SOCKET_HANDLE hSocket,
                                  CARD_ISR ISRFunction, UINT32 uISRContextData);

    STATUS CardGetEventMask(CARD_CLIENT_HANDLE hCardClient, PCARD_EVENT_MASK_PARMS pMaskParms);
    STATUS CardSetEventMask(CARD_CLIENT_HANDLE hCardClient, PCARD_EVENT_MASK_PARMS pMaskParms);

    STATUS GetSocketStatus(DWORD dwSocketIndex, PDWORD pdwStatus);
    STATUS EnumSocket(PDWORD pdwNumOfStructure,PSOCKET_DESCRIPTOR pSocketDescriptorArray,PDWORD pdwNumOfStructureCopied);
    STATUS EnumCard(PDWORD pdwNumOfStructure,PCARD_DESCRIPTOR pCardDescriptorArray,PDWORD pdwNumOfStructureCopied);
    STATUS GetSocketIndex( CARD_SOCKET_HANDLE hSocket,PDWORD pdwSocketIndex);

#endif
