// This is a part of the Active Template Library.
// Copyright (C) 1996-1998 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Active Template Library Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Active Template Library product.

#ifndef __ATLDDX_H__
#define __ATLDDX_H__

#pragma once

#ifndef _ATL_NO_FLOAT_DDX
#include <float.h>
#ifndef _DEBUG
#include <stdio.h>
#endif //!_DEBUG
#endif //!_ATL_NO_FLOAT_DDX

namespace ATL
{

// Constants
#define DDX_LOAD	FALSE
#define DDX_SAVE	TRUE

// DDX map macros
#define BEGIN_DDX_MAP(thisClass) \
	BOOL DoDataExchange(BOOL bSaveAndValidate = FALSE, UINT nCtlID = (UINT)-1) \
	{ \

#define DDX_TEXT(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Text(nID, var, sizeof(var), bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_TEXT_LEN(nID, var, len) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Text(nID, var, sizeof(var), bSaveAndValidate, TRUE, len)) \
				return FALSE; \
		}

#define DDX_INT(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Int(nID, var, TRUE, bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_INT_RANGE(nID, var, min, max) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Int(nID, var, TRUE, bSaveAndValidate, TRUE, min, max)) \
				return FALSE; \
		}

#define DDX_UINT(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Int(nID, var, FALSE, bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_UINT_RANGE(nID, var, min, max) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Int(nID, var, FALSE, bSaveAndValidate, TRUE, min, max)) \
				return FALSE; \
		}

#ifndef _ATL_NO_FLOAT_DDX
#define DDX_FLOAT(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Float(nID, var, bSaveAndValidate)) \
				return FALSE; \
		}

#define DDX_FLOAT_RANGE(nID, var, min, max) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
		{ \
			if(!DDX_Float(nID, var, bSaveAndValidate, TRUE, min, max)) \
				return FALSE; \
		}
#endif //!_ATL_NO_FLOAT_DDX

#define DDX_CONTROL(nID, obj) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			DDX_Control(nID, obj, bSaveAndValidate);

#define DDX_CHECK(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			DDX_Check(nID, var, bSaveAndValidate);

#define DDX_RADIO(nID, var) \
		if(nCtlID == (UINT)-1 || nCtlID == nID) \
			DDX_Radio(nID, var, bSaveAndValidate);

#define END_DDX_MAP() \
		return TRUE; \
	}


/////////////////////////////////////////////////////////////////////////////
// CWinDataExchange - provides support for DDX

template <class T>
class CWinDataExchange
{
public:
// Data exchange method - override in your derived class
	BOOL DoDataExchange(BOOL /*bSaveAndValidate*/ = FALSE, UINT /*nCtlID*/ = (UINT)-1)
	{
		// this one should never be called, override it in
		// your derived class by implementing DDX map
		ATLASSERT(FALSE);
		return FALSE;
	}

// Helpers for validation error reporting
	enum _XDataType
	{
		ddxDataNull = 0,
		ddxDataText = 1,
		ddxDataInt = 2,
		ddxDataFloat = 3,
		ddxDataDouble = 4
	};

	struct _XTextData
	{
		int nLength;
		int nMaxLength;
	};

	struct _XIntData
	{
		long nVal;
		long nMin;
		long nMax;
	};

	struct _XFloatData
	{
		double nVal;
		double nMin;
		double nMax;
	};

