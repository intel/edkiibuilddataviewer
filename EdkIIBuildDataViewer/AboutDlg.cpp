/*++
  Copyright (c) 2012-2013, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// AboutDlg.cpp : implementation file

#include "stdafx.h"
#include "AboutDlg.h"
#include "GetVersionInfo.h"

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_APP_VERSION, m_appVersion);
	DDX_Control(pDX, IDC_STATIC_COPYRIGHT, m_copyright);
	DDX_Control(pDX, IDC_STATIC_APP_NAME, m_appName);
	DDX_Control(pDX, IDC_STATIC_ACK, m_acknowledgements);
	DDX_Control(pDX, IDC_BSD_LICENSE_LINK, m_BsdLicenseLink);
	DDX_Control(pDX, IDC_ACK1_LINK, m_ack1Link);
	DDX_Control(pDX, IDC_ACK1_LICENSE_LINK, m_ack1LicenseLink);
	DDX_Control(pDX, IDC_ACK2_LINK, m_ack2Link);
	DDX_Control(pDX, IDC_ACK2_LICENSE_LINK, m_ack2LicenseLink);
	DDX_Control(pDX, IDC_ACK3_LINK, m_ack3Link);
	DDX_Control(pDX, IDC_ACK3_LICENSE_LINK, m_ack3LicenseLink);
	DDX_Control(pDX, IDC_ACK4_LINK, m_ack4Link);
	DDX_Control(pDX, IDC_ACK4_LICENSE_LINK, m_ack4LicenseLink);
	DDX_Control(pDX, IDC_ACK5_LINK, m_ack5Link);
	DDX_Control(pDX, IDC_ACK5_LICENSE_LINK, m_ack5LicenseLink);
	DDX_Control(pDX, IDC_ACK6_LINK, m_ack6Link);
	DDX_Control(pDX, IDC_ACK6_LICENSE_LINK, m_ack6LicenseLink);
	DDX_Control(pDX, IDC_ACK7_LINK, m_ack7Link);
	DDX_Control(pDX, IDC_ACK7_LICENSE_LINK, m_ack7LicenseLink);
	DDX_Control(pDX, IDC_ACK8_LINK, m_ack8Link);
	DDX_Control(pDX, IDC_ACK8_LICENSE_LINK, m_ack8LicenseLink);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

/*++
  Function: OnInitDialog

  Parameters: none

  Purpose: initializes About dialog

  Returns: none
--*/
BOOL CAboutDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_appName.SetWindowTextW(GetVersionInfo(_T("ProductName")));
	m_appVersion.SetWindowTextW(GetVersionInfo(_T("FileVersion")));
	m_copyright.SetWindowTextW(GetVersionInfo(_T("LegalCopyright")));
	m_acknowledgements.SetBold(TRUE, FALSE);
	m_acknowledgements.SetFont(_T("Arial"), 16, FALSE);	// set font last, or it won't be used because font is recreated each SetXXX() call

	m_BsdLicenseLink.SetURL(_T("http://opensource.org/licenses/bsd-license.php"));
	m_ack1Link.SetURL(_T("http://www.mimec.org/components/mfc"));
	m_ack1LicenseLink.SetURL(_T("http://doc.mimec.org/articles/mfc/mctree/index.html"));
	m_ack2Link.SetURL(_T("http://www.codeproject.com/Articles/1657/EasySize-Dialog-resizing-in-no-time"));
	m_ack2LicenseLink.SetURL(_T("http://www.codeproject.com/info/EULA.aspx"));
	m_ack3Link.SetURL(_T("http://www.codeproject.com/Articles/29016/XGroupBox-an-MFC-groupbox-control-to-display-text"));
	m_ack3LicenseLink.SetURL(_T("http://www.codeproject.com/info/EULA.aspx"));
	m_ack4Link.SetURL(_T("http://www.codeproject.com/Articles/5242/XColorStatic-a-colorizing-static-control"));
	m_ack4LicenseLink.SetURL(_T("http://www.codeproject.com/info/EULA.aspx"));
	m_ack5Link.SetURL(_T("http://www.codeproject.com/KB/miscctrl/XProgressWnd.aspx"));
	m_ack5LicenseLink.SetURL(_T("http://www.codeproject.com/info/EULA.aspx"));
	m_ack6Link.SetURL(_T("http://www.codeproject.com/Articles/1940/XGetopt-A-Unix-compatible-getopt-for-MFC-and-Win32"));
	m_ack6LicenseLink.SetURL(_T("http://www.codeproject.com/info/EULA.aspx"));
	m_ack7Link.SetURL(_T("http://www.codeproject.com/Articles/9865/XFolderDialog-a-folder-selection-dialog-based-on-C"));
	m_ack7LicenseLink.SetURL(_T("http://www.codeproject.com/info/EULA.aspx"));
	m_ack8Link.SetURL(_T("http://msdn.microsoft.com/en-us/library/windows/desktop/ms646985%28v=vs.85%29.aspx"));
	m_ack8LicenseLink.SetURL(_T("http://msdn.microsoft.com/en-us/cc300389.aspx#D"));

	return TRUE;  // return TRUE unless you set the focus to a control
}
