// Code located at http://www.codeproject.com/Articles/29016/XGroupBox-an-MFC-groupbox-control-to-display-text
// License located at http://www.codeproject.com/info/EULA.aspx

// XVisualStyles.h  Version 1.0
// http://www.codeproject.com/Articles/29016/XGroupBox-an-MFC-groupbox-control-to-display-text
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// Adapted from code written by David Yuheng Zhao (yuheng_zhao@yahoo.com).
// 
// No warrantee of any kind, express or implied, is included with this
// software; use at your own risk, responsibility for damages (if any) to
// anyone resulting from the use of this software rests entirely with the
// user.
//
// Partly based on the _ThemeHelper struct in MFC7.0 source code (winctrl3.cpp), 
// and the difference is that this implementation wraps the full set of 
// visual style APIs from the platform SDK August 2001
//
// How to use:
// Instead of calling the API directly, 
//    OpenThemeData(...);
// use the global variable
//    g_xpStyle.OpenThemeData(...);
//
// CXVisualStyles Public Members:
//                NAME
//    ----------------------------
//    CXVisualStyles()
//    ~CXVisualStyles()
//    CloseThemeData()
//    DrawThemeBackground()
//    DrawThemeEdge()
//    DrawThemeParentBackground()
//    DrawThemeText()
//    EnableThemes()
//    GetCurrentThemeName()
//    GetThemeColor()
//    GetWindowTheme()
//    IsAppThemed()
//    IsThemeActive()
//    OpenThemeData()
//    SetThemeAppProperties()
//    SetWindowTheme( 
//    UseVisualStyles()
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XVISUALSTYLES_H
#define XVISUALSTYLES_H

#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <crtdbg.h>

//=============================================================================
// The following two files are from the Windows Platform SDK.  Search for 
// download here:
//     http://www.google.com/search?q=%22platform+sdk%22
// Note that the download location changes frequently, so it's best to let
// google find it for you.
//=============================================================================
#include <uxtheme.h>
//#include <tmschema.h>
#include <vssym32.h>
//#include "XTrace.h"

//=============================================================================
class CXVisualStyles
//=============================================================================
{
public:
	HMODULE m_hThemeDll;
	HTHEME  m_hTheme;
	operator HTHEME() { return m_hTheme; }

private:
	void* GetProc(LPCSTR szProc, void* pfnFail)
	{
		void* pRet = pfnFail;
		if (m_hThemeDll != NULL)
			pRet = GetProcAddress(m_hThemeDll, szProc);
		return pRet;
	}
	
	typedef HTHEME (__stdcall *PFNOPENTHEMEDATA)(HWND hwnd, 
		LPCWSTR pszClassList);
	static HTHEME OpenThemeDataFail(HWND, LPCWSTR)
	{return FALSE;}

	typedef HRESULT (__stdcall *PFNCLOSETHEMEDATA)(HTHEME hTheme);
	static HRESULT CloseThemeDataFail(HTHEME)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNDRAWTHEMEBACKGROUND)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pRect,  const RECT *pClipRect);
	static HRESULT DrawThemeBackgroundFail(HTHEME, HDC, int, int, const RECT *, 
		const RECT *)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNDRAWTHEMEPARENTBACKGROUND)(HWND hwnd, 
		HDC hdc,  RECT* prc);
	static HRESULT DrawThemeParentBackgroundFail(HWND /*hwnd*/, HDC /*hdc*/,  
		RECT* /*prc*/)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNDRAWTHEMETEXT)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, LPCWSTR pszText, int iCharCount, 
		DWORD dwTextFlags, DWORD dwTextFlags2, const RECT *pRect);
	static HRESULT DrawThemeTextFail(HTHEME, HDC, int, int, LPCWSTR, int, 
		DWORD, DWORD, const RECT*)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNDRAWTHEMEEDGE)(HTHEME hTheme, HDC hdc, 
		int iPartId, int iStateId, const RECT *pDestRect, UINT uEdge, 
		UINT uFlags, RECT *pContentRect);
	static HRESULT DrawThemeEdgeFail(HTHEME /*hTheme*/, HDC /*hdc*/, 
		int /*iPartId*/, int /*iStateId*/, const RECT * /*pDestRect*/, 
		UINT /*uEdge*/, UINT /*uFlags*/,   RECT * /*pContentRect*/)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETCURRENTTHEMENAME)(
		LPWSTR pszThemeFileName, int cchMaxNameChars, 
		LPWSTR pszColorBuff, int cchMaxColorChars,
		LPWSTR pszSizeBuff, int cchMaxSizeChars);
	static HRESULT GetCurrentThemeNameFail(
		LPWSTR /*pszThemeFileName*/, int /*cchMaxNameChars*/, 
		LPWSTR /*pszColorBuff*/, int /*cchMaxColorChars*/,
		LPWSTR /*pszSizeBuff*/, int /*cchMaxSizeChars*/)
	{return E_FAIL;}

	typedef HRESULT (__stdcall *PFNGETTHEMECOLOR)(HTHEME hTheme, int iPartId, 
		int iStateId, int iPropId,  COLORREF *pColor);
	static HRESULT GetThemeColorFail(HTHEME /*hTheme*/, int /*iPartId*/, 
		int /*iStateId*/, int /*iPropId*/,  COLORREF * /*pColor*/)
	{return E_FAIL;}

	typedef BOOL (__stdcall *PFNISTHEMEACTIVE)();
	static BOOL IsThemeActiveFail()
	{return FALSE;}

	typedef BOOL (__stdcall *PFNISAPPTHEMED)();
	static BOOL IsAppThemedFail()
	{return FALSE;}

	typedef HTHEME (__stdcall *PFNGETWINDOWTHEME)(HWND hwnd);
	static HTHEME GetWindowThemeFail(HWND /*hwnd*/)
	{return NULL;}

	typedef HRESULT (__stdcall *PFNSETWINDOWTHEME)(
		HWND hwnd, LPCWSTR pszSubAppName,
		LPCWSTR pszSubIdList);
	static HRESULT SetWindowThemeFail(
		HWND /*hwnd*/, LPCWSTR /*pszSubAppName*/,
		LPCWSTR /*pszSubIdList*/)
	{return E_FAIL;}

	typedef void (__stdcall *PFNSETTHEMEAPPPROPERTIES)(DWORD dwFlags);
	static void SetThemeAppPropertiesFail(DWORD /*dwFlags*/)
	{ }

