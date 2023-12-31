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
////////////////////////////////////////////////////////////////////////////////
//
//  TUXTEST TUX DLL
//  Copyright (c) Microsoft Corporation
//
//  Module: PUNJAB.h
//          
//
//  Revision History:changed #define values:
//
////////////////////////////////////////////////////////////////////////////////

#include "..\kbddef.h"

// ****************************************************
// Punjabi keyboard
//
// Punjabi and other Indic keyboards can compose multiple
// keystrokes into composed characters so be careful!
//
//  Num  VK1   VK2    Num   Unicode
testSequence KBDTests_Punjabi[] = 
{
    // Without VK_SHIFT
    {1, {0x20, 0x00}, 1,    0x0020}, // Space key
    //  {1, {0x1B, 0x00}, 1,    0x001B}, // Esc key
    {1, {0x08, 0x00}, 1,    0x0008}, // BackSpace key
    {1, {0x0D, 0x00}, 1,    0x000D}, // Enter key
    {1, {0x09, 0x00}, 1,    0x0009}, // Tab key
    /* Keys 1234567890-= */
    {1, {0x31, 0x00}, 1,    0x0031},      
    {1, {0x32, 0x00}, 1,    0x0032},      
    {1, {0x33, 0x00}, 1,    0x0033},      
    {1, {0x34, 0x00}, 1,    0x0034},      
    {1, {0x35, 0x00}, 1,    0x0035},      
    {1, {0x36, 0x00}, 1,    0x0036},      
    {1, {0x37, 0x00}, 1,    0x0037},      
    {1, {0x38, 0x00}, 1,    0x0038},      
    {1, {0x39, 0x00}, 1,    0X0039},      
    {1, {0x30, 0x00}, 1,    0x0030},      
    {1, {0xBD, 0x00}, 1,    0x002D},      
    {1, {0xBB, 0x00}, 0,    0x0000},      
    /* Keys qwertyuiop[]\ */
    {1, {0x51, 0x00}, 1,    0x0A4C},            
    {1, {0x57, 0x00}, 1,    0x0A48},            
    {1, {0x45, 0x00}, 1,    0x0A3E},            
    {1, {0x52, 0x00}, 1,    0x0A40},            
    {1, {0x54, 0x00}, 1,    0x0A42},            
    {1, {0x59, 0x00}, 1,    0x0A2C},            
    {1, {0x55, 0x00}, 1,    0x0A39},            
    {1, {0x49, 0x00}, 1,    0x0A17},            
    {1, {0x4F, 0x00}, 1,    0x0A26},            
    {1, {0x50, 0x00}, 1,    0x0A1C},            
    {1, {0xDB, 0x00}, 1,    0x0A21},            
    {1, {0xDD, 0x00}, 1,    0x0A3C},            
    {1, {0xDC, 0x00}, 0,    0x0000},            
    /* Keys asdfghjkl;' */
    {1, {0x41, 0x00}, 1,    0x0A4B},            
    {1, {0x53, 0x00}, 1,    0x0A47},            
    {1, {0x44, 0x00}, 1,    0x0A4D},            
    {1, {0x46, 0x00}, 1,    0x0A3F},            
    {1, {0x47, 0x00}, 1,    0x0A41},            
    {1, {0x48, 0x00}, 1,    0x0A2A},            
    {1, {0x4A, 0x00}, 1,    0x0A30},            
    {1, {0x4B, 0x00}, 1,    0x0A15},            
    {1, {0x4C, 0x00}, 1,    0x0A24},            
    {1, {0xBA, 0x00}, 1,    0x0A1A},      
    {1, {0xDE, 0x00}, 1,    0x0A1F},      
    /* Keys xcvbnm,./ */
    {1, {0x58, 0x00}, 1,    0x0A70},            
    {1, {0x43, 0x00}, 1,    0x0A2E},            
    {1, {0x56, 0x00}, 1,    0x0A28},
    {1, {0x42, 0x00}, 1,    0x0A35},
    {1, {0x4E, 0x00}, 1,    0x0A32},            
    {1, {0x4D, 0x00}, 1,    0x0A38},            
    {1, {0xBC, 0x00}, 1,    0x002C},      
    {1, {0xBE, 0x00}, 1,    0x002E},      
    {1, {0xBF, 0x00}, 1,    0x0A2F},      
    // With VK_SHIFT
    /* Keys 1290-= */
    {2, {0x10, 0x34}, 1,    0x0A71},
    {2, {0x10, 0x39}, 1,    0x0028},     
    {2, {0x10, 0x30}, 1,    0x0029},      
    {2, {0x10, 0xBD}, 0,    0x0000},     
    {2, {0x10, 0xBB}, 0,    0x0000},      
    /* Keys qwertyuiop[]\ */
    {2, {0x10, 0x51}, 1,    0x0A14},            
    {2, {0x10, 0x57}, 1,    0x0A10},            
    {2, {0x10, 0x45}, 1,    0x0A06},        
    {2, {0x10, 0x52}, 1,    0x0A08},            
    {2, {0x10, 0x54}, 1,    0x0A0A},
    {2, {0x10, 0x59}, 1,    0x0A2D},            
    {2, {0x10, 0x55}, 1,    0x0A19},            
    {2, {0x10, 0x49}, 1,    0x0A18},            
    {2, {0x10, 0x4F}, 1,    0x0A27},            
    {2, {0x10, 0x50}, 1,    0x0A1D},            
    {2, {0x10, 0xDB}, 1,    0x0A22},            
    {2, {0x10, 0xDD}, 1,    0x0A1E},        
    {2, {0x10, 0xDC}, 0,    0x0000},            
    /* Keys asdfghjkl;' */
    {2, {0x10, 0x41}, 1,    0x0A13},            
    {2, {0x10, 0x53}, 1,    0x0A0F},            
    {2, {0x10, 0x44}, 1,    0x0A05},            
    {2, {0x10, 0x46}, 1,    0x0A07},            
    {2, {0x10, 0x47}, 1,    0x0A09},
    {2, {0x10, 0x48}, 1,    0x0A2B},            
    {2, {0x10, 0x4A}, 0,    0x0000},            
    {2, {0x10, 0x4B}, 1,    0x0A16},            
    {2, {0x10, 0x4C}, 1,    0x0A25},            
    {2, {0x10, 0xBA}, 1,    0x0A1B},    
    {2, {0x10, 0xDE}, 1,    0x0A20},     
    /* Keys xcnm,./ */          
    {2, {0x10, 0x58}, 1,    0x0A02},            
    {2, {0x10, 0x43}, 1,    0x0A23},                        
    {2, {0x10, 0x42}, 1,    0x0A72},
    {2, {0x10, 0x4E}, 1,    0x0A33},            
    {2, {0x10, 0x4D}, 1,    0x0A36},            
    {2, {0x10, 0xBC}, 0,    0x0000},     
    {2, {0x10, 0xBE}, 1,    0x0964},      
    {2, {0x10, 0xBF}, 0,    0x0000},
    // Control chars
    // Row ZXCV
    {2, {0x11, 90}, 1, 0x1A},           // Ctrl Z
    {2, {0x11, 88}, 1, 0x18},           // Ctrl X
    {2, {0x11, 67}, 1, 0x03},           // Ctrl C
    {2, {0x11, 86}, 1, 0x16},           // Ctrl V
    {2, {0x11, 66}, 1, 0x02},           // Ctrl B
    {2, {0x11, 78}, 1, 0x0E},           // Ctrl N
    {2, {0x11, 77}, 1, 0x0D},           // Ctrl M
    //Row ASDF
    {2, {0x11, 65}, 1, 0x01},           // Ctrl A
    {2, {0x11, 83}, 1, 0x13},           // Ctrl S
    {2, {0x11, 68}, 1, 0x04},           // Ctrl D
    {2, {0x11, 70}, 1, 0x06},           // Ctrl F
    {2, {0x11, 71}, 1, 0x07},           // Ctrl G
    {2, {0x11, 72}, 1, 0x08},           // Ctrl H
    {2, {0x11, 74}, 1, 0x0A},           // Ctrl J
    {2, {0x11, 75}, 1, 0x0B},           // Ctrl K
    {2, {0x11, 76}, 1, 0x0C},           // Ctrl L
    // Row QWER
    {2, {0x11, 81}, 1, 0x11},           // Ctrl Q
    {2, {0x11, 87}, 1, 0x17},           // Ctrl W
    {2, {0x11, 69}, 1, 0x05},           // Ctrl E
    {2, {0x11, 82}, 1, 0x12},           // Ctrl R
    {2, {0x11, 84}, 1, 0x14},           // Ctrl T
    {2, {0x11, 89}, 1, 0x19},           // Ctrl Y
    {2, {0x11, 85}, 1, 0x15},           // Ctrl U
    {2, {0x11, 73}, 1, 0x09},           // Ctrl I
    {2, {0x11, 79}, 1, 0x0F},           // Ctrl O
    {2, {0x11, 80}, 1, 0x10},           // Ctrl P
    // With ALT-Gr key (VK_CONTROL, VK_MENU)
    /*keys `1234567890-= */
    {3, {0x0011, 0x0012, 0x00C0}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0031}, 1, 0x0A67},
    {3, {0x0011, 0x0012, 0x0032}, 1, 0x0A68},
    {3, {0x0011, 0x0012, 0x0033}, 1, 0x0A69},
    {3, {0x0011, 0x0012, 0x0034}, 1, 0x0A6A},
    {3, {0x0011, 0x0012, 0x0035}, 1, 0x0A6B},
    {3, {0x0011, 0x0012, 0x0036}, 1, 0x0A6C},
    {3, {0x0011, 0x0012, 0x0037}, 1, 0x0A6D},
    {3, {0x0011, 0x0012, 0x0038}, 1, 0x0A6E},
    {3, {0x0011, 0x0012, 0x0039}, 1, 0x0A6F},
    {3, {0x0011, 0x0012, 0x0030}, 1, 0x0A66},
    /* Keys qwertyuiop[]\ */
    {3, {0x0011, 0x0012, 0x0051}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0057}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0045}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0052}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0054}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0059}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0055}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0049}, 1, 0x0A5A},
    {3, {0x0011, 0x0012, 0x004F}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0050}, 1, 0x0A5B},
    {3, {0x0011, 0x0012, 0x00DB}, 1, 0x0A5C},
    {3, {0x0011, 0x0012, 0x00DD}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x00DC}, 0, 0x0000},
    /* Keys asdfghjkl;' */
    {3, {0x0011, 0x0012, 0x0041}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0053}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0044}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0046}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0047}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0048}, 1, 0x0A5E},
    {3, {0x0011, 0x0012, 0x004A}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x004B}, 1, 0x0A59},
    {3, {0x0011, 0x0012, 0x004C}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x00BA}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x00DE}, 0, 0x0000},
    /* Keys zxcvbnm,./ */
    {3, {0x0011, 0x0012, 0x005A}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0058}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0043}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0056}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x0042}, 1, 0x0A73},
    {3, {0x0011, 0x0012, 0x004E}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x004D}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x00BC}, 0, 0x0000},
    {3, {0x0011, 0x0012, 0x00BE}, 1, 0x0965},
    {3, {0x0011, 0x0012, 0x00BF}, 0, 0x0000},
    // With ALT-Gr + VK_SHIFT
    /*keys `1234567890-= */
    {4, {0x0010, 0x0011, 0x0012, 0x00C0}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0031}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0032}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0033}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0034}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0035}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0036}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0037}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0038}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0039}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0030}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00BD}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00BB}, 0, 0x0000},
    /* Keys qwertyuiop[]\ */
    {4, {0x0010, 0x0011, 0x0012, 0x0051}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0057}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0045}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0052}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0054}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0059}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0055}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0049}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x004F}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0050}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00DB}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00DD}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00DC}, 0, 0x0000},
    /* Keys asdfghjkl;' */
    {4, {0x0010, 0x0011, 0x0012, 0x0041}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0053}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0044}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0046}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0047}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0048}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x004A}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x004B}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x004C}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00BA}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00DE}, 0, 0x0000},
    /* Keys zxcvbnm,./ */
    {4, {0x0010, 0x0011, 0x0012, 0x005A}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0058}, 1, 0x0A74},
    {4, {0x0010, 0x0011, 0x0012, 0x0043}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0056}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x0042}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x004E}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x004D}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00BC}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00BE}, 0, 0x0000},
    {4, {0x0010, 0x0011, 0x0012, 0x00BF}, 0, 0x0000},
    // Keystrokes that generate multiple chars
    // With VK_SHIFT
    /*Keys `123 */
    {2, {0x10, 0xC0}, 2, {0x0A4D, 0x0A39        }},  
    {2, {0x10, 0x31}, 2, {0x0A4D, 0x0A35        }}, 
    {2, {0x10, 0x32}, 2, {0x0A4D, 0x0A2F        }}, 
    {2, {0x10, 0x33}, 2, {0x0A4D, 0x0A30        }}, 
}; // end Punjabi keyboard
