/*++
  Copyright (c) 2012-2013, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// LoadBuildLog.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"

/*++
  Function: OnBnClickedSelectBuildLog

  Parameters: none

  Purpose: loads build log

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::OnBnClickedSelectBuildLog()
{
	CFileDialog		fd(TRUE, L"*.log", NULL, OFN_DONTADDTORECENT | OFN_ENABLESIZING, _T("Log files (*.log)|*.log|All files (*.*)|*.*||"));

	fd.m_ofn.lpstrTitle = _T("Select EDK II BIOS build log file");
	if (fd.DoModal() == IDOK) {
		CStdioFile		csf, csf2;
		BOOL			bBuildCfgFound = FALSE;
		CString			resToken, resToken2;
		int				curPos, i, j, lineNum = 0;
		CString			fileStr;
		CString			tempStr, tempStr2;
		LVITEM			lvi;

		// init build data before reading from files
		InitBuildData();

// *** error check and return
		if (!csf.Open(fd.GetPathName(), CFile::modeRead | CFile::typeText)) {
			MessageBox(_T("Cannot open file!"), _T("ERROR"), MB_ICONERROR);
			return;
		}

		if (m_pwndProgress != NULL)
			delete m_pwndProgress;
		m_pwndProgress = new CXProgressWnd(this, _T("Parsing build log file"), FALSE, FALSE);
		m_pwndProgress->GoModal(this);

		m_buildLog = fd.GetPathName();
		// Replace backslash with slash so code that searches paths can expect one format to find.
		m_buildLog.Replace(_T('\\'), _T('/'));
		GetDlgItem(IDC_EDIT_BUILD_LOG)->SetWindowTextW(m_buildLog);
		m_fileInf = m_buildLog;

		// tokenize string, search for Build Configuration data
#define BUILD_CFG_MAX				4
#define BUILD_CFG_ALT_MAX			4
#define BUILD_CFG_TYPE_INIT			-1
#define BUILD_CFG_TYPE				0
#define BUILD_CFG_TYPE_ALT			1
#define BUILD_CFG_IDC_WORKSPACE		0
		TCHAR		findBuildCfg[BUILD_CFG_MAX][32] = {_T("WORKSPACE"), _T("TARGET_ARCH"), _T("TARGET"), _T("TOOL_CHAIN_TAG")};
		UINT32		idBuildCfg[BUILD_CFG_MAX] = {IDC_STATIC_WORKSPACE, IDC_STATIC_TARGET_ARCH, IDC_STATIC_TARGET, IDC_STATIC_TOOL_CHAIN};
		TCHAR		findBuildCfgAlt[BUILD_CFG_ALT_MAX][32] = {_T("WORKSPACE"), _T("Architecture(s)"), _T("Build target"), _T("Toolchain")};
		UINT32		idBuildCfgAlt[BUILD_CFG_ALT_MAX] = {IDC_STATIC_WORKSPACE, IDC_STATIC_TARGET_ARCH, IDC_STATIC_TARGET, IDC_STATIC_TOOL_CHAIN};
		int			nBuildCfgType = BUILD_CFG_TYPE_INIT; // must be initialized to init value
		int			nExpectedBuildCfgCount = max(BUILD_CFG_MAX, BUILD_CFG_ALT_MAX); // must be initialized to max of the 2 values

		// loop until all build cfg items are found
		m_pwndProgress->SetText(_T("Detecting build configuration ..."));
		m_pwndProgress->Show();
		i = 0;
		while (i < nExpectedBuildCfgCount) {
			// if EOF then break
			if (!csf.ReadString(fileStr))
				break;

			lineNum++;

			if (lineNum == 1) {
				if (fileStr.Find(_T("Build environment:")) == 0)
					continue;
				else {
					MessageBox(_T("Invalid log file!"), _T("ERROR"),  MB_ICONERROR);
					break;
				}
			} else if (lineNum == 2) {
				if (fileStr.Find(_T("Build start time:")) == 0)
					continue;
				else {
					MessageBox(_T("Invalid log file!"), _T("ERROR"),  MB_ICONERROR);
					break;
				}
			}

			// tokenize the string
			curPos = 0;
  			resToken = fileStr.Tokenize(_T("="), curPos);
			// is a token found?
			if (resToken != _T("")) {
				// does token match a BuildCfg item?
				resToken.Trim(_T("\t "));
				if ((nBuildCfgType == BUILD_CFG_TYPE_INIT && ((i < BUILD_CFG_MAX && resToken.CompareNoCase(findBuildCfg[i]) == 0) || (i < BUILD_CFG_ALT_MAX && resToken.CompareNoCase(findBuildCfgAlt[i]) == 0))) ||
						(nBuildCfgType == BUILD_CFG_TYPE && i < BUILD_CFG_MAX && resToken.CompareNoCase(findBuildCfg[i]) == 0) ||
						(nBuildCfgType == BUILD_CFG_TYPE_ALT && i < BUILD_CFG_ALT_MAX && resToken.CompareNoCase(findBuildCfgAlt[i]) == 0)) {
					// check build cfg types and set type based on string set used for comparison
					// if type=INIT, and compare matches 1 set and not the other ...
					if (nBuildCfgType == BUILD_CFG_TYPE_INIT && resToken.CompareNoCase(findBuildCfg[i]) == 0 && resToken.CompareNoCase(findBuildCfg[i]) != 0) {
						nBuildCfgType = BUILD_CFG_TYPE;
						nExpectedBuildCfgCount = BUILD_CFG_MAX;
					} else if (nBuildCfgType == BUILD_CFG_TYPE_INIT && resToken.CompareNoCase(findBuildCfgAlt[i]) == 0 && resToken.CompareNoCase(findBuildCfg[i]) != 0) {
						nBuildCfgType = BUILD_CFG_TYPE_ALT;
						nExpectedBuildCfgCount = BUILD_CFG_ALT_MAX;
					}
					// get token on right of =
		 			resToken = fileStr.Tokenize(_T("="), curPos);
					// is a token found?
					if (resToken != _T("")) {
						// remove whitespace on left and right of string
						resToken.Trim();
						// write string to control var
						GetDlgItem(idBuildCfg[i])->SetWindowTextW(resToken);
						i++;
					}
				}
			}
		}

// *** error check and return
		// all build cfg items not found, so return
		if (i != nExpectedBuildCfgCount) {
			delete m_pwndProgress;
			m_pwndProgress = NULL;
			InitBuildData();
			return;
		}

		// update variables associated with control IDs due to SetWindowTextW() calls
		UpdateData(TRUE);
		// clear all checks because it is not known what sections are present in the report
		InitBuildReportData();

		// Workspace may end with / or \ (e.g. C:\), or it may not (e.g. C:\xyz) in build log.
		// Force Workspace to end with / or \ (whichever is reverse first found with reverse find)
		// so removing Workspace from file paths is 1 line of code based on string length.
		// This also makes it easier to form file paths because we know workspace ends with / or \.
		//
		// Get last character from Workspace string e.g. C:\ or C:\xyz
		TCHAR	tch = m_workspace.GetAt(m_workspace.GetLength() - 1);
		if (tch != _T('/') && tch != _T('\\')) {
			// If reverse find encounters /, then append / to workspace.
			if (m_workspace.ReverseFind(_T('/')) != -1)
				m_workspace += _T('/');
			// Else append \ to workspace.
			else
				m_workspace += _T('\\');

			UpdateData(FALSE);  // save m_workspace value
		}

		// search for .INF files
		m_pwndProgress->SetText(_T("Finding .INF files used in build ..."));
		m_pwndProgress->Show();

		CString	archStr, findStr1, findStr2, findStr3;

// *** START: find .INF files in build log
		// find .INF files used in build by looking for "Building ... <workspace>" followed by ".inf ["
		// create strings to match
		findStr1 = _T("Building ... ") + m_workspace;
		findStr2 = _T("inf [");
		findStr3 = _T("GenFds ");
		while (csf.ReadString(fileStr)) {
			// find string to match, and ".inf [", which indicates .INF file
			int findIndex1 = fileStr.Find(findStr1);
			int findIndex2 = fileStr.Find(findStr2);
//			if (findIndex1 == 0 && findIndex2 != -1) {
			// "Building ... " should be at beginning of string (index 0)
			// workaround for build log bug: check find index to not be -1, and then remove characters from index 0 to the find index
			// 1. sometimes there is whitespace before it
			// 2. sometimes a newline isn't output on the previous build command, which causes "Building ... " to be at end of that string
			if (findIndex1 != -1 && findIndex2 != -1) {
				// column 0 text: remove prefix string "Building ... " and workspace because it's redundant data
//				fileStr.Delete(0, findStr1.GetLength());
				fileStr.Delete(0, findIndex1 + findStr1.GetLength());
				//  column 1 text: create string with "[arch]" data
				archStr = fileStr;
				archStr.Delete(0, fileStr.ReverseFind(_T('[')));
				// column 0 text: now remove "[arch]" text because that goes in column 1
				fileStr.Delete(fileStr.ReverseFind(_T('[')) - 1, fileStr.GetLength());
				// ensure all backslashes are slashes
				fileStr.Replace(_T('\\'), _T('/'));

				// store INF filename for use later
				m_InfArray.Add(fileStr);

				lvi.iItem = m_cvListInf.GetItemCount();
				lvi.iSubItem = 0;
				lvi.mask =  LVIF_TEXT;
				lvi.pszText = fileStr.GetBuffer();
				fileStr.ReleaseBuffer();
				m_cvListInf.InsertItem(&lvi);
				lvi.iSubItem = 1;
				lvi.pszText = archStr.GetBuffer();
				m_cvListInf.SetItem(&lvi);
			} else if (!bBuildCfgFound && fileStr.Find(findStr3) != -1) {
				BOOL	bfSwitch = FALSE;	// FDF file
				BOOL	bpSwitch = FALSE;	// DSC file
				BOOL	boSwitch = FALSE;	// output folder

				// tokenize the string
				curPos = 0;
  				resToken = fileStr.Tokenize(_T("\t "), curPos);
				// is a token found?
				while (resToken != _T("")) {
					// if -o found, then next token will be output folder
					if (!boSwitch && resToken.CompareNoCase(_T("-o")) == 0) {
						m_buildOutputDir = fileStr.Tokenize(_T("\t "), curPos);
						if (!m_buildOutputDir.IsEmpty()) {
							boSwitch = TRUE;
							// Extract output folder used in build from "GenFds -o <folder>".
							m_buildOutputDir.Delete(0, m_workspace.GetLength());
							// Replace backslash with slash so code that searches paths can expect one format to find.
							m_buildOutputDir.Replace(_T('\\'), _T('/'));
							// Remove "<TARGET>_<TOOL_CHAIN_TAG>" so it matches OUTPUT_DIRECTORY in DSC file.
							m_buildOutputDir = m_buildOutputDir.Left(m_buildOutputDir.GetLength() - m_target.GetLength() - 1 - m_toolChain.GetLength());
							GetDlgItem(IDC_STATIC_BUILD_OUTPUT_DIR)->SetWindowTextW(m_buildOutputDir);
						}
					// if -f found, then next token will be FDF filename
					} else if (!bfSwitch && resToken.CompareNoCase(_T("-f")) == 0) {
						m_packageFDF = fileStr.Tokenize(_T("\t "), curPos);
						if (!m_packageFDF.IsEmpty()) {
							bfSwitch = TRUE;
							// Extract FDF used in build from "GenFds -f <FDF file>".
							m_packageFDF.Delete(0, m_workspace.GetLength());
							// Replace backslash with slash so code that searches paths can expect one format to find.
							m_packageFDF.Replace(_T('\\'), _T('/'));
							GetDlgItem(IDC_STATIC_PACKAGE_FDF)->SetWindowTextW(m_packageFDF);
						}
					// if -p found, then next token will be DSC filename
					} else if (!bpSwitch && resToken.CompareNoCase(_T("-p")) == 0) {
						m_packageDSC = fileStr.Tokenize(_T("\t "), curPos);
						if (!m_packageDSC.IsEmpty()) {
							bpSwitch = TRUE;
							// Extract DSC used in build from "GenFds -p <DSC file>".
							m_packageDSC.Delete(0, m_workspace.GetLength());
							// Replace backslash with slash so code that searches paths can expect one format to find.
							m_packageDSC.Replace(_T('\\'), _T('/'));
							GetDlgItem(IDC_STATIC_PACKAGE_DSC)->SetWindowTextW(m_packageDSC);
						}
					}

					// if both switches found, then break
					if (bfSwitch && bpSwitch && boSwitch) {
						// all build cfg items found, so set flag TRUE
						bBuildCfgFound = TRUE;
						break;
					}

					// get next token
					resToken = fileStr.Tokenize(_T("\t "), curPos);
				}
			}
		}
// *** END: find .INF files in build log
		csf.Close();

// *** error check and return
		if (!bBuildCfgFound) {
			InitBuildData();
			return;
		}
	
// TODO parse DSC and FDF
		// add package DSC to index 0 of .DEC array because it may have GUIDs and PCDs defined
//		m_DecArray.Add(m_packageDSC);

		// add package FDF to index 1 of .DEC array because it may have GUIDs and PCDs defined
//		m_DecArray.Add(m_packageFDF);

		// for storing each DEC filename and its treeitem root value
		CArray<HTREEITEM, HTREEITEM> hta;

		t_InfBuild				myInfBuild;
		t_ListSourceItemData	myListSourceItemData;
		int						nCount = 0;

// *** START: parse .INF files
		for (i = 0; i < m_InfArray.GetCount(); i++) {
			// get .INF filename
			tempStr = m_InfArray.GetAt(i);

			// add .INF file to array for later use when examining .DEC files
			myInfBuild.InfFileName = tempStr;

			// add .INF to source list, then open each .INF and parse the [Sources] section for source files used by that .INF
			lvi.iItem = m_cvListSource.GetItemCount();
			lvi.iSubItem = 0;
			lvi.mask =  LVIF_TEXT | LVIF_PARAM;
			lvi.pszText = tempStr.GetBuffer();
			m_cvListSource.InsertItem(&lvi);
			tempStr.ReleaseBuffer();

			// init the ListSource item's data
			myListSourceItemData.bIsInfFile = TRUE;
			// init m_DecClassLibrary index to -1; this will be set when DEC files are scanned for library implementation.
			myListSourceItemData.indexDecLibClassArray = -1;
			myListSourceItemData.indexInfRow = nCount;
			myListSourceItemData.indexInf = i;
			m_ListSourceItemData.Add(myListSourceItemData);

			// set INF rows to color style
			m_cvListSource.SetRowStyle(nCount, STYLE_COLOR_ROW, false);
			m_cvListSource.SetRowTextColor(nCount, RGB(255, 0, 0));

			BOOL			bProcessSources = FALSE;
			BOOL			bProcessDefines = FALSE;
			BOOL			bProcessPackages = FALSE;
			t_DecInInf		myDecInInf;
			CStringArray	csaDefineLHS;
			CStringArray	csaDefineRHS;

			// Any .DEC file found in .INF [Packages] is used in build.
			lineNum = 0;

			// track row number in list control for INF files with infRow
			// track item count in list control with nCount
			DWORD			infRow = nCount++;

			tempStr = m_workspace + myInfBuild.InfFileName;
			if (csf2.Open(tempStr, CFile::modeRead | CFile::typeText)) {
				tempStr.Format(_T("Parsing sections of .INF file %u ..."), i + 1);
				m_pwndProgress->SetText(tempStr);
				m_pwndProgress->Show();
				while (1) {
					// if EOF then break
					if (!csf2.ReadString(fileStr)) break;

					lineNum++;
					fileStr.Trim();
					if (fileStr.IsEmpty())
						continue;
					if (fileStr.GetAt(0) == _T('#'))
						continue;

					tempStr = fileStr;
					tempStr.MakeLower();

					if (tempStr.Find(_T("define ")) == 0) {
						// tokenize the string
						curPos = 0;
						// this Tokenize should parse the DEFINE string
  						resToken = fileStr.Tokenize(_T("= "), curPos);
						// get next token (LHS of equate), format should be DEFINE x=y
  						resToken = fileStr.Tokenize(_T("= "), curPos);
						// get next token (RHS of equate), format should be DEFINE x=y
  						resToken2 = fileStr.Tokenize(_T("= "), curPos);
						// is a token found?
						if (resToken != _T("") && resToken2 != _T("")) {
							csaDefineLHS.Add(resToken);
							csaDefineRHS.Add(resToken2);
						}
					}

					// if a new section is detected, then stop processing sections
					if (fileStr.GetAt(0) == _T('[')) {
						bProcessDefines = FALSE;
						bProcessSources = FALSE;
						bProcessPackages = FALSE;
					}

					if (tempStr.Find(_T("[defines]")) == 0) {
						bProcessDefines = TRUE;
						continue;
					}
					if (tempStr.Find(_T("[sources]")) == 0 || tempStr.Find(_T("[sources.")) == 0) {
						bProcessSources = TRUE;
						continue;
					}

					if (tempStr.Find(_T("[packages]")) == 0) {
						bProcessPackages = TRUE;
						continue;
					}

					if (bProcessPackages) {
						// tokenize, because there may be comments after the .DEC filename
						curPos = 0;
						resToken = fileStr.Tokenize(_T("\t #"), curPos);
						// is a token found?
// *** START: add item to DEC tree
						if (resToken != _T("")) {
							// Replace backslash with slash so code that searches paths can expect one format to find.
							resToken.Replace(_T('\\'), _T('/'));
							myDecInInf.DecFileName = resToken;
							myDecInInf.InfLineNum = lineNum;
							myInfBuild.vPackages.push_back(myDecInInf);

							// The tree control will be built with each branch named after a unique DEC filename,
							// and each child of that branch will be an INF that uses the DEC file.
							BOOL	bAddDec = TRUE;
							HTREEITEM hRoot, hItem;
							hRoot = m_cvTreeDec.GetTreeCtrl().GetRootItem();
	
							for (j = 0; j < m_DEC.GetCount(); j++) {
								// if filename is found, then don't add it
								if (myDecInInf.DecFileName == m_DEC.GetAt(j).DecFileName) {
									bAddDec = FALSE;
									break;
								}
								// get next root level tree item
								hRoot = m_cvTreeDec.GetTreeCtrl().GetNextItem(hRoot, TVGN_NEXT);
							}

							// The .DEC filename must be unique, so add it to the DEC tree.
							if (bAddDec) {
								t_DEC	myDEC;
								myDEC.DecFileName = myDecInInf.DecFileName;
								// PkgGuid will be set when ParseDecFiles() is called.
								m_DEC.Add(myDEC);
								hRoot = m_cvTreeDec.GetTreeCtrl().InsertItem(myDecInInf.DecFileName);
								hta.Add(hRoot);
							}
							// If DEC filename is unique, then hRoot equals the new tree branch.
							// If DEC filename isn't unique, then hRoot equals the existing branch per the loop above.
							// Insert item at hRoot branch.
							hItem = m_cvTreeDec.GetTreeCtrl().InsertItem(_T(""), hRoot);
							m_cvTreeDec.SetItemText(hItem, 1, myInfBuild.InfFileName);
							tempStr.Format(_T("%lu"), myDecInInf.InfLineNum);
							m_cvTreeDec.SetItemText(hItem, 2, tempStr);
						}
// *** END: add item to DEC tree
					}

					if (bProcessDefines) {
						curPos = 0;
						resToken = fileStr.Tokenize(_T("\t ="), curPos);
						resToken2 = fileStr.Tokenize(_T("\t ="), curPos);
						if (resToken != _T("") && resToken2 != _T("")) {
							if (resToken == _T("MODULE_TYPE")) {
								m_cvListSource.SetItemText(infRow, 2, resToken2);
								myInfBuild.ModuleType = resToken2;
							} else if (resToken == _T("LIBRARY_CLASS")) {
								curPos = 0;
								resToken = resToken2.Tokenize(_T("\t |"), curPos);
								m_cvListSource.SetItemText(infRow, 1, resToken);
								myInfBuild.LibraryClass = resToken;
								// tokenize library types
								while (curPos != -1) {
									resToken = resToken2.Tokenize(_T("\t |"), curPos);
									if (resToken != _T("")) {
										myInfBuild.vLibraryTypes.push_back(resToken);
									}
								}
							} else if (resToken == _T("ENTRY_POINT")) {
								m_cvListSource.SetItemText(infRow, 3, resToken2);
								myInfBuild.EntryPoint = resToken2;
							}
						}
					}

					if (bProcessSources) {
						// ensure all backslashes are slashes
						fileStr.Replace(_T('\\'), _T('/'));
						BOOL	bAddSource = TRUE;

						// if the source file uses a DEFINE, then try to find it and replace with the value
						if (fileStr.GetAt(0) == _T('$')) {
							// find a match for defined variable, indicated by $(name)
							bAddSource = FALSE;
							if (csaDefineLHS.GetCount() == csaDefineLHS.GetCount()) {
								// find the LHS=RHS pair that matches the tokens
								for (j = 0; j < csaDefineLHS.GetCount(); j++) {
									if (fileStr.Find(csaDefineLHS.GetAt(j)) != -1) {
										// create string to be replaced
										tempStr = _T("$(") + csaDefineLHS.GetAt(j) + _T(")");
										// replace the string
										fileStr.Replace(tempStr, csaDefineRHS.GetAt(j));
										bAddSource = TRUE;
										break;
									}
								}
							}
						}

						if (bAddSource) {
							tempStr2.Empty();
							// if options are specified after the filename (e.g. foo.c|MSFT||), then strip the options
							if (fileStr.Find(_T('|')) != -1) {
								// set tempStr2 to family
								tempStr2 = fileStr;
								tempStr2.Delete(0, 1 + fileStr.Find(_T('|')));
								tempStr2.TrimLeft();
								// if family isn't specified (| separates options, and it's at char 0), then empty the string
								if (tempStr2.GetAt(0) == _T('|'))
									tempStr2.Empty();
								// else parse family from the options
								else if (tempStr2.Find(_T('|')) != -1)
									tempStr2.Delete(tempStr2.Find(_T('|')), tempStr2.GetLength());

								fileStr.Delete(fileStr.Find(_T('|')), fileStr.GetLength());
								fileStr.TrimRight();
							}
							tempStr = _T("     " ) + fileStr;
							lvi.iItem = m_cvListSource.GetItemCount();
							lvi.iSubItem = 0;
							lvi.mask =  LVIF_TEXT | LVIF_PARAM;
							lvi.pszText = tempStr.GetBuffer();
							m_cvListSource.InsertItem(&lvi);
							tempStr.ReleaseBuffer();

							if (!tempStr2.IsEmpty())
								m_cvListSource.SetItemText(lvi.iItem, 1, tempStr2);

							// init the ListSource item's data
							myListSourceItemData.bIsInfFile = FALSE;
							// init m_DecClassLibrary index to -1; this is not used for source files.
							myListSourceItemData.indexDecLibClassArray = -1;
							myListSourceItemData.indexInfRow = infRow;
							myListSourceItemData.indexInf = -1;
							m_ListSourceItemData.Add(myListSourceItemData);

							nCount++;
						}
					}
				}

				m_vInfBuild.push_back(myInfBuild);
				csf2.Close();
			}
		}
// *** END: parse .INF file

/*
// saved for reference
		// SetItemData() must be called after all items have been added to m_ListSourceItemData.
		// The CArray data types allocates memory as it changes size and moves its data around in memory,
		// thus the memory addresses of each index aren't static while the size changes.
		for (i = 0; i < m_cvListSource.GetItemCount(); i++)
			m_cvListSource.SetItemData(i, (DWORD_PTR) &m_ListSourceItemData[i]);
*/

