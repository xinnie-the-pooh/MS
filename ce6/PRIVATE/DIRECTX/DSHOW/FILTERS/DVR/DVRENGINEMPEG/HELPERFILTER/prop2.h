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
#include <atlbase.h>
#include <string>
#include <cprop.h>

#include "..\main\DvrInterfaces.h"

class NullIPProperties2 : public CBasePropertyPage
{

public:

	static CUnknown * WINAPI CreateInstance(LPUNKNOWN lpunk, HRESULT *phr);
	DECLARE_IUNKNOWN;

private:

	BOOL OnReceiveMessage(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam);

	HRESULT OnConnect(IUnknown *pUnknown);
	HRESULT OnDisconnect();

	NullIPProperties2(LPUNKNOWN lpunk, HRESULT *phr);

	HRESULT GetPointers();

	int         m_nIndex ;       // Index of the selected media type
	CComPtr<IPin> m_pInPin ;        // The upstream output pin connected to us
	CComPtr<IPin> m_pOutPin ;        // The downstream output pin connected to us

	HWND m_hWnd;
	HANDLE m_hThread;

//	CComQIPtr<IStreamBufferCapture> m_pSBC;
//	CComQIPtr<IFileSinkFilter> m_pFileSink;
	CComPtr<IBaseFilter> m_pDownstreamFilter;
	CComPtr<IBaseFilter> m_pUpstreamFilter;

	static DWORD WINAPI ThreadProc(void *p);

	int GetCurSeekPos();
	int GetCurRate();
	int SetRate(double dRate);
	int SetRecordingPath();
	int BeginTempRecording();
	int DeleteRecording();
	int DeleteOrphans();
	int ConvertToTemp();
	int MuteStream(int id);

	int Refresh();

	bool m_enabled[10];

	INullIPP    *m_pINullIPP;    // Null In Place property interface

};  // class NullIPProperties2

