/*++
  Copyright (c) 2012-2013, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// EDKIIBuildDataViewer.cpp : Defines the class behaviors for the application.

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"
#include <fcntl.h>
#include <io.h>
#include <conio.h>

// if OUTPUT_USAGE_TO_CONSOLE is non-zero, then usage is output to console, else usage is output to dialog
#define OUTPUT_USAGE_TO_CONSOLE		1

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEDKIIBuildDataViewerApp

BEGIN_MESSAGE_MAP(CEDKIIBuildDataViewerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CEDKIIBuildDataViewerApp construction

CEDKIIBuildDataViewerApp::CEDKIIBuildDataViewerApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CEDKIIBuildDataViewerApp object

CEDKIIBuildDataViewerApp theApp;

///////////////////////////////////////////////////////////////////////////////
// ProcessCommandLine
//
// In non-dialog based app, this function would be in CWinApp module, and
// would be called from InitInstance() like this:
//          ProcessCommandLine(__argc, __argv);
//
BOOL CEDKIIBuildDataViewerApp::ProcessCommandLine(int argc, TCHAR *argv[])
{
#if OUTPUT_USAGE_TO_CONSOLE > 0
	FILE	*fh_stdout = NULL;
	BOOL	bDialog = (AfxGetMainWnd() != NULL);
	if (!bDialog) {
		AttachConsole(ATTACH_PARENT_PROCESS);
		int		hCrt = _open_osfhandle((intptr_t) GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
		if (hCrt != -1) {
			fh_stdout = _fdopen(hCrt, "w");
			FILE	*stdoutOld = stdout;
			*stdout = *fh_stdout;
			int		i = setvbuf(stdout, NULL, _IONBF, 0);
		}
	}
#endif

	CString		errorStr;

	// In the following loop you would set/unset any global command 
	// line flags and option arguments (usually in the CWinApp object) 
	// as each option was found in the command line.  
	//
	// In general it is probably best to let ProcessCommandLine's caller
	// sort out the command line arguments that were used, and whether 
	// they are consistent.  In ProcessCommandLine, you want to save the
	// options and the arguments, doing any conversion (atoi, etc.) that
	// is necessary.
	//
	// Normally you would have a case statement for each option letter.

	int c;
	while ((c = getopt(argc, argv, _T("l:s:dih"))) != EOF)
	{
		switch (c)
		{
			case _T('l'):
				{
					FILE	*fp;
					_tfopen_s(&fp, optarg, _T("rt"));
					if (fp == NULL) {
#if OUTPUT_USAGE_TO_CONSOLE == 0
						CString msg;
						msg.Format(_T("ERROR: build log %s couldn't be opened for read\n"), optarg);
						AfxMessageBox(msg, MB_ICONERROR);
#else
						wprintf(_T("\n\nERROR: build log %s couldn't be opened for read\n"), optarg);
#endif
						return FALSE;
					}

					fclose(fp);
					m_commandLineData.buildLog = optarg;
				}
				break;

			case _T('s'):
				{
					FILE	*fp;
					_tfopen_s(&fp, optarg, _T("wt"));
					if (fp == NULL) {
#if OUTPUT_USAGE_TO_CONSOLE == 0
						CString msg;
						msg.Format(_T("ERROR: source list file %s couldn't be opened for write\n"), optarg);
						AfxMessageBox(msg, MB_ICONERROR);
#else
						wprintf(_T("\n\nERROR: source list file %s couldn't be opened for write\n"), optarg);
#endif
						return FALSE;
					}

					fclose(fp);
					m_commandLineData.sourceFileList = optarg;
				}
				break;

			case _T('d'):
				m_commandLineData.bSourceUseDoxygenFormat = TRUE;
				break;

			case _T('i'):
				m_commandLineData.bSourceIncludeInf = TRUE;
				break;

			case _T('?'):
#if OUTPUT_USAGE_TO_CONSOLE == 0
				errorStr.Format(_T("ERROR: illegal option %s\n\n"), argv[optind-1]);
#else
				errorStr.Format(_T("\n\nERROR: illegal option %s\n\n"), argv[optind-1]);
#endif

			case _T('h'):
#if OUTPUT_USAGE_TO_CONSOLE == 0
				// output help to dialog
				{
					CString str;
					str.Format(_T("%s"), m_pszExeName);
					CString msg;
					if (!errorStr.IsEmpty())
						msg = errorStr;
					msg += _T("Usage: ") + str + _T(" -l buildLog [-s] sourceFileList [-d] [-i]\n");
					msg += _T("-l buildLog\tfilename for BIOS build log to open\n");
					msg += _T("-s sourceFileList\tfilename for source list file to create\n");
					msg += _T("-d\t\tsource list file, use doxygen format\n");
					msg += _T("-i\t\tsource list file, include INF files\n");
					msg += _T("\nApp errorlevel is 0 for ERROR, or 1 for SUCCESS\n");
					if (!errorStr.IsEmpty())
						AfxMessageBox(msg, MB_ICONERROR);
					else
						AfxMessageBox(msg, MB_ICONINFORMATION);
				}
#else
				// output help to console
				{
					if (!errorStr.IsEmpty())
						wprintf(_T("%s"), errorStr);
					wprintf(_T("Usage: %s -l buildLog [-s] sourceFileList [-d] [-i]\n"), m_pszExeName);
					wprintf(_T("-l buildLog        filename for BIOS build log to open\n"));
					wprintf(_T("-s sourceFileList  filename for source list file to create\n"));
					wprintf(_T("-d                 source list file, use doxygen format\n"));
					wprintf(_T("-i                 source list file, include INF files\n"));
					wprintf(_T("\nApp errorlevel is 0 for ERROR, or 1 for SUCCESS\n"));
				}
#endif
				return FALSE;
				break;
				
			default:
				break;
		}
	}

	// ensure all necessary command line parameters have been specified
	if (argc > 1 && (m_commandLineData.buildLog.IsEmpty() || m_commandLineData.sourceFileList.IsEmpty())) {
#if OUTPUT_USAGE_TO_CONSOLE == 0
		AfxMessageBox(_T("ERROR: must specify build log and source list filenames"), MB_ICONERROR);
#else
		wprintf(_T("\n\nERROR: must specify build log and source list filenames\n"));
#endif
		m_commandLineData.bCommandLineActive = FALSE;
		return FALSE;
	}
	// if command line parameters have been specified, then set flag to TRUE
	if (argc > 1)
		m_commandLineData.bCommandLineActive = TRUE;

#if OUTPUT_USAGE_TO_CONSOLE > 0
	if (fh_stdout != NULL) {
		fclose(fh_stdout);
		FreeConsole();
	}
#endif

	// all options processed, return success
	return TRUE;
}


// CEDKIIBuildDataViewerApp initialization
int CEDKIIBuildDataViewerApp::ExitInstance()
{
	return m_exitCode;
}

BOOL CEDKIIBuildDataViewerApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Create the shell manager, in case the dialog contains
	// any shell tree view or shell list view controls.
	CShellManager *pShellManager = new CShellManager;

	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	// Assume SUCCESS exit code of 1 on app exit.
	m_exitCode = 1;

	// Put EXE as first parameter to conform to ARGC/ARGV format.
	CString		cmdLine;
	cmdLine.Format(_T("%s %s"), m_pszExeName, m_lpCmdLine);
	// convert string to argv format
	int argc = _ConvertCommandLineToArgcArgv(cmdLine);
	if (argc > 1) {
		if (!ProcessCommandLine(argc, _ppszArgv))
			m_exitCode = 0;

		return FALSE;
	}

	CEDKIIBuildDataViewerDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Delete the shell manager created above.
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
//	AfxPostQuitMessage(1);
	return FALSE;
}
