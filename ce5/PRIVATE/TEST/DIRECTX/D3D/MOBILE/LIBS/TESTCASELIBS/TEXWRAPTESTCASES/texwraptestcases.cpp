//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// This source code is licensed under Microsoft Shared Source License
// Version 1.0 for Windows CE.
// For a copy of the license visit http://go.microsoft.com/fwlink/?LinkId=3223.
//
#include <windows.h>
#include "TexWrapCases.h"
#include "TexWrapTestFunctions.h"
#include "ImageManagement.h"
#include "utils.h"
#include <tux.h>

//
// TexWrapTest
//
//   Render and capture a single test frame; corresponding to a test case
//   for lighting.
//
// Arguments:
//   
//   LPTESTCASEARGS pTestCaseArgs:  Information pertinent to test case execution
//
// Return Value:
//
//   TPR_PASS, TPR_FAIL, TPR_SKIP, or TPR_ABORT; depending on test result.
//
INT TexWrapTest(LPTESTCASEARGS pTestCaseArgs)

{
	//
	// API Result code
	//
	HRESULT hr;

	//
	// Texture capabilities required by this test.
	//
	CONST DWORD RequiredTextureOpCap = D3DMTEXOPCAPS_SELECTARG1;

	//
	// Test Result
	//
	INT Result = TPR_PASS;

	//
	// Target device (local variable for brevity in code)
	//
	LPDIRECT3DMOBILEDEVICE pDevice = NULL;

	//
	// Texture to wrap with
	//
	LPDIRECT3DMOBILETEXTURE pTexture = NULL;

	//
	// The vertex buffer which contains the test primitives.
	//
	LPDIRECT3DMOBILEVERTEXBUFFER pVertexBuffer = NULL;
	UINT VertexBufferStride = 0;

	//
	// The index into the test case table, based off of the test case ID
	//
	DWORD dwTableIndex;

	//
	// The number of passes required for the texture wrapping. (Hopefully 1).
	//
	DWORD dwNumPasses;

	//
	// Device capabilities
	//
	D3DMCAPS Caps;

	//
	// Parameter validation
	//
	if ((NULL == pTestCaseArgs) || (NULL == pTestCaseArgs->pParms) || (NULL == pTestCaseArgs->pDevice))
	{
		OutputDebugString(_T("Invalid argument(s)."));
		Result = TPR_ABORT;
		goto cleanup;
	}

	pDevice = pTestCaseArgs->pDevice;

	//
	// Choose test case subset
	//
	if ((pTestCaseArgs->dwTestIndex >= D3DMQA_TEXWRAPTEST_BASE) &&
	    (pTestCaseArgs->dwTestIndex <= D3DMQA_TEXWRAPTEST_MAX))
	{
		//
		// Lighting test cases
		//

		dwTableIndex = pTestCaseArgs->dwTestIndex - D3DMQA_TEXWRAPTEST_BASE;
	}
	else
	{
		OutputDebugString(L"Invalid test index.\n");
		Result = TPR_FAIL;
		goto cleanup;
	}

    if (FAILED(hr = pDevice->GetDeviceCaps(&Caps)))
    {
        OutputDebugString(L"Could not retrieve device capabilities.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

	//
	// Verify that texture op, required for this test, is supported
	//
	if (!(Caps.TextureOpCaps & RequiredTextureOpCap))
	{
		OutputDebugString(_T("Inadequate TextureOpCaps; skipping.\n"));
		Result = TPR_SKIP;
		goto cleanup;
	}


    /////////////////////////////////////////////////////
    //
    // Prepare our test objects
    //
    /////////////////////////////////////////////////////
	
    //
    // Create test texture(s)
    //

    if (FAILED(hr = CreateAndPrepareTexture(pDevice, dwTableIndex, &pTexture)))
    {
        OutputDebugString(L"Could not create and prepare texture. Aborting.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

    //
    // Create test vertex buffer
    //

    if(FAILED(hr = CreateAndPrepareVertexBuffer(pDevice, pTestCaseArgs->hWnd, dwTableIndex, &pVertexBuffer, &VertexBufferStride)))
    {
        OutputDebugString(L"Could not create and prepare vertex buffer. Aborting.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }


    /////////////////////////////////////////////////////
    //
    // Prepare the pipeline
    //
    /////////////////////////////////////////////////////
    

    //
    // Set the default texture stage states
    //

    // SetDefaultTextureStates is from FVFTestCases\utils.*
    if (FAILED(hr = SetDefaultTextureStates(pDevice)))
    {
        OutputDebugString(L"Could not set the default texture states.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

    //
    // Clear the Texture Transform Flag
    //

    if (FAILED(hr = pDevice->SetTextureStageState(0, D3DMTSS_TEXTURETRANSFORMFLAGS, D3DMTTFF_DISABLE)))
    {
        OutputDebugString(L"Could not set default TTFF state.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }
    
    //
    // Set the proper texture stage states for the test
    //

    if (FAILED(hr = SetupTextureStages(pDevice, dwTableIndex)))
    {
        OutputDebugString(L"Could not setup the texture stages.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

    //
    // Set the coordinate transformations
    //

    if (FAILED(hr = SetTransforms(pDevice)))
    {
        OutputDebugString(L"Could not set the coordinate transformations.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

    //
    // Set the Texture Transform Flag accordingly
    //

    if (FAILED(hr = SetupTextureTransformFlag(pDevice, dwTableIndex)))
    {
        OutputDebugString(L"Could not setup the texture transform flag.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

    //
    // Set the pipeline's Stream Source
    //

    if (FAILED(hr = pDevice->SetStreamSource(0, pVertexBuffer, VertexBufferStride)))
    {
        OutputDebugString(L"Could not set the stream source to the test vertex buffer.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

    //
    // Set the Render states for the current test case
    //

    if (FAILED(hr = SetupRenderState(pDevice, dwTableIndex)))
    {
        OutputDebugString(L"Could not set the shade render state.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

    //
    // Verify that the current pipeline state is supported
    //

    if (FAILED(hr = pDevice->ValidateDevice(&dwNumPasses)))
    {
        OutputDebugString(L"ValidateDevice failed, current test case unsupported. Skipping.\n");
        Result = TPR_SKIP;
        goto cleanup;
    }

    /////////////////////////////////////////////////////
    //
    // Use the pipeline!
    //
    /////////////////////////////////////////////////////

    //
    // Clear the device
    //

    if (FAILED(hr = pDevice->Clear(0, NULL, D3DMCLEAR_TARGET | D3DMCLEAR_ZBUFFER, D3DMCOLOR_XRGB(255, 0, 0), 1.0, 0)))
    {
        OutputDebugString(L"Could not clear target and zbuffer.\n");
        Result = TPR_ABORT;
        goto cleanup;
    }

    //
    // Begin the scene
    //
    
	if (FAILED(hr = pDevice->BeginScene()))
	{
		OutputDebugString(L"BeginScene failed.\n");
		Result = TPR_FAIL;
		goto cleanup;
	}

    //
    // DrawPrimitive
    //
	if (FAILED(hr = pDevice->DrawPrimitive(D3DMQA_PRIMTYPE, 0, D3DMQA_NUMPRIM)))
	{
		OutputDebugString(L"DrawPrimitive failed.\n");
		Result = TPR_ABORT;
		goto cleanup;
	}

    //
    // End the scene
    //
    
	if (FAILED(hr = pDevice->EndScene()))
	{
		OutputDebugString(L"EndScene failed.\n");
		Result = TPR_FAIL;
		goto cleanup;
	}

    //
	// Presents the contents of the next buffer in the sequence of
	// back buffers owned by the device.
	//
	if (FAILED(pDevice->Present(NULL, NULL, NULL, NULL)))
	{
		OutputDebugString(L"Present failed.\n");
		Result = TPR_FAIL;
		goto cleanup;
	}
	
	//
	// Flush the swap chain and capture a frame
	//
	if (FAILED(DumpFlushedFrame(pTestCaseArgs, // LPTESTCASEARGS pTestCaseArgs
	                            0,             // UINT uiFrameNumber,
	                            NULL)))        // RECT *pRect = NULL
	{
		OutputDebugString(_T("DumpFlushedFrame failed."));
		Result = TPR_FAIL;
		goto cleanup;
	}

cleanup:
    if (pVertexBuffer)
    {
        pDevice->SetStreamSource(0, NULL, 0);
        pVertexBuffer->Release();
    }
    if (pTexture)
    {
        pDevice->SetTexture(0, NULL);
        pTexture->Release();
    }
	return Result;
}
