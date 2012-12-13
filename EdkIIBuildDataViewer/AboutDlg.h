/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// AboutDlg.h : header file

#pragma once

#include "resource.h"		// main symbols
// 3rd party code
#include "XColorStatic.h"

// CAboutDlg dialog used for App About
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	CStatic			m_appVersion;
	CStatic			m_copyright;
	CStatic			m_appName;
	CXColorStatic	m_acknowledgements;

	CMFCLinkCtrl	m_BsdLicenseLink;
	CMFCLinkCtrl	m_ack1Link;
	CMFCLinkCtrl	m_ack1LicenseLink;
	CMFCLinkCtrl	m_ack2Link;
	CMFCLinkCtrl	m_ack2LicenseLink;
	CMFCLinkCtrl	m_ack3Link;
	CMFCLinkCtrl	m_ack3LicenseLink;
	CMFCLinkCtrl	m_ack4Link;
	CMFCLinkCtrl	m_ack4LicenseLink;
	CMFCLinkCtrl	m_ack5Link;
	CMFCLinkCtrl	m_ack5LicenseLink;
	CMFCLinkCtrl	m_ack6Link;
	CMFCLinkCtrl	m_ack6LicenseLink;
};