public:
	CXVisualStyles()
	{
		TRACE(_T("in CXVisualStyles ======================================\n"));
		m_hThemeDll = LoadLibrary(_T("UxTheme.dll"));
		m_hTheme = NULL;
	}

	~CXVisualStyles()
	{
		TRACE(_T("in ~CXVisualStyles =====================================\n"));
		CloseThemeData();
		if (m_hThemeDll)
			FreeLibrary(m_hThemeDll);
		m_hThemeDll = NULL;
	}

	BOOL OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
	{
		//TRACE(_T("in OpenThemeData\n"));
		static PFNOPENTHEMEDATA pfn = NULL;
		if (!pfn)
			pfn = (PFNOPENTHEMEDATA)GetProc("OpenThemeData", 
					(void*)OpenThemeDataFail);
		m_hTheme = (*pfn)(hwnd, pszClassList);
		return (m_hTheme != NULL);
	}

	HRESULT CloseThemeData()
	{
		static PFNCLOSETHEMEDATA pfn = NULL;
		if (!pfn)
			pfn = (PFNCLOSETHEMEDATA)GetProc("CloseThemeData", 
					(void*)CloseThemeDataFail);
		HRESULT hr = S_OK;
		if (m_hTheme)
			hr = (*pfn)(m_hTheme);
		m_hTheme = NULL;
		return hr;
	}

	HRESULT DrawThemeBackground(//HTHEME hTheme, 
								HDC hdc, 
								int iPartId, 
								int iStateId, 
								const RECT *pRect, 
								const RECT *pClipRect)
	{
		_ASSERTE(m_hTheme);
		static PFNDRAWTHEMEBACKGROUND pfn = NULL;
		if (!pfn)
			pfn = (PFNDRAWTHEMEBACKGROUND)GetProc("DrawThemeBackground", 
					(void*)DrawThemeBackgroundFail);
		return (*pfn)(m_hTheme, hdc, iPartId, iStateId, pRect, 
					pClipRect);
	}

	HRESULT DrawThemeParentBackground(HWND hwnd, HDC hdc,  RECT* prc)
	{
		static PFNDRAWTHEMEPARENTBACKGROUND pfn = NULL;
		if (!pfn)
			pfn = (PFNDRAWTHEMEPARENTBACKGROUND)GetProc(
					"DrawThemeParentBackground", 
					(void*)DrawThemeParentBackgroundFail);
		return (*pfn)(hwnd, hdc, prc);
	}

	HRESULT DrawThemeText(//HTHEME hTheme, 
						  HDC hdc, 
						  int iPartId, 
						  int iStateId, 
						  LPCWSTR pszText, 
						  int iCharCount, 
						  DWORD dwTextFlags, 
						  DWORD dwTextFlags2, 
						  const RECT *pRect)
	{
		_ASSERTE(m_hTheme);
		static PFNDRAWTHEMETEXT pfn = NULL;
		if (!pfn)
			pfn = (PFNDRAWTHEMETEXT)GetProc("DrawThemeText", 
					(void*)DrawThemeTextFail);
		return (*pfn)(m_hTheme, hdc, iPartId, iStateId, pszText, iCharCount, 
					dwTextFlags, dwTextFlags2, pRect);
	}

	HRESULT DrawThemeEdge(//HTHEME hTheme, 
						  HDC hdc, 
						  int iPartId, 
						  int iStateId, 
						  const RECT *pDestRect, 
						  UINT uEdge, 
						  UINT uFlags,
						  RECT *pContentRect)
	{
		_ASSERTE(m_hTheme);
		static PFNDRAWTHEMEEDGE pfn = NULL;
		if (!pfn)
			pfn = (PFNDRAWTHEMEEDGE)GetProc("DrawThemeEdge", 
					(void*)DrawThemeEdgeFail);
		return (*pfn)(m_hTheme, hdc, iPartId, iStateId, pDestRect, uEdge, uFlags, 
					pContentRect);
	}

	HRESULT GetCurrentThemeName(LPWSTR pszThemeFileName, int cchMaxNameChars, 
								LPWSTR pszColorBuff, int cchMaxColorChars,
								LPWSTR pszSizeBuff, int cchMaxSizeChars)
	{
		static PFNGETCURRENTTHEMENAME pfn = NULL;
		if (!pfn)
			pfn = (PFNGETCURRENTTHEMENAME)GetProc("GetCurrentThemeName", 
					(void*)GetCurrentThemeNameFail);
		return (*pfn)(pszThemeFileName, cchMaxNameChars, pszColorBuff, 
					cchMaxColorChars, pszSizeBuff, cchMaxSizeChars);
	}

	HRESULT GetThemeColor(//HTHEME hTheme, 
						  int iPartId, 
						  int iStateId, int iPropId,  COLORREF *pColor)
	{
		_ASSERTE(m_hTheme);
		static PFNGETTHEMECOLOR pfn = NULL;
		if (!pfn)
			pfn = (PFNGETTHEMECOLOR)GetProc("GetThemeColor", 
					(void*)GetThemeColorFail);
		return (*pfn)(m_hTheme, iPartId, iStateId, iPropId, pColor);
	}

	BOOL IsThemeActive()
	{
		static PFNISTHEMEACTIVE pfn = NULL;
		if (!pfn)
			pfn = (PFNISTHEMEACTIVE)GetProc("IsThemeActive", 
					(void*)IsThemeActiveFail);
		return (*pfn)();
	}

	BOOL IsAppThemed()
	{
		static PFNISAPPTHEMED pfn = NULL;
		if (!pfn)
			pfn = (PFNISAPPTHEMED)GetProc("IsAppThemed", 
					(void*)IsAppThemedFail);
		return (*pfn)();
	}

	HTHEME GetWindowTheme(HWND hwnd)
	{
		static PFNGETWINDOWTHEME pfn = NULL;
		if (!pfn)
			pfn = (PFNGETWINDOWTHEME)GetProc("GetWindowTheme", 
					(void*)GetWindowThemeFail);
		return (*pfn)(hwnd);
	}

	HRESULT SetWindowTheme(HWND hwnd, LPCWSTR pszSubAppName, 
				LPCWSTR pszSubIdList)
	{
		static PFNSETWINDOWTHEME pfn = NULL;
		if (!pfn)
			pfn = (PFNSETWINDOWTHEME)GetProc("SetWindowTheme", 
					(void*)SetWindowThemeFail);
		return (*pfn)(hwnd, pszSubAppName, pszSubIdList);
	}

	void SetThemeAppProperties(DWORD dwFlags)
	{
		static PFNSETTHEMEAPPPROPERTIES pfn = NULL;
		if (!pfn)
			pfn = (PFNSETTHEMEAPPPROPERTIES)GetProc("SetThemeAppProperties", 
					(void*)SetThemeAppPropertiesFail);
		(*pfn)(dwFlags);
	}

	BOOL UseVisualStyles()
	{
		static BOOL bUse = -1;
		if (bUse != -1)
			return bUse;

		bUse = FALSE;

		DLLVERSIONINFO dvi;
		ZeroMemory(&dvi, sizeof(dvi));

		HINSTANCE hinstDll = LoadLibrary(_T("comctl32.dll"));
		if (hinstDll)
		{
			DLLGETVERSIONPROC pDllGetVersion;
			pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, 
								"DllGetVersion");
			if (pDllGetVersion)
			{
				dvi.cbSize = sizeof(dvi);
				(*pDllGetVersion)(&dvi);
			}
			FreeLibrary(hinstDll);
		}

		if (dvi.dwMajorVersion >= 6)
		{
			bUse = IsAppThemed() && m_hThemeDll;
		}

		return bUse;
	}

	// enable theme for child windows
	void EnableThemes(HWND hParent, BOOL bEnable, BOOL bEnableParent)
	{
		if (bEnableParent)
		{
			if (bEnable)
				SetWindowTheme(hParent, 0, 0);
			else
				SetWindowTheme(hParent, L"", L"");
		}

		HWND hWndChild = ::GetWindow(hParent, GW_CHILD);

		while (hWndChild)
		{
			if (bEnable)
				SetWindowTheme(hWndChild, 0, 0);
			else
				SetWindowTheme(hWndChild, L"", L"");

			// get next child control
			hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
		}
	}
};

#endif //XVISUALSTYLES_H
