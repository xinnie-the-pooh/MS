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
//
//  _RTTIBaseClassDescriptor
//
//  TypeDescriptor is declared in rttidata.h
//
#ifndef WANT_NO_TYPES
typedef const struct    _s_RTTIBaseClassDescriptor  {
    TypeDescriptor                  *pTypeDescriptor;
    DWORD                           numContainedBases;
    PMD                             where;
    DWORD                           attributes;
    } _RTTIBaseClassDescriptor;
#endif // WANT_NO_TYPES

#define BCD_NOTVISIBLE              0x00000001
#define BCD_AMBIGUOUS               0x00000002
#define BCD_PRIVORPROTINCOMPOBJ     0x00000004
#define BCD_PRIVORPROTBASE          0x00000008
#define BCD_VBOFCONTOBJ             0x00000010
#define BCD_NONPOLYMORPHIC          0x00000020

#define BCD_PTD(bcd)                ((bcd).pTypeDescriptor)
#define BCD_NUMCONTBASES(bcd)       ((bcd).numContainedBases)
#define BCD_WHERE(bcd)              ((bcd).where)
#define BCD_ATTRIBUTES(bcd)         ((bcd).attributes)


//
//  _RTTIBaseClassArray
//
#pragma warning(disable:4200)       // get rid of obnoxious nonstandard extension warning
#ifndef WANT_NO_TYPES
typedef const struct    _s_RTTIBaseClassArray   {
    _RTTIBaseClassDescriptor        *arrayOfBaseClassDescriptors[];
    } _RTTIBaseClassArray;
#endif // WANT_NO_TYPES
#pragma warning(default:4200)


//
//  _RTTIClassHierarchyDescriptor
//
#ifndef WANT_NO_TYPES
typedef const struct    _s_RTTIClassHierarchyDescriptor {
    DWORD                           signature;
    DWORD                           attributes;
    DWORD                           numBaseClasses;
    _RTTIBaseClassArray             *pBaseClassArray;
    } _RTTIClassHierarchyDescriptor;
#endif // WANT_NO_TYPES

#define CHD_MULTINH                 0x00000001
#define CHD_VIRTINH                 0x00000002
#define CHD_AMBIGUOUS               0x00000004

#define CHD_SIGNATURE(chd)          ((chd).signature)
#define CHD_ATTRIBUTES(chd)         ((chd).attributes)
#define CHD_NUMBASES(chd)           ((chd).numBaseClasses)
#define CHD_PBCA(chd)               ((chd).pBaseClassArray)

//
//  _RTTICompleteObjectLocator
//
#ifndef WANT_NO_TYPES
typedef const struct    _s_RTTICompleteObjectLocator    {
    DWORD                           signature;
    DWORD                           offset;
    DWORD                           cdOffset;
    TypeDescriptor                  *pTypeDescriptor;
    _RTTIClassHierarchyDescriptor   *pClassDescriptor;
    } _RTTICompleteObjectLocator;
#endif // WANT_NO_TYPES

#define COL_SIGNATURE(col)          ((col).signature)
#define COL_OFFSET(col)             ((col).offset)
#define COL_CDOFFSET(col)           ((col).cdOffset)
#define COL_PTD(col)                ((col).pTypeDescriptor)
#define COL_PCHD(col)               ((col).pClassDescriptor)

#ifdef BUILDING_TYPESRC_C
//
// Type of the result of __RTtypeid and internal applications of typeid().
// This also introduces the tag "type_info" as an incomplete type.
//

typedef const class type_info &__RTtypeidReturnType;

//
// Declaration of CRT entrypoints, as seen by the compiler.  Types are 
// simplified so as to avoid type matching hassles.
//

// Perform a dynamic_cast on obj. of polymorphic type
extern "C" PVOID __cdecl __RTDynamicCast (
                                PVOID,              // ptr to vfptr
                                LONG,               // offset of vftable
                                PVOID,              // src type
                                PVOID,              // target type
                                BOOL);              // isReference

// Perform 'typeid' on obj. of polymorphic type
extern "C" PVOID __cdecl __RTtypeid (PVOID);        // ptr to vfptr

// Perform a dynamic_cast from obj. of polymorphic type to void*
extern "C" PVOID __cdecl __RTCastToVoid (PVOID);    // ptr to vfptr
#endif