// *** START: find .INF files in .DSC file
		fileStr = m_workspace + m_packageDSC;
		// now find INF file in DSC file, and add the DSC line # to the list ctrl in column 2
		if (csf.Open(fileStr, CFile::modeRead | CFile::typeText)) {
			m_pwndProgress->SetText(_T("Finding .INF files in package DSC ..."));
			m_pwndProgress->Show();

			CString			infFilename;
			BOOL			bProcessLibraryClasses = FALSE;
			CStringArray	tokenArray;

			lineNum = 0;
			while (1) {
				// if EOF then break
				if (!csf.ReadString(fileStr)) break;

				lineNum++;
				tempStr = fileStr;
				tempStr.MakeLower();

				// find [LibraryClasses] items
				if (tempStr.Find(_T("[libraryclasses]")) == 0 || tempStr.Find(_T("[libraryclasses.")) == 0) {
					bProcessLibraryClasses = TRUE;
					continue;
				}

				// if a [LibraryClasses] declaration is found e.g. (Library|filename.inf)
				if (bProcessLibraryClasses && tempStr.Find(_T('|')) > 0 && tempStr.Find(_T(".inf")) != -1) {
					// tokenize the string
					curPos = 0;
					resToken = fileStr.Tokenize(_T("\t |{#"), curPos);
					resToken2 = fileStr.Tokenize(_T("\t |{#"), curPos);
					// is a library and .INF found?
					if (resToken != _T("") && resToken2 != _T("")) {
						// search INF list for matching .INF
						for (i = 0; i < m_cvListInf.GetItemCount(); i++) {
							infFilename = m_cvListInf.GetItemText(i, 0);
							// ensure all backslashes are slashes
							resToken2.Replace(_T('\\'), _T('/'));
							if (resToken2.CompareNoCase(infFilename) == 0) {
								tempStr.Format(_T("%u"), lineNum);
								// set package DSC line # where .INF file is used
								m_cvListInf.SetItemText(i, 2, tempStr);
								break;
							}
						}
					}
				}
			}
			csf.Close();
		}
