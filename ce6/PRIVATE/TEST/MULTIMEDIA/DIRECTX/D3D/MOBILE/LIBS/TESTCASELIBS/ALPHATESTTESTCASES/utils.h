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
#pragma once
// Included for the SetDefaultTextureStages method.
#include "..\FVFTestCases\utils.h"

#define countof(x) (sizeof(x) / sizeof(*x))

// Macros to get individual channels from D3DMCOLOR value
#define A(color) ((((UINT)color) >> 24) & 0xff)
#define R(color) ((((UINT)color) >> 16) & 0xff)
#define G(color) ((((UINT)color) >> 8) & 0xff)
#define B(color) (((UINT)color) & 0xff)

namespace AlphaTestNamespace
{
    bool IsRefDriver(LPDIRECT3DMOBILEDEVICE pDevice);

    HRESULT GetBestFormat(
        LPDIRECT3DMOBILEDEVICE pDevice,
        D3DMRESOURCETYPE       ResourceType,
        D3DMFORMAT           * pFormat);
        
    HRESULT GetBestAlphaFormat(
        LPDIRECT3DMOBILEDEVICE pDevice,
        D3DMRESOURCETYPE       ResourceType,
        D3DMFORMAT           * pFormat);

    HRESULT FillAlphaSurface(LPDIRECT3DMOBILESURFACE pSurface, TextureFill TexFill, D3DMCOLOR Color);

    bool IsPowerOfTwo(DWORD dwNum);
    DWORD NearestPowerOfTwo(DWORD dwNum);
};
