/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// Registry.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"

/*++
  Function: ReadSettingsFromRegistry

  Parameters: none

  Purpose: reads app settings from registry

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::ReadSettingsFromRegistry()
{
	// Read Notepad++ registry key for where it is installed.
	// Notepad++ is launched when items are double-clicked. If it's not found in registry, then user must download and install.
// HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\App Paths\notepad++.exe\<default value>
	CRegKey		regKey;
	ULONG		i = 1024;
	CString		tempStr;
	if (regKey.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\EDKII_BIOS\\EDKIIBuildDataViewer"), KEY_READ) == ERROR_SUCCESS) {
		CString		exeStr, nameStr, switchStr;
		LONG		rv;

		i = 1024; // set size of string buffer; on return, variable is size of string read
		rv = regKey.QueryStringValue(_T("Editor"), exeStr.GetBuffer(i), &i);
		exeStr.ReleaseBuffer();
		i = 1024; // set size of string buffer; on return, variable is size of string read
		rv = regKey.QueryStringValue(_T("Name"), nameStr.GetBuffer(i), &i);
		nameStr.ReleaseBuffer();
		i = 1024; // set size of string buffer; on return, variable is size of string read
		rv = regKey.QueryStringValue(_T("Switches"), switchStr.GetBuffer(i), &i);
		switchStr.ReleaseBuffer();
		m_chooseEditorDlg.SetEditorData(exeStr, nameStr, switchStr);
		rv = regKey.Close();
	} else if (regKey.Open(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\notepad++.exe"), KEY_READ) == ERROR_SUCCESS) {
		// if Notepad++ reg key found, then verify app location is present in registry
		// read default value by passing NULL string as key to read
		i = 1024;
		if (regKey.QueryStringValue(_T(""), tempStr.GetBuffer(i), &i) == ERROR_SUCCESS) {
			// Notepad++ default key found, which has app location; set this location in Editor dialog
			m_chooseEditorDlg.SetEditorData(tempStr, _T("Notepad++"), _T("-nLineNum"));
		} else {
			// Notepad++ default key not found, which has app location; default to Notepad location in Editor dialog
			m_chooseEditorDlg.SetEditorData(_T("%SystemRoot%\\System32\\Notepad.exe"), _T("Notepad"), _T(""));
		}
		tempStr.ReleaseBuffer();
		regKey.Close();
	} else {
		// if Notepad++ reg key not found, then default to Notepad location in Editor dialog
		m_chooseEditorDlg.SetEditorData(_T("%SystemRoot%\\System32\\Notepad.exe"), _T("Notepad"), _T(""));
	}
}


/*++
  Function: WriteSettingsToRegistry

  Parameters: none

  Purpose: writes app settings to registry

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::WriteSettingsToRegistry()
{
	CRegKey		regKey;
	BOOL		bWriteReg = FALSE;
	if (regKey.Open(HKEY_CURRENT_USER, _T("SOFTWARE\\EDKII_BIOS\\EDKIIBuildDataViewer"), KEY_WRITE) == ERROR_SUCCESS) {
		bWriteReg = TRUE;
	} else {
		if (regKey.Create(HKEY_CURRENT_USER, _T("SOFTWARE\\EDKII_BIOS\\EDKIIBuildDataViewer")) == ERROR_SUCCESS) {
			bWriteReg = TRUE;
		}
	}

	if (bWriteReg) {
		CString		exeStr, nameStr, switchStr;
		LONG		rv;

		// get editor settings from Editor dialog
		m_chooseEditorDlg.GetEditorData(exeStr, nameStr, switchStr);
		rv = regKey.SetStringValue(_T("Editor"), exeStr);
		rv = regKey.SetStringValue(_T("Name"), nameStr);
		rv = regKey.SetStringValue(_T("Switches"), switchStr);
		rv = regKey.Close();
	}
}
