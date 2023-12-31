#include <d3d9.h>
#include <d3dx9.h>
#include "MipGen.h"

CMipGenEmuCubeTexture::CMipGenEmuCubeTexture()
{
	m_szTestName = "Emulation of AutoGenMipMap with cube textures";
	m_szCommandKey = "EmuCubeTexture";

	// Init framework options
	m_Options.D3DOptions.fMinDXVersion = 8.0f;

	// Initialize some variables
    m_pTextures = NULL;
	m_pPatternTexture = NULL;
	for (int iFace=0; iFace<6; iFace++)
	{
		m_pPatternSurface[iFace] = NULL;
	}

	m_RTYPECurrent = RTYPE_CUBETEXTURE;
}

bool CMipGenEmuCubeTexture::CapsCheck()
{
	if (! CMipGenEmulation::CapsCheck() )
		return false;

	return CheckCubeTextureCaps();
}

bool CMipGenEmuCubeTexture::CreateBaseTexture()
{
	RELEASE(m_pTextures);

	// Indicate a CreateTexture failure
	if ( !CreateCubeTexture(256,0,USAGE_RENDERTARGET,m_pCurrentCase->format,POOL_DEFAULT,(LPCUBETEXTURES*)&m_pTextures) || (NULL == m_pTextures) )
	{
		WriteToLog("CreateBaseTexture failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
        return false;
	}

	return true;
}

bool CMipGenEmuCubeTexture::CreatePatternSurface()
{
	ReleasePatternSurface();

	if ( !CreateCubeTexture(256, 1, 0, m_pCurrentCase->format, POOL_MANAGED, &m_pPatternTexture) )
	{
		WriteToLog("CreatePatternSurface() - CreateCubeTexture() failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
		return false;
	}

	for (int iFace=0; iFace<6; iFace++)
	{
		//Get the top level of the texture
		if ( !m_pPatternTexture->GetCubeMapSurface((CUBEMAP_FACES)iFace, 0, &m_pPatternSurface[iFace], CD3D_ALL) )
		{
			WriteToLog("CreatePatternSurface() - GetSurfaceLevel(m_pPatternSurface) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
			RELEASE(m_pPatternTexture);
			return false;
		}

		for (int iDevice=0; iDevice<2; iDevice++)
		{
			LPSURFACE pSurf = NULL;
			m_pPatternSurface[iFace]->GetSurface(&pSurf, iDevice);

			if (!WritePatternOnSurface(pSurf, m_pCurrentCase->dwPattern, iFace))
			{
				RELEASE(pSurf);
				WriteToLog("CreatePatternSurface() - WritePatternOnSurface() failed.\n");
				return false;
			}

			RELEASE(pSurf);
		}
	}

	return true;
}

void CMipGenEmuCubeTexture::ReleasePatternSurface()
{
	for (int iFace=0; iFace<6; iFace++)
	{
		RELEASE(m_pPatternSurface[iFace]);
	}
	RELEASE(m_pPatternTexture);
}

bool CMipGenEmuCubeTexture::RenderMipLevels()
{
	HRESULT hr;
	LPSURFACES pRTSurf = NULL, pOriginalRT = NULL;
	LPTEXTURES pTempTex = NULL;
	LPTEXTURES pTempRTTex = NULL;
	LPSURFACES pTempSurf = NULL;
	LPSURFACES pPreviousSurf = NULL;
	bool bResult = false;

	// Get the current render target
	if ( !GetRenderTarget(&pOriginalRT) )
	{
		WriteToLog("RenderMipLevels - GetRenderTarget() failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
		goto Exit;
	}

	//Set the min filter (autogen filter) (also set the mag filter since some driver have problems with that)
	SetSamplerState(0, SAMP_MAXMIPLEVEL, 0);
    SetSamplerState(0, SAMP_MIPFILTER, TEXF_NONE);
	SetSamplerState(0, SAMP_MINFILTER, m_pCurrentCase->dwAutoGenFilter);
	if ( m_bMagTexFilSupported[m_pCurrentCase->uAutoGenFilterIndex] && m_bMagTexFilSupportedRef[m_pCurrentCase->uAutoGenFilterIndex] )
		SetSamplerState(0, SAMP_MAGFILTER, m_pCurrentCase->dwAutoGenFilter);

	UINT size = 256;

	for (int iFace = 0; iFace<6; iFace++)
	{
		//Fill the top level

		// Get the temp texture top level surface
		if ( !((LPCUBETEXTURES)m_pTextures)->GetCubeMapSurface((CUBEMAP_FACES)iFace, 0, &pTempSurf, CD3D_ALL) || (NULL == pTempSurf) )
		{
			WriteToLog("RenderMipLevels - GetSurfaceLevel(pTempSurf) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
			goto Exit;
		}

		SetTexture(0, m_pPatternTexture);

		// Set the current surface as the render target
		if ( !SetRenderTarget(pTempSurf, NULL) )
		{
			WriteToLog("RenderMipLevels - SetRenderTarget(pTempSurf) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
			goto Exit;
		}

		//Render on the temp texture
		DrawCubeTextureFace(0, 0, 256, iFace);

		RELEASE(pTempSurf);
		RELEASE(pTempTex);

		for (int i = 1; i < 9; ++i)
		{
			// Use the previous level to generate the current level
			size = 1 << (9 - i);

			// Create the temp texture
			if ( !CreateTexture(size,size,1,0,m_pCurrentCase->format,POOL_SYSTEMMEM,&pTempTex) || (NULL == pTempTex) )
			{
				WriteToLog("RenderMipLevels - CreateTexture(pTempTex) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
				goto Exit;
			}

			// Create the temp RT texture
			if ( !CreateTexture(size,size,1,USAGE_RENDERTARGET,m_pCurrentCase->format,POOL_DEFAULT,&pTempRTTex) || (NULL == pTempTex) )
			{
				WriteToLog("RenderMipLevels - CreateTexture(pTempRTTex) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
				goto Exit;
			}

			// Get the temp texture top level surface
			if ( !pTempTex->GetSurfaceLevel(0, &pTempSurf) || (NULL == pTempSurf) )
			{
				WriteToLog("RenderMipLevels - GetSurfaceLevel(pTempSurf) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
				goto Exit;
			}

			// Get the previous mip surface
			if ( !((LPCUBETEXTURES)m_pTextures)->GetCubeMapSurface((CUBEMAP_FACES)iFace, i - 1, &pPreviousSurf, CD3D_ALL) || (NULL == pPreviousSurf) )
			{
				WriteToLog("RenderMipLevels - GetSurfaceLevel(pPreviousSurf) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
				goto Exit;
			}

			// Get the current mip surface
			if ( !((LPCUBETEXTURES)m_pTextures)->GetCubeMapSurface((CUBEMAP_FACES)iFace, i, &pRTSurf, CD3D_ALL) || (NULL == pRTSurf) )
			{
				WriteToLog("RenderMipLevels - GetSurfaceLevel(pRTSurf) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
				goto Exit;
			}

			//Copy the previous level to the temp texture
			if (m_fDXVersion < 9.0f)
			{
				if ( !CopyRects(pPreviousSurf, NULL, 0, pTempSurf, NULL) )
				{
					WriteToLog("RenderMipLevels - CopyRects() failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
					goto Exit;
				}
			}
			else
			{
				if ( !GetRenderTargetData(pPreviousSurf, pTempSurf) )
				{
					WriteToLog("RenderMipLevels - GetRenderTargetData() failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
					goto Exit;
				}
			}

			if ( !UpdateTexture(pTempTex, pTempRTTex) )
			{
				WriteToLog("RenderMipLevels - UpdateTexture(pTempTex, pTempRTTex) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
				goto Exit;
			}

			//Render the previous level on the current one
			SetTexture(0, pTempRTTex);

			// Set the current level surface as the render target
			if ( !SetRenderTarget(pRTSurf, NULL) )
			{
				WriteToLog("RenderMipLevels - SetRenderTarget(pRTSurf) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
				goto Exit;
			}

			//Render on the current texture mip level
			size = 1 << (8 - i);
			DrawTexture(0, 0, size);

			RELEASE(pRTSurf);
			RELEASE(pPreviousSurf);
			RELEASE(pTempSurf);
			RELEASE(pTempTex);
			RELEASE(pTempRTTex);
		}
	}

	bResult = true;

Exit:

	SetTexture(0, m_pTextures);

	// Set render target back to original
	if ( !SetRenderTarget(pOriginalRT, NULL) )
	{
		WriteToLog("RenderMipLevels - SetRenderTarget(pOriginalRT) failed with HResult = %s.\n",m_pD3D->HResultToString(GetLastError()));
		bResult = false;
	}

	// Cleanup surfaces
	RELEASE(pRTSurf);
	RELEASE(pPreviousSurf);
	RELEASE(pTempSurf);
	RELEASE(pTempTex);
	RELEASE(pTempRTTex);
	RELEASE(pOriginalRT);

	return bResult;
}

bool CMipGenEmuCubeTexture::RenderScene()
{
	return RenderCubeTextureScene();
}