// *** END: find .INF files in .DSC file

		// autosize the columns after data has been added
		for (i = 0; i < m_cvListInf.GetColumnCount(); i++)
			m_cvListInf.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

		// autosize the columns after data has been added
		for (i = 0; i < m_cvListSource.GetColumnCount(); i++)
			m_cvListSource.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

		// autosize the columns after data has been added
		// don't adjust last column "INF Line #"
		for (i = 0; i < m_cvTreeDec.GetHeaderCtrl().GetItemCount() - 1; i++)
			m_cvTreeDec.AdjustColumnWidth(i, FALSE);

		// set first visible item to top of tree
		m_cvTreeDec.GetTreeCtrl().SelectSetFirstVisible(m_cvTreeDec.GetTreeCtrl().GetRootItem());

		// update variables associated with control IDs due to SetWindowTextW() calls
		UpdateData(TRUE);

// *** START: parse .DEC files
		ParseDecFiles();
// *** END: parse .DEC files

		GetDlgItem(IDC_SEARCH_BUTTON)->EnableWindow();
		GetDlgItem(IDC_EDIT_SEARCH)->EnableWindow();
		GetDlgItem(IDC_SELECT_BUILD_REPORT)->EnableWindow();
		GetDlgItem(IDC_WRITE_SOURCE_LIST)->EnableWindow();

		// ensure all backslashes are slashes
		m_workspace.Replace(_T('\\'), _T('/'));
		// *** NOTE ***
		// Change workspace slash at end of INF file searching because INF files are extracted from build log based on the original
		// WORKSPACE string format.  If workspace is changed before this time, then matches may not be found.
		GetDlgItem(idBuildCfg[BUILD_CFG_IDC_WORKSPACE])->SetWindowTextW(m_workspace);

		m_radioView = e_RadioViewSourceUsedInBuild;
		UpdateData(FALSE);  // save m_radioView value
		OnBnClickedRadioFileData();

