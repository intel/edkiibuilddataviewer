/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// ChooseEditor.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "ChooseEditor.h"

// CChooseEditor dialog

IMPLEMENT_DYNCREATE(CChooseEditor, CDHtmlDialog)

CChooseEditor::CChooseEditor(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CChooseEditor::IDD, CChooseEditor::IDH, pParent)
	, m_EditorExe(_T(""))
	, m_EditorName(_T(""))
	, m_EditorSwitches(_T(""))
	, m_bDefaultApp(FALSE)
	, m_btmpDefaultApp(FALSE)
{
}

CChooseEditor::~CChooseEditor()
{
}

void CChooseEditor::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDITOR_EXE, m_EditorExe);
	DDX_Text(pDX, IDC_EDITOR_NAME, m_EditorName);
	DDX_Text(pDX, IDC_EDITOR_SWITCHES, m_EditorSwitches);
	DDX_Control(pDX, IDC_NPP_LINK, m_NotepadPlusPlusLink);
	DDX_Check(pDX, IDC_CHECK_DEFAULT_APP_SHELLEXEC, m_bDefaultApp);
}


BEGIN_MESSAGE_MAP(CChooseEditor, CDHtmlDialog)
	ON_BN_CLICKED(IDOK, &CChooseEditor::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CChooseEditor::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_CHOOSE_EDITOR_EXE, &CChooseEditor::OnBnClickedChooseEditorExe)
	ON_BN_CLICKED(IDC_CHECK_DEFAULT_APP_SHELLEXEC, &CChooseEditor::OnBnClickedCheckDefaultAppShellexec)
END_MESSAGE_MAP()


BEGIN_DHTML_EVENT_MAP(CChooseEditor)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()


// CChooseEditor message handlers

BOOL CChooseEditor::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();

	m_NotepadPlusPlusLink.SetURL(_T("http://notepad-plus-plus.org/"));
	OnBnClickedCheckDefaultAppShellexec();

	return TRUE;  // return TRUE  unless you set the focus to a control
}


void CChooseEditor::OnBnClickedChooseEditorExe()
{
	CFileDialog		fd(TRUE, L"*.exe", NULL, OFN_DONTADDTORECENT | OFN_ENABLESIZING, _T("Executable files (*.exe)|*.exe||"));

	fd.m_ofn.lpstrTitle = _T("Select editor executable");
	if (fd.DoModal() == IDOK) {
		m_EditorExe = fd.GetPathName();
		UpdateData(FALSE);
	}
}


void CChooseEditor::GetEditorData(CString &strExe, CString &strName, CString &strSwitches)
{
	if (!m_bDefaultApp) {
		strExe = m_EditorExe;
		strName = m_EditorName;
		strSwitches = m_EditorSwitches;
	} else {
		strExe = _T("");
		strName = _T("default app for file extension");
		strSwitches = _T("");
	}
}


void CChooseEditor::SetEditorData(CString strExe, CString strName, CString strSwitches)
{
	if (!m_bDefaultApp) {
		m_EditorExe = strExe;
		m_EditorName = strName;
		m_EditorSwitches = strSwitches;
	} else {
		m_EditorExe = _T("");
		m_EditorName = _T("");
		m_EditorSwitches = _T("");
	}
}


HRESULT CChooseEditor::OnButtonOK(IHTMLElement* /*pElement*/)
{
	UpdateData(TRUE);

	CDHtmlDialog::OnOK();
	return S_OK;
}

HRESULT CChooseEditor::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	CDHtmlDialog::OnCancel();
	return S_OK;
}


void CChooseEditor::OnBnClickedOk()
{
	UpdateData(TRUE);
	m_btmpDefaultApp = m_bDefaultApp;

	CDHtmlDialog::OnOK();
}


void CChooseEditor::OnBnClickedCancel()
{
	m_bDefaultApp = m_btmpDefaultApp;
	CDHtmlDialog::OnCancel();
}


void CChooseEditor::OnBnClickedCheckDefaultAppShellexec()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_CHOOSE_EDITOR_EXE)->EnableWindow(!m_bDefaultApp);
	((CEdit *) GetDlgItem(IDC_EDITOR_NAME))->SetReadOnly(m_bDefaultApp);
	((CEdit *) GetDlgItem(IDC_EDITOR_SWITCHES))->SetReadOnly(m_bDefaultApp);
}
