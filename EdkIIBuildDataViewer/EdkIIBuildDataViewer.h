/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// EDKIIBuildDataViewer.h : main header file for the PROJECT_NAME application

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CEDKIIBuildDataViewerApp:
// See EDKIIBuildDataViewer.cpp for the implementation of this class
//

// see EdkIIBuildDataViewer.cpp ProcessCommandLine() for command line switches and usage
typedef struct _COMMAND_LINE_DATA {
	BOOL		bCommandLineActive;
	CString		buildLog;			// input
	CString		sourceFileList;		// output
	BOOL		bSourceUseDoxygenFormat;
	BOOL		bSourceIncludeInf;
} COMMAND_LINE_DATA;

class CEDKIIBuildDataViewerApp : public CWinApp
{
public:
	CEDKIIBuildDataViewerApp();

// Overrides
public:
	virtual BOOL		InitInstance();
	virtual int			ExitInstance();

	COMMAND_LINE_DATA	m_commandLineData;
	int					m_exitCode;

	BOOL				ProcessCommandLine(int argc, TCHAR *argv[]);

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CEDKIIBuildDataViewerApp theApp;