// TODO: scan C files to find where GUID is referenced
		lvi.iItem = m_cvListGuidModuleRef.GetItemCount();
		lvi.iSubItem = 0;
		lvi.mask =  LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = _T("list to be used to display C files that reference selected GUID   <not implemented>");
		m_cvListGuidModuleRef.InsertItem(&lvi);
	
		delete m_pwndProgress;
		m_pwndProgress = NULL;
	}
}


/*++
  Function: ParseDecFiles

  Parameters: none

  Purpose: parse .DEC files found when scanning .INF files, and populate list/tree controls.

  Returns: 0, success
--*/
int CEDKIIBuildDataViewerDlg::ParseDecFiles() {
	m_vGUID.clear();
	m_vPCD.clear();

	t_GUID			myGUID;
	t_PCD			myPCD;
	t_DecLibraryClass	myDecLibClass;
	int				i, j, counter, lineNum;
	int				curPos;
	CStdioFile		csf;
	CString			fileStr;
	CString			resToken;
	CString			tempStr;
	CStringArray	tokenArray;
	BOOL			bProcessDefines = FALSE;
	BOOL			bProcessPcds = FALSE;
	BOOL			bProcessGuids = FALSE;
	BOOL			bProcessPpis = FALSE;
	BOOL			bProcessProtocols = FALSE;
	BOOL			bProcessLibraryClasses = FALSE;
	LVITEM			lvi;
	// Start at top of tree when adding Package GUID.
	HTREEITEM		hItem = m_cvTreeDec.GetTreeCtrl().GetRootItem();
	vector<t_PCD>::iterator	itPCD;
	vector<t_GUID>::iterator itGUID;

	// scan *.dec files
	for (i = 0; i < m_DEC.GetCount(); i++) {
		tempStr.Format(_T("Parsing .DEC file %u ..."), i + 1);
		m_pwndProgress->SetText(tempStr);
		m_pwndProgress->Show();

		lineNum = 0;
		if (csf.Open(m_workspace + m_DEC.GetAt(i).DecFileName, CFile::modeRead | CFile::typeText)) {
			while (1) {
				lineNum++;
				// if EOF then break
				if (!csf.ReadString(fileStr)) break;

				if (fileStr.IsEmpty())
					continue;
				fileStr.Trim();
				if (fileStr.GetAt(0) == _T('#'))
					continue;

				if (fileStr.GetAt(0) == _T('[')) {
					bProcessDefines = FALSE;
					bProcessPcds = FALSE;
					bProcessGuids = FALSE;
					bProcessPpis = FALSE;
					bProcessProtocols = FALSE;
					bProcessLibraryClasses = FALSE;
					// IMPORTANT: section names aren't case sensitive, so switch to lower so all compares below use same case
					fileStr.MakeLower();
				}

				// IMPORTANT: Find using lower case strings so fileStr, which is forced to lower case, can succeed.
				if (fileStr.Find(_T("[defines]")) == 0) {
					bProcessDefines = TRUE;
					continue;
				}

				// There are many different PCD types, and each starts with '[Pcds' - see DEC spec for details.
				if (fileStr.Find(_T("[pcds")) == 0) {
					bProcessPcds = TRUE;
					continue;
				}

				if (fileStr.Find(_T("[guids]")) == 0 || fileStr.Find(_T("[guids.")) == 0) {
					bProcessGuids = TRUE;
					continue;
				}

				if (fileStr.Find(_T("[ppis]")) == 0 || fileStr.Find(_T("[ppis.")) == 0) {
					bProcessPpis = TRUE;
					continue;
				}

				if (fileStr.Find(_T("[protocols]")) == 0 || fileStr.Find(_T("[protocols.")) == 0) {
					bProcessProtocols = TRUE;
					continue;
				}

				if (fileStr.Find(_T("[libraryclasses]")) == 0 || fileStr.Find(_T("[libraryclasses.")) == 0) {
					bProcessLibraryClasses = TRUE;
					continue;
				}

				if (bProcessDefines) {
					// is a token found?
					curPos = 0;
					resToken = fileStr.Tokenize(_T("="), curPos);
					resToken.Trim(_T(" \t"));
					if (resToken.Compare(_T("PACKAGE_GUID")) == 0) {
						resToken = fileStr.Tokenize(_T("="), curPos);
						resToken.Trim(_T(" \t"));
						// Set Package GUID in m_cvTreeDec.
//						tempStr = m_cvTreeDec.GetItemText(hItem, 0);
//						fileStr = m_DEC.GetAt(i).DecFileName;
						m_cvTreeDec.SetItemText(hItem, 1, resToken);
						// Get next tree item for next iteration.
						hItem = m_cvTreeDec.GetTreeCtrl().GetNextItem(hItem, TVGN_NEXT);

						if (InsertGuidPool(resToken, e_GuidTypePackageDec, &m_DEC.GetAt(i).indexGuidPool, m_DEC.GetAt(i).DecFileName, lineNum)) {
						}
					}
				}

				if (bProcessLibraryClasses) {
					curPos = 0;
					tokenArray.RemoveAll();

					// is a token found?
					while ((resToken = fileStr.Tokenize(_T("|"), curPos)) != _T(""))
						tokenArray.Add(resToken);

					// found all 2 elements of a LibraryClass declaration
					if (tokenArray.GetSize() == 2) {
						myDecLibClass.DecFileName = m_DEC.GetAt(i).DecFileName;
						myDecLibClass.DecLineNum = lineNum;
						myDecLibClass.LibName = tokenArray.GetAt(0);
						myDecLibClass.ImplementFileName = m_workspace + m_DEC.GetAt(i).DecFileName;
						myDecLibClass.ImplementFileName.Replace(_T('\\'), _T('/'));
						// now remove DEC filename but keep the path, because the Library implementation is relative to the DEC path
						myDecLibClass.ImplementFileName.Delete(myDecLibClass.ImplementFileName.ReverseFind(_T('/')) + 1, myDecLibClass.ImplementFileName.GetLength() + 1);
						myDecLibClass.ImplementFileName += tokenArray.GetAt(1);
						m_DecLibClassArray.Add(myDecLibClass);
					}
				}

				if (bProcessPcds) {
					curPos = 0;
					tokenArray.RemoveAll();

					// is a token found?
					while ((resToken = fileStr.Tokenize(_T("{}| \t"), curPos)) != _T(""))
						tokenArray.Add(resToken);

					// found all 4 elements of a PCD declaration
					if (tokenArray.GetSize() == 4) {
						myPCD.DecFileName = m_DEC.GetAt(i).DecFileName;
						// split guidName.pcdName into 2 parts
						curPos = 0;
						resToken = tokenArray.GetAt(0);
						myPCD.GuidNameSpace = resToken.Tokenize(_T("."), curPos);
						myPCD.PcdName = resToken.Tokenize(_T("."), curPos);
						// if PCD value is a string, then display special string
						if (tokenArray.GetAt(1).Find(_T("L\"")) == 0)
							myPCD.Value = _T("string");
						else
							myPCD.Value = tokenArray.GetAt(1);
						myPCD.Type = tokenArray.GetAt(2);
						myPCD.TokenNum = _tcstoul(tokenArray.GetAt(3), 0, 16);
						myPCD.LineNum = lineNum;
						myPCD.bDuplicate = FALSE;
						myPCD.indexDuplicate = -1;

						for (counter = 0, itPCD = m_vPCD.begin(); itPCD != m_vPCD.end(); itPCD++, counter++) {
							if (itPCD->GuidNameSpace == myPCD.GuidNameSpace && itPCD->PcdName == myPCD.PcdName) {
								myPCD.bDuplicate = TRUE;
								myPCD.indexDuplicate = counter;
								break;
							}
						}

						m_vPCD.push_back(myPCD);
					} else if (tokenArray.GetSize() > 4) {
						// is it a string with spaces in it, thus causing size to be > 4?
						if (tokenArray.GetAt(1).Find(_T("L\"")) == 0) {
							myPCD.Value = _T("string");
						} else if (tokenArray.GetAt(1).ReverseFind(_T(',')) == tokenArray.GetAt(1).GetLength()-1) {
						// is it a data stream with byte values separated by ,?
							myPCD.Value = _T("data stream");
						} else {
							myPCD.Value = _T("unknown");
						}

						myPCD.DecFileName = m_DEC.GetAt(i).DecFileName;
						// split guidName.pcdName into 2 parts
						curPos = 0;
						resToken = tokenArray.GetAt(0);
						myPCD.GuidNameSpace = resToken.Tokenize(_T("."), curPos);
						myPCD.PcdName = resToken.Tokenize(_T("."), curPos);
						myPCD.Type = tokenArray.GetAt(tokenArray.GetCount()-2);
						myPCD.TokenNum = _tcstoul(tokenArray.GetAt(tokenArray.GetCount()-1), 0, 16);
						myPCD.LineNum = lineNum;
						myPCD.bDuplicate = FALSE;
						myPCD.indexDuplicate = -1;

						// search for item to determine if it's a duplicate before adding new item
						for (counter = 0, itPCD = m_vPCD.begin(); itPCD != m_vPCD.end(); itPCD++, counter++) {
							if (itPCD->GuidNameSpace == myPCD.GuidNameSpace && itPCD->PcdName == myPCD.PcdName) {
								myPCD.bDuplicate = TRUE;
								myPCD.indexDuplicate = counter;
								break;
							}
						}

						m_vPCD.push_back(myPCD);
					}
				}

				if (bProcessGuids || bProcessPpis || bProcessProtocols) {
					int		retVal;
					UINT8	GuidType;
					if (bProcessGuids)
						GuidType = e_GuidTypeVariable;
					else if (bProcessPpis)
						GuidType = e_GuidTypePpi;
					else if (bProcessProtocols)
						GuidType = e_GuidTypeProtocol;
					retVal = InsertGuidPool(fileStr, GuidType, &myGUID.indexGuidPool, m_DEC.GetAt(i).DecFileName, lineNum);
					if (retVal >= 0) {
						LVITEM lvi;
						lvi.iItem = m_cvListGuidVar.GetItemCount();
						lvi.iSubItem = 0;
						lvi.mask =  LVIF_TEXT;
						lvi.pszText = m_vGuidPool[myGUID.indexGuidPool].GuidName.GetBuffer();
						m_cvListGuidVar.InsertItem(&lvi);
						if (bProcessGuids)
							lvi.pszText = _T("GUID");
						else if (bProcessPpis)
							lvi.pszText = _T("PPI");
						else if (bProcessProtocols)
							lvi.pszText = _T("Protocol");
						m_cvListGuidVar.SetItemText(lvi.iItem, 1, lvi.pszText);
						GuidPoolValueAsString(tempStr, myGUID.indexGuidPool);
						m_cvListGuidVar.SetItemText(lvi.iItem, 2, tempStr);
						m_cvListGuidVar.SetItemText(lvi.iItem, 3, m_vGuidPool[myGUID.indexGuidPool].Filename);
						tempStr.Format(_T("%lu"), m_vGuidPool[myGUID.indexGuidPool].LineNum);
						m_cvListGuidVar.SetItemText(lvi.iItem, 4, tempStr);
						if (m_vGuidPool[myGUID.indexGuidPool].bDuplicate) {
							m_cvListGuidVar.SetRowStyle(lvi.iItem, STYLE_COLOR_ROW, false);
							m_cvListGuidVar.SetRowTextColor(lvi.iItem, RGB(255, 0, 0));
							m_cvListGuidVar.SetItemText(lvi.iItem, 5, _T("Yes"));
						} else
							m_cvListGuidVar.SetItemText(lvi.iItem, 5, _T("No"));
						m_vGUID.push_back(myGUID);
					} else {
					}
				}
			}
			csf.Close();
		}
	}

	// autosize the columns after data has been added
	for (i = 0; i < m_cvListGuidVar.GetColumnCount(); i++)
		m_cvListGuidVar.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

	DWORD	id;
	for (i = 0; i < m_cvListPcdUse.GetItemCount(); i++) {
		id = m_cvListPcdUse.GetItemData(i);
		if (id == PCD_LIST_ITEM_DATA_GUID) {
			myPCD.GuidNameSpace = m_cvListPcdUse.GetItemText(i, 0);

			UINT32	ii;
			for (ii = 0; ii < m_vGuidPool.size(); ii++) {
				if (myPCD.GuidNameSpace == m_vGuidPool[i].GuidName) {
					m_cvListPcdUse.SetItemText(i, 5, m_vGuidPool[i].Filename);
					tempStr.Format(_T("%lu"), m_vGuidPool[i].LineNum);
					m_cvListPcdUse.SetItemText(i, 6, tempStr);
					break;
				}
			}
		} else if (id == PCD_LIST_ITEM_DATA_PCD || id == PCD_LIST_ITEM_DATA_PCD_OVERRIDE) {
			myPCD.PcdName = m_cvListPcdUse.GetItemText(i, 0);
			myPCD.PcdName.TrimLeft(); // remove indenting whitespace

			for (itPCD = m_vPCD.begin(); itPCD != m_vPCD.end(); itPCD++) {
				if (myPCD.GuidNameSpace == itPCD->GuidNameSpace && myPCD.PcdName == itPCD->PcdName) {
					m_cvListPcdUse.SetItemText(i, 5, itPCD->DecFileName);
					tempStr.Format(_T("%lu"), itPCD->LineNum);
					m_cvListPcdUse.SetItemText(i, 6, tempStr);
					break;
				}
			}
		}
	}
	// autosize the columns after data has been added
	for (i = 0; i < m_cvListPcdUse.GetColumnCount(); i++)
		m_cvListPcdUse.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

	// fill list in with where PCDs are defined
	for (itPCD = m_vPCD.begin(); itPCD != m_vPCD.end(); itPCD++) {
		lvi.iItem = m_cvListPcdDef.GetItemCount();
		lvi.iSubItem = 0;
		lvi.mask =  LVIF_TEXT | LVIF_PARAM;
		lvi.pszText = itPCD->GuidNameSpace.GetBuffer();
		m_cvListPcdDef.InsertItem(&lvi);
		m_cvListPcdDef.SetItemText(lvi.iItem, 1, itPCD->PcdName);
		m_cvListPcdDef.SetItemText(lvi.iItem, 2, itPCD->Type);
		m_cvListPcdDef.SetItemText(lvi.iItem, 3, itPCD->Value);
		m_cvListPcdDef.SetItemText(lvi.iItem, 4, itPCD->DecFileName);
		tempStr.Format(_T("%lu"), itPCD->LineNum);
		m_cvListPcdDef.SetItemText(lvi.iItem, 5, tempStr);
		m_cvListPcdDef.SetItemText(lvi.iItem, 6, itPCD->bDuplicate ? _T("Yes") : _T("No"));
	}
	// autosize the columns after data has been added
	for (i = 0; i < m_cvListPcdDef.GetColumnCount(); i++)
		m_cvListPcdDef.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

// *** START: add Library implementation file for INF
	// An INF file that declares LIBRARY_CLASS must have the implementation .h file in one of the DEC files 
	// listed under the INF's [Packages] section.
	int			infIndex = -1;
	BOOL		bFlag;
	CString		libStr;
	for (i = 0; i < m_cvListSource.GetItemCount(); i++) {
		// is item an INF file?
		if (m_ListSourceItemData[i].bIsInfFile) {
			// index for m_InfBuild
			infIndex++;
			// get Library Name
			libStr = m_cvListSource.GetItemText(i, 1);
		} else {
			if (libStr.IsEmpty())
				continue;

			// set Library Name match flag to false
			bFlag = FALSE;

			// look for matching Library Name
			for (j = 0; j < m_DecLibClassArray.GetCount(); j++) {
				myDecLibClass = m_DecLibClassArray.GetAt(j);
				// If Library Name match found, then check the Library's DEC filename against the DEC files for the INF.
				if (libStr == myDecLibClass.LibName) {
					vector<t_DecInInf>::iterator itDec = m_vInfBuild.at(infIndex).vPackages.begin();
					vector<t_DecInInf>::iterator itDecEnd = m_vInfBuild.at(infIndex).vPackages.end();
					for (; itDec != itDecEnd; itDec++) {
						if (itDec->DecFileName == myDecLibClass.DecFileName) {
							tempStr = myDecLibClass.ImplementFileName;
							tempStr.Delete(0, m_workspace.GetLength());
							m_cvListSource.SetItemText(i, 1, tempStr);
							bFlag = TRUE;

							// Set index of m_DecLibClassArray item at the INF index so when the Library column is 
							// double-clicked (it is in the same row as the INF filename), the correct index of 
							// m_DecLibClassArray will be referenced.
							m_ListSourceItemData[m_ListSourceItemData[i].indexInfRow].indexDecLibClassArray = j;
							m_vInfBuild.at(infIndex).indexDecLibClass = j;
							break;
						}
					}
					if (bFlag)
						break;
				}
			}
			// if Library Name match not found in INF's [Packages] .DEC files, then it must be a NULL lib
			if (!bFlag)
				m_cvListSource.SetItemText(m_ListSourceItemData[i].indexInfRow, 1, libStr + _T(" (unknown)"));
		}
	}
// *** END: add Library implementation file for INF

	// autosize the columns after data has been added
	for (i = 0; i < m_cvListSource.GetColumnCount(); i++)
		m_cvListSource.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

	return 0;
}