	struct _XData
	{
		_XDataType nDataType;
		union
		{
			_XTextData textData;
			_XIntData intData;
			_XFloatData floatData;
		};
	};

// Text exchange
	BOOL DDX_Text(UINT nID, LPTSTR lpstrText, int nSize, BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			HWND hWndCtrl = pT->GetDlgItem(nID);
			int nRetLen = ::GetWindowText(hWndCtrl, lpstrText, nSize);
			if(nRetLen < ::GetWindowTextLength(hWndCtrl))
				bSuccess = FALSE;
		}
		else
		{
			ATLASSERT(!bValidate || lstrlen(lpstrText) <= nLength);
			bSuccess = pT->SetDlgItemText(nID, lpstrText);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
			ATLASSERT(nLength > 0);
			if(lstrlen(lpstrText) > nLength)
			{
				_XData data;
				data.nDataType = ddxDataText;
				data.textData.nLength = lstrlen(lpstrText);
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

	BOOL DDX_Text(UINT nID, BSTR& bstrText, int /*nSize*/, BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			bSuccess = pT->GetDlgItemText(nID, bstrText);
		}
		else
		{
			USES_CONVERSION;
			LPTSTR lpstrText = OLE2T(bstrText);
			ATLASSERT(!bValidate || lstrlen(lpstrText) <= nLength);
			bSuccess = pT->SetDlgItemText(nID, lpstrText);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
			ATLASSERT(nLength > 0);
			if((int)::SysStringLen(bstrText) > nLength)
			{
				_XData data;
				data.nDataType = ddxDataText;
				data.textData.nLength = (int)::SysStringLen(bstrText);
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

	BOOL DDX_Text(UINT nID, CComBSTR& bstrText, int /*nSize*/, BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			bSuccess = pT->GetDlgItemText(nID, (BSTR&)bstrText);
		}
		else
		{
			USES_CONVERSION;
			LPTSTR lpstrText = OLE2T(bstrText);
			ATLASSERT(!bValidate || lstrlen(lpstrText) <= nLength);
			bSuccess = pT->SetDlgItemText(nID, lpstrText);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
			ATLASSERT(nLength > 0);
			if((int)bstrText.Length() > nLength)
			{
				_XData data;
				data.nDataType = ddxDataText;
				data.textData.nLength = (int)bstrText.Length();
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

#ifdef __ATLSTR_H__
	BOOL DDX_Text(UINT nID, CString& strText, int /*nSize*/, BOOL bSave, BOOL bValidate = FALSE, int nLength = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			HWND hWndCtrl = pT->GetDlgItem(nID);
			int nLen = ::GetWindowTextLength(hWndCtrl);
			int nRetLen = ::GetWindowText(hWndCtrl, strText.GetBufferSetLength(nLen), nLen + 1);
			if(nRetLen < nLen)
				bSuccess = FALSE;
			strText.ReleaseBuffer();
		}
		else
		{
			bSuccess = pT->SetDlgItemText(nID, strText);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
			ATLASSERT(nLength > 0);
			if(strText.GetLength() > nLength)
			{
				_XData data;
				data.nDataType = ddxDataText;
				data.textData.nLength = strText.GetLength();
				data.textData.nMaxLength = nLength;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}
#endif //__ATLSTR_H__

// Numeric exchange
	template <class Type>
	BOOL DDX_Int(UINT nID, Type& nVal, BOOL bSigned, BOOL bSave, BOOL bValidate = FALSE, Type nMin = 0, Type nMax = 0)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;

		if(bSave)
		{
			nVal = (Type)pT->GetDlgItemInt(nID, &bSuccess, bSigned);
		}
		else
		{
			ATLASSERT(!bValidate || nVal >= nMin && nVal <= nMax);
			bSuccess = pT->SetDlgItemInt(nID, nVal, bSigned);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
			ATLASSERT(nMin != nMax);
			if(nVal < nMin || nVal > nMax)
			{
				_XData data;
				data.nDataType = ddxDataInt;
				data.intData.nVal = (long)nVal;
				data.intData.nMin = (long)nMin;
				data.intData.nMax = (long)nMax;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

// Float exchange
#ifndef _ATL_NO_FLOAT_DDX
	static BOOL _AtlSimpleFloatParse(LPCTSTR lpszText, double& d)
	{
		ATLASSERT(lpszText != NULL);
		while (*lpszText == ' ' || *lpszText == '\t')
			lpszText++;

		TCHAR chFirst = lpszText[0];
		d = _tcstod(lpszText, (LPTSTR*)&lpszText);
		if (d == 0.0 && chFirst != '0')
			return FALSE;   // could not convert
		while (*lpszText == ' ' || *lpszText == '\t')
			lpszText++;

		if (*lpszText != '\0')
			return FALSE;   // not terminated properly

		return TRUE;
	}

	BOOL DDX_Float(UINT nID, float& nVal, BOOL bSave, BOOL bValidate = FALSE, float nMin = 0.F, float nMax = 0.F)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;
		TCHAR szBuff[32];

		if(bSave)
		{
			pT->GetDlgItemText(nID, szBuff, sizeof(szBuff) / sizeof(TCHAR));
			double d = 0;
			if(_AtlSimpleFloatParse(szBuff, d))
				nVal = (float)d;
			else
				bSuccess = FALSE;
		}
		else
		{
			ATLASSERT(!bValidate || nVal >= nMin && nVal <= nMax);
			_stprintf(szBuff, _T("%.*g"), FLT_DIG, nVal);
			bSuccess = pT->SetDlgItemText(nID, szBuff);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
			ATLASSERT(nMin != nMax);
			if(nVal < nMin || nVal > nMax)
			{
				_XData data;
				data.nDataType = ddxDataFloat;
				data.floatData.nVal = (double)nVal;
				data.floatData.nMin = (double)nMin;
				data.floatData.nMax = (double)nMax;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}

	BOOL DDX_Float(UINT nID, double& nVal, BOOL bSave, BOOL bValidate = FALSE, double nMin = 0., double nMax = 0.)
	{
		T* pT = static_cast<T*>(this);
		BOOL bSuccess = TRUE;
		TCHAR szBuff[32];

		if(bSave)
		{
			pT->GetDlgItemText(nID, szBuff, sizeof(szBuff) / sizeof(TCHAR));
			double d = 0;
			if(_AtlSimpleFloatParse(szBuff, d))
				nVal = d;
			else
				bSuccess = FALSE;
		}
		else
		{
			ATLASSERT(!bValidate || nVal >= nMin && nVal <= nMax);
			_stprintf(szBuff, _T("%.*g"), DBL_DIG, nVal);
			bSuccess = pT->SetDlgItemText(nID, szBuff);
		}

		if(!bSuccess)
		{
			pT->OnDataExchangeError(nID, bSave);
		}
		else if(bSave && bValidate)	// validation
		{
			ATLASSERT(nMin != nMax);
			if(nVal < nMin || nVal > nMax)
			{
				_XData data;
				data.nDataType = ddxDataFloat;
				data.floatData.nVal = nVal;
				data.floatData.nMin = nMin;
				data.floatData.nMax = nMax;
				pT->OnDataValidateError(nID, bSave, data);
				bSuccess = FALSE;
			}
		}
		return bSuccess;
	}
#endif //!_ATL_NO_FLOAT_DDX

// Control subclassing
	template <class TControl>
	void DDX_Control(UINT nID, TControl& ctrl, BOOL bSave)
	{
		T* pT = static_cast<T*>(this);
		if(!bSave && ctrl.m_hWnd == NULL)
			ctrl.SubclassWindow(pT->GetDlgItem(nID));
	}

// Control state
	void DDX_Check(UINT nID, int& nValue, BOOL bSave)
	{
		T* pT = static_cast<T*>(this);
		HWND hWndCtrl = pT->GetDlgItem(nID);
		if(bSave)
		{
			nValue = (int)::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L);
			ATLASSERT(nValue >= 0 && nValue <= 2);
		}
		else
		{
			if(nValue < 0 || nValue > 2)
			{
				ATLTRACE(atlTraceWindowing, 2, _T("ATL: Warning - dialog data checkbox value (%d) out of range.\n"), nValue);
				nValue = 0;  // default to off
			}
			::SendMessage(hWndCtrl, BM_SETCHECK, nValue, 0L);
		}
	}

	void DDX_Radio(UINT nID, int& nValue, BOOL bSave)
	{
		T* pT = static_cast<T*>(this);
		HWND hWndCtrl = pT->GetDlgItem(nID);
		ATLASSERT(hWndCtrl != NULL);

		// must be first in a group of auto radio buttons
		ATLASSERT(::GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP);
		ATLASSERT(::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON);

		if(bSave)
			nValue = -1;     // value if none found

		// walk all children in group
		int nButton = 0;
		do
		{
			if(::SendMessage(hWndCtrl, WM_GETDLGCODE, 0, 0L) & DLGC_RADIOBUTTON)
			{
				// control in group is a radio button
				if(bSave)
				{
					if(::SendMessage(hWndCtrl, BM_GETCHECK, 0, 0L) != 0)
					{
						ATLASSERT(nValue == -1);    // only set once
						nValue = nButton;
					}
				}
				else
				{
					// select button
					::SendMessage(hWndCtrl, BM_SETCHECK, (nButton == nValue), 0L);
				}
				nButton++;
			}
			else
			{
				ATLTRACE(atlTraceWindowing, 2, _T("ATL: Warning - skipping non-radio button in group.\n"));
			}
			hWndCtrl = ::GetWindow(hWndCtrl, GW_HWNDNEXT);
		}
		while (hWndCtrl != NULL && !(GetWindowLong(hWndCtrl, GWL_STYLE) & WS_GROUP));
	}

// Overrideables
	void OnDataExchangeError(UINT nCtrlID, BOOL /*bSave*/)
	{
		// Override to display an error message
		::MessageBeep((UINT)-1);
		T* pT = static_cast<T*>(this);
#if (_ATL_VER >= 0x0400)
		pT->GetDlgItem(nCtrlID).SetFocus();
#else
		CWindow wnd = pT->GetDlgItem(nCtrlID);
		wnd.SetFocus();
#endif //(_ATL_VER >= 0x0400)
	}

	void OnDataValidateError(UINT nCtrlID, BOOL /*bSave*/, _XData& /*data*/)
	{
		// Override to display an error message
		::MessageBeep((UINT)-1);
		T* pT = static_cast<T*>(this);
#if (_ATL_VER >= 0x0400)
		pT->GetDlgItem(nCtrlID).SetFocus();
#else
		CWindow wnd = pT->GetDlgItem(nCtrlID);
		wnd.SetFocus();
#endif //(_ATL_VER >= 0x0400)
	}
};

}; //namespace ATL

#endif //__ATLDDX_H__
