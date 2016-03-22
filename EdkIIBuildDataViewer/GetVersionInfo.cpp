// Code located at http://msdn.microsoft.com/en-us/library/windows/desktop/ms646985%28v=vs.85%29.aspx
// License located at http://msdn.microsoft.com/en-us/cc300389.aspx#P
//
// modify build to link version.lib:
// Project -> Properties -> Linker -> Input -> Additional Dependencies -> version.lib

// GetVersionInfo.cpp : implementation file

#include "stdafx.h"

// KSA 2010/08/23 add function for get file info (version number & e.t.c)
/*
GetInfo(_T("Comments"));
GetInfo(_T("CompanyName"));
GetInfo(_T("FileDescription"));
GetInfo(_T("FileVersion"));
GetInfo(_T("InternalName"));
GetInfo(_T("LegalCopyright"));
GetInfo(_T("LegalTrademarks"));
GetInfo(_T("OriginalFilename"));
GetInfo(_T("PrivateBuild"));
GetInfo(_T("ProductName"));
GetInfo(_T("ProductVersion"));
*/
TCHAR *GetVersionInfo(TCHAR *InfoItem)
{
	static TCHAR		szResult[256] = {0};
	TCHAR		szFullPath[256];
	TCHAR		szGetName[256];
	LPTSTR		lpVersion;        // String pointer to Item text
	DWORD		dwVerInfoSize;    // Size of version information block
	DWORD		dwVerHnd=0;       // An 'ignored' parameter, always '0'
	UINT		uVersionLen;
	BOOL		bRetCode;

	GetModuleFileName (NULL, szFullPath, sizeof(szFullPath));
	dwVerInfoSize = GetFileVersionInfoSize(szFullPath, &dwVerHnd);
	if (dwVerInfoSize) {
		LPSTR   lpstrVffInfo;
		HANDLE  hMem;
		hMem = GlobalAlloc(GMEM_MOVEABLE, dwVerInfoSize);
		lpstrVffInfo  =  (LPSTR)GlobalLock(hMem);
		GetFileVersionInfo(szFullPath, dwVerHnd, dwVerInfoSize, lpstrVffInfo);

		// Get a codepage from base_file_info_sctructure
		_tcscpy_s(szGetName, 256, _T("\\VarFileInfo\\Translation"));

		uVersionLen   = 0;
		lpVersion     = NULL;
		bRetCode = VerQueryValue((LPVOID)lpstrVffInfo, (LPTSTR)szGetName,
			(void **)&lpVersion, (UINT *)&uVersionLen);
		if ( bRetCode && uVersionLen && lpVersion) {
			_stprintf_s(szResult, _T("%04x%04x"), (WORD)(*((DWORD *)lpVersion)),
				(WORD)(*((DWORD *)lpVersion)>>16));
//            lstrcpy(szResult, lpVersion);
		} else {
            // 041904b0 is a very common one, because it means:
            //   US English/Russia, Windows MultiLingual characterset
            // Or to pull it all apart:
            // 04------        = SUBLANG_ENGLISH_USA
            // --09----        = LANG_ENGLISH
            // --19----        = LANG_RUSSIA
            // ----04b0 = 1200 = Codepage for Windows:Multilingual
			_tcscpy_s(szResult, 256, _T("041904b0"));
    }

		// Add a codepage to base_file_info_sctructure
		_stprintf_s (szGetName, _T("\\StringFileInfo\\%s\\"), szResult);
		// Get a specific item
		_tcscat_s (szGetName, InfoItem);

		uVersionLen   = 0;
		lpVersion     = NULL;
		bRetCode = VerQueryValue((LPVOID)lpstrVffInfo, (LPTSTR)szGetName,
			(void **)&lpVersion, (UINT *)&uVersionLen);
		if ( bRetCode && uVersionLen && lpVersion) {
			_tcscpy_s(szResult, 256, lpVersion);
        } else {
			_tcscpy_s(szResult, 256, _T(""));
		}
	}
	
	return szResult;
}
