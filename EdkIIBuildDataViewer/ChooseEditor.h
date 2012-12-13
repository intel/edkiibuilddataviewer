/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// ChooseEditor.h : header file

#pragma once

#include "afxlinkctrl.h"

#ifdef _WIN32_WCE
#error "CDHtmlDialog is not supported for Windows CE."
#endif 

// CChooseEditor dialog

class CChooseEditor : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CChooseEditor)

public:
	CChooseEditor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChooseEditor();
// Overrides
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);

// Dialog Data
	enum { IDD = IDD_EDITOR_DIALOG, IDH = IDR_HTML_EDITOR_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()

	CString			m_EditorExe;
	CString			m_EditorName;
	CString			m_EditorSwitches;
	CMFCLinkCtrl	m_NotepadPlusPlusLink;
	BOOL			m_bDefaultApp;
	BOOL			m_btmpDefaultApp;

	afx_msg void OnBnClickedChooseEditorExe();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedCheckDefaultAppShellexec();

public:
	void	GetEditorData(CString &strExe, CString &strName, CString &strSwitches);
	void	SetEditorData(CString strExe, CString strName, CString strSwitches);
};
