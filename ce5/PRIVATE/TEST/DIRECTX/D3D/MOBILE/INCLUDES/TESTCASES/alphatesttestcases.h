//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// This source code is licensed under Microsoft Shared Source License
// Version 1.0 for Windows CE.
// For a copy of the license visit http://go.microsoft.com/fwlink/?LinkId=3223.
//
// To be included from ..\TestCases.h only

#pragma once

//
// Test function prototype
//
TESTPROCAPI AlphaTestTest(LPTESTCASEARGS pTestCaseArgs);

//
// Number of frames generated by each call to test function
//
#define D3DMQA_ALPHATESTTEST_FRAMECOUNT 1

//
// Test Ordinal Extents
//
#define D3DMQA_ALPHATESTTEST_BASE 3100
#define D3DMQA_ALPHATESTTEST_COUNT 168
#define D3DMQA_ALPHATESTTEST_MAX  (D3DMQA_ALPHATESTTEST_BASE + D3DMQA_ALPHATESTTEST_COUNT - 1)

