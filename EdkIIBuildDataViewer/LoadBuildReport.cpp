/*++
  Copyright (c) 2012-2013, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// LoadBuildYLog.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"

/*++
  Function: OnBnClickedSelectYLog

  Parameters: none

  Purpose: loads build report

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::OnBnClickedSelectBuildReport()
{
	CFileDialog		fd(TRUE, L"*.log", NULL, OFN_DONTADDTORECENT | OFN_ENABLESIZING, _T("Report files (*.log)|*.log|All files (*.*)|*.*||"));

	fd.m_ofn.lpstrTitle = _T("Select EDK II BIOS build report file");
	if (fd.DoModal() == IDOK) {
		CStdioFile		csf;
		int				curPos, i, j, count, lineNum = 0, moduleCounter = 0;
		CString			fileStr, checkStr, resToken, guidStr, tempStr, errorStr;
		CStringArray	tokenArray;
		TCHAR			ch;
		BOOL			bHeaderFound = FALSE;
		BOOL			bValidated = FALSE;
		BOOL			bSectionPcd = FALSE, bProcessPcd = FALSE;
		BOOL			bSectionModuleSummary = FALSE, bProcessModuleSummary = FALSE;
		int				nModuleSummaryType = -1;
		BOOL			bSectionFD = FALSE, bProcessFD = FALSE;
		BOOL			bSectionFV = FALSE, bProcessFV = FALSE;
		BOOL			bProcessFVModules = FALSE;
		LVITEM			lvi;
		t_FD			FD;
		t_FV			FV;
		t_FVModule		FVModule;
		HTREEITEM		hRoot, hItem, hItem2;

		if (csf.Open(fd.GetPathName(), CFile::modeRead | CFile::typeText)) {
			// validate Y log configuration matches build log
			count = 0;
			while (!bValidated) {
				// if EOF then break
				if (!csf.ReadString(fileStr)) {
					MessageBox(_T("Log file configurations do not match!\n\nPlatform DSC Path, Architecture, Tool Chain, Target"), _T("ERROR"), MB_ICONERROR);
					break;
				}

				lineNum++;
				fileStr.Trim();

				if (lineNum == 1) {
					if (fileStr.Find(_T("Platform Summary")) == 0)
						continue;
					else {
						MessageBox(_T("Invalid report file!"), _T("ERROR"),  MB_ICONERROR);
						break;
					}
				}

				if (fileStr.Find(_T("Platform DSC Path:")) == 0) {
					// get DSC file from string
					fileStr.Delete(0, _tcslen(_T("Platform DSC Path:")));
					fileStr.TrimLeft(); // trim leading whitespace after string was deleted
					// ensure all backslashes are slashes
					fileStr.Replace(_T('\\'), _T('/'));
					tempStr = m_workspace + _T('/') + m_packageDSC;
					if (fileStr.Compare(m_workspace + _T('/') + m_packageDSC) != 0) {
						errorStr.Format(_T("DSC file paths in log files do not match\n\nLog file: %s\nReport file: %s"), fileStr, m_workspace + _T('/') + m_packageDSC);
						MessageBox(errorStr, _T("ERROR"),  MB_ICONERROR);
						break;
					} else {
						count++;
					}
				}

				if (fileStr.Find(_T("Architectures:")) == 0) {
					// get DSC file from string
					fileStr.Delete(0, _tcslen(_T("Architectures:")));
					fileStr.TrimLeft(); // trim leading whitespace after string was deleted
					if (fileStr.Compare(m_targetArch) != 0) {
						MessageBox(_T("Architectures in log files do not match!"), _T("ERROR"),  MB_ICONERROR);
						break;
					} else {
						count++;
					}
				}

				if (fileStr.Find(_T("Tool Chain:")) == 0) {
					// get DSC file from string
					fileStr.Delete(0, _tcslen(_T("Tool Chain:")));
					fileStr.TrimLeft(); // trim leading whitespace after string was deleted
					if (fileStr.Compare(m_toolChain) != 0) {
						MessageBox(_T("Tool Chain in log files do not match!"), _T("ERROR"),  MB_ICONERROR);
						break;
					} else {
						count++;
					}
				}

				if (fileStr.Find(_T("Target:")) == 0) {
					// get DSC file from string
					fileStr.Delete(0, _tcslen(_T("Target:")));
					fileStr.TrimLeft(); // trim leading whitespace after string was deleted
					if (fileStr.Compare(m_target) != 0) {
						MessageBox(_T("Target in log files do not match!"), _T("ERROR"),  MB_ICONERROR);
						break;
					} else {
						count++;
					}
				}

				if (count == 4)
					bValidated = TRUE;
			}

			if (m_pwndProgress != NULL)
				delete m_pwndProgress;
			m_pwndProgress = new CXProgressWnd(this, _T("Parsing build report file"), FALSE, FALSE);
			m_pwndProgress->GoModal(this);

			while (bValidated && !bHeaderFound) {
				// if EOF then break
				if (!csf.ReadString(fileStr)) break;
				fileStr.Trim();

				// search for "Report Content:" followed by options separated by ,
				// This is in the header of the report. It specifies the section types output by -Y on command line.
				// If "Report Content:" is not found, then the file should not be parsed.
				if (fileStr.Find(_T("Report Content:")) == 0) {
					// delete string to get to tokens
					fileStr.Delete(0, CString(_T("Report Content:")).GetLength());

					// clear all checks because it is not known what sections are present in the report
					InitBuildReportData();

					// clear controls before parsing report file
					//
					// DO NOT clear DEC items m_vcListDec and m_Dec because they are populated when build log was loaded.
					// .DEC files found by scanning the workspace that are not in build log are added at end of list.
					m_cvListPcdUse.DeleteAllItems();
					m_cvListFdf.DeleteAllItems();
					m_cvTreeFdf.GetTreeCtrl().DeleteAllItems();
					m_cvTreeModuleSummary.GetTreeCtrl().DeleteAllItems();
					m_GuidIndecesForListPcdUse.RemoveAll();
					
					UpdateData(FALSE);

					// clear internal data structures before parsing report file
					m_DecFindArray.RemoveAll();
					m_FDF.RemoveAll();

					// determine sections in the report
					// tokenize the string
					curPos = 0;
					// is a token found?
					while ((resToken = fileStr.Tokenize(_T(", "), curPos)) != _T("")) {
						if (resToken.Compare(_T("PCD")) == 0) m_buildReportPcd.SetCheck(BST_CHECKED);
						else if (resToken.Compare(_T("LIBRARY")) == 0) m_buildReportLibrary.SetCheck(BST_CHECKED);
						else if (resToken.Compare(_T("FLASH")) == 0) m_buildReportFlash.SetCheck(BST_CHECKED);
						else if (resToken.Compare(_T("DEPEX")) == 0) m_buildReportDepex.SetCheck(BST_CHECKED);
						else if (resToken.Compare(_T("BUILD_FLAGS")) == 0) m_buildReportBuildFlags.SetCheck(BST_CHECKED);
						else if (resToken.Compare(_T("FIXED_ADDRESS")) == 0) m_buildReportFixedAddress.SetCheck(BST_CHECKED);
						else if (resToken.Compare(_T("EXECUTION_ORDER")) == 0) m_buildReportExecutionOrder.SetCheck(BST_CHECKED);
					}

					bHeaderFound = TRUE;
				}
			}

			// Header not found, so clean up and return.
			if (!bHeaderFound) {
				csf.Close();
				delete m_pwndProgress;
				m_pwndProgress = NULL;
				return;
			}

			m_pwndProgress->SetText(_T("Parsing PCDs and FVs used in build ..."));
			m_pwndProgress->Show();

			m_buildReport = fd.GetPathName();
			// Replace backslash with slash so code that searches paths can expect one format to find.
			m_buildReport.Replace(_T('\\'), _T('/'));
			GetDlgItem(IDC_EDIT_BUILD_REPORT)->SetWindowTextW(m_buildReport);
			m_filePcd = m_buildReport + _T("\r\nPCD section");
			m_fileFlash = m_buildReport + _T("\r\nFLASH section");
			m_fileModuleSummary = m_buildReport + _T("\r\nModule Summary sections");

			m_FDF.RemoveAll();

			CStringArray	tokenArray;
			CString			moduleName;
			t_GUIDIndex_PCDCount	GuidIndexPcdCount;

			while (1) {
				// if EOF then break
				if (!csf.ReadString(fileStr)) break;
				fileStr.Trim();

// *** START: Module Summary section parse
				if (bSectionModuleSummary && !fileStr.IsEmpty()) {
					if (!bProcessModuleSummary) {
						checkStr = fileStr.Left(2);
						if (checkStr.Compare(_T(">-")) == 0) { // start of Summary type section
							bProcessModuleSummary = TRUE;
							continue;
						} else if (checkStr.Compare(_T("<=")) == 0) { // end of Module Summary section
							bSectionModuleSummary = FALSE;
							continue;
						}

						// parse Module Summary header
						curPos = 0;
						// is a token found?
						resToken = fileStr.Tokenize(_T(":"), curPos);
						if (resToken != _T("")) {
							// Parse Module Summary header
							// Track which Module Summary item is being parsed so it can be added to the correct control column
							if (resToken.Compare(_T("Module Name")) == 0) {
								i = 0;
							} else if (resToken.Compare(_T("Module INF Path")) == 0) {
								i = 1;
							} else if (resToken.Compare(_T("File GUID")) == 0) {
								i = 2;
							} else if (resToken.Compare(_T("Size")) == 0) {
								i = 3;
							} else if (resToken.Compare(_T("Driver Type")) == 0) {
								i = 4;
							} else {
								i = -1;
							}

							if (i >= 0 && i <= 4) {
								fileStr.Delete(0, 1 + resToken.GetLength());
								fileStr.TrimLeft(); // trim leading whitespace after string was deleted
								// Module Name is i==0 (see "Parse Module Summary header" above)
								if (i == 0) {
									// If a Module Summary header is found, then add new item to GUID Module list.
									// Dispatch # and Name are added here.  GUID is added below when that value is parsed.
									lvi.iItem = m_cvListGuidModule.GetItemCount();
									lvi.iSubItem = 0;
									lvi.mask =  LVIF_TEXT;
									lvi.pszText = _T("");
									m_cvListGuidModule.InsertItem(&lvi);
									lvi.iSubItem = 1;
									lvi.pszText = fileStr.GetBuffer();
									m_cvListGuidModule.SetItem(&lvi);

									hRoot = m_cvTreeModuleSummary.GetTreeCtrl().InsertItem(fileStr);
//									m_cvTreeModuleSummary.GetTreeCtrl().SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);
									moduleName = fileStr;

									if (moduleCounter == 0)
										m_fileGuidModule = m_buildReport + _T("\r\nModule Summary Execution Order");

									// Modules are listed in execution order, so add the index to the GUID module list.
									int index;
									moduleCounter++;
									for (index = 0; index < m_cvListGuidModule.GetItemCount(); index++) {
										tempStr = m_cvListGuidModule.GetItemText(index, 1);
										if (moduleName == tempStr) {
											tempStr.Format(_T("%d"), moduleCounter);
											m_cvListGuidModule.SetItemText(index, 0, tempStr);
											break;
										}
									}
								} else {
									// Module INF Path is item 1 (see "Parse Module Summary header" above)
									if (i == 1) {
										// ensure all backslashes are slashes
										fileStr.Replace(_T('\\'), _T('/'));
									} else if (i == 2) {
									// Module GUID is item 2 (see "Parse Module Summary header" above)
										lvi.iItem = m_cvListGuidModule.GetItemCount() - 1;
										lvi.mask =  LVIF_TEXT;
										lvi.iSubItem = 2;
										lvi.pszText = fileStr.GetBuffer();
										m_cvListGuidModule.SetItem(&lvi);

										UINT32	GuidPoolIndex;
										if (InsertGuidPool(fileStr, e_GuidTypeFileInf, &GuidPoolIndex, m_cvTreeModuleSummary.GetItemText(hRoot, i), 1)) {
										}
									}
									m_cvTreeModuleSummary.SetItemText(hRoot, i, fileStr);
								}
							}
						}
						continue;
					}

					// determine summary section type
					if (nModuleSummaryType == -1) {
						if (fileStr.Compare(_T("PCD")) == 0) {
							nModuleSummaryType = e_ModuleSummaryPcd;
							hItem = m_cvTreeModuleSummary.GetTreeCtrl().InsertItem(fileStr, hRoot);
							m_cvTreeModuleSummary.GetTreeCtrl().SetItemData(hItem, e_ModuleSummaryPcd);
						} else if (fileStr.Compare(_T("Library")) == 0) {
							nModuleSummaryType = e_ModuleSummaryLibrary;
							hItem = m_cvTreeModuleSummary.GetTreeCtrl().InsertItem(fileStr, hRoot);
							m_cvTreeModuleSummary.GetTreeCtrl().SetItemData(hItem, e_ModuleSummaryLibrary);
							j = 0; // index set to 0 to indicate .INF is expected
						} else if (fileStr.Compare(_T("Final Dependency Expression (DEPEX) Instructions")) == 0) {
							nModuleSummaryType = e_ModuleSummaryDepEx;
							hItem = m_cvTreeModuleSummary.GetTreeCtrl().InsertItem(_T("DEPEX"), hRoot);
							m_cvTreeModuleSummary.GetTreeCtrl().SetItemData(hItem, e_ModuleSummaryDepEx);
						}
						else if (fileStr.Compare(_T("Build Flags")) == 0)
							nModuleSummaryType = e_ModuleSummaryBuildFlags;
						else if (fileStr.Compare(_T("Fixed Address Prediction")) == 0)
							nModuleSummaryType = e_ModuleSummaryFixedAddress;
					} else {
//						fileStr.TrimLeft(); // trim leading whitespace after string was deleted
						checkStr = fileStr.Left(2);
						if (checkStr.Compare(_T("<-")) == 0 || // end of Summary type block
							checkStr.Compare(_T("<=")) == 0) { // end of Summary type section

							if (checkStr.Compare(_T("<=")) == 0) { // end of Summary type section
								bSectionModuleSummary = FALSE;
							}

							bProcessModuleSummary = FALSE;
							nModuleSummaryType = -1;

							// if no children were inserted, then delete the item
							if (m_cvTreeModuleSummary.GetTreeCtrl().GetChildItem(hItem) == NULL)
								m_cvTreeModuleSummary.GetTreeCtrl().DeleteItem(hItem);
						}

						if (nModuleSummaryType == e_ModuleSummaryPcd) {
							tokenArray.RemoveAll();

							// trim - char, because string could be PCD header end
							fileStr.TrimLeft(_T('-'));
							if (!fileStr.IsEmpty()) {
								curPos = 0;
								// is a token found?
								while ((resToken = fileStr.Tokenize(_T(":(){}= \t"), curPos)) != _T(""))
									tokenArray.Add(resToken);

								if (tokenArray.GetSize() == 1) { // must be GUID namespace
									guidStr = fileStr;
								} else if (tokenArray.GetSize() >= 4) { // must be PCD
									if (tokenArray.GetSize() == 5) // *x special indicator before PCD name, so delete it
										tokenArray.RemoveAt(0);

									CString pcdStr = tokenArray.GetAt(0);
									hItem2 = m_cvTreeModuleSummary.GetTreeCtrl().InsertItem(pcdStr, hItem);
									m_cvTreeModuleSummary.GetTreeCtrl().SetItemData(hItem2, e_ModuleSummaryPcd);
									m_cvTreeModuleSummary.SetItemText(hItem2, 2, guidStr);
									m_cvTreeModuleSummary.SetItemText(hItem2, 4, tokenArray.GetAt(2));
									m_cvTreeModuleSummary.SetItemText(hItem2, 3, tokenArray.GetAt(3));

									vector<t_PCD>::iterator	itPCD;
									for (itPCD = m_vPCD.begin(); itPCD != m_vPCD.end(); itPCD++) {
										if (guidStr == itPCD->GuidNameSpace && pcdStr == itPCD->PcdName) {
											m_cvTreeModuleSummary.SetItemText(hItem2, 1, itPCD->DecFileName);
											break;
										}
									}
								}
							}
						} else if (nModuleSummaryType == e_ModuleSummaryLibrary) {
							resToken = fileStr;
							// trim - char, because string could be Library header end
							resToken.TrimLeft(_T('-'));
							if (!resToken.IsEmpty()) {
								// ensure all backslashes are slashes
								fileStr.Replace(_T('\\'), _T('/'));
								if (j == 0) {
									j++; // index set to 1 to indicate LibraryName is expected
									fileStr.Delete(0, 1 + m_workspace.GetLength());
									hItem2 = m_cvTreeModuleSummary.GetTreeCtrl().InsertItem(_T(""), hItem);
									m_cvTreeModuleSummary.GetTreeCtrl().SetItemData(hItem2, e_ModuleSummaryLibrary);
									m_cvTreeModuleSummary.SetItemText(hItem2, 1, fileStr);
								} else {
									j--; // index set to 0 to indicate .INF/LibraryName pair is completed
									fileStr.Trim(_T("{}"));
									// get first token because library name may be followed by class/constructor/destructor
									curPos = 0;
									resToken = fileStr.Tokenize(_T(":"), curPos);
									m_cvTreeModuleSummary.SetItemText(hItem2, 0, resToken);
								}
							}
						} else if (nModuleSummaryType == e_ModuleSummaryDepEx) {
							// trim - char, because string could be PCD header end
							fileStr.TrimLeft(_T('-'));
							if (!fileStr.IsEmpty()) {
								count = 0;
								// look for DEPEX keywords
								if (fileStr.Find(_T("PUSH")) == 0) {
									count = 4;
									fileStr.Delete(0, count); // delete PUSH keyword; it's followed by GUID name
									fileStr.TrimLeft(); // trim whitespace between PUSH and GUID name
								} else if (fileStr.Find(_T("TRUE")) == 0)
									count = 4;
								else if (fileStr.Find(_T("FALSE")) == 0)
									count = 5;

								if (count > 0) {
									hItem2 = m_cvTreeModuleSummary.GetTreeCtrl().InsertItem(fileStr, hItem);
									m_cvTreeModuleSummary.GetTreeCtrl().SetItemData(hItem2, e_ModuleSummaryDepEx);
								}
							}
						} else if (nModuleSummaryType == e_ModuleSummaryBuildFlags) {
						} else if (nModuleSummaryType == e_ModuleSummaryFixedAddress) {
						}
					}
					continue;
				}
// *** END: Module Summary section parse
// *** START: FLASH section parse
				else if (bSectionFD && !fileStr.IsEmpty()) {
					if (!bSectionFV) {
						bProcessFD = TRUE;

						if (bProcessFD) {
							// search for "FD Name:" which is the beginning of FD data
							if (fileStr.Find(_T("FD Name:")) == 0) {
								// delete string to get to tokens
								fileStr.Delete(0, CString(_T("FD Name:")).GetLength());
								fileStr.Trim();
								FD.FdName = fileStr;
								continue;
							}
							else if (fileStr.Find(_T("Base Address:")) == 0) {
								// delete string to get to tokens
								fileStr.Delete(0, CString(_T("Base Address:")).GetLength());
								fileStr.Trim();
								FD.BaseAddr = fileStr;
								continue;
							}
							else if (fileStr.Find(_T("Size:")) == 0) {
								// delete string to get to tokens
								fileStr.Delete(0, CString(_T("Size:")).GetLength());
								fileStr.Trim();
								FD.Size = fileStr;
								continue;
							}
							else if (fileStr.Compare(_T("Fd Region")) == 0) {
								bProcessFD = FALSE;
								bSectionFV = TRUE;
								bProcessFV = TRUE;

								FD.FV.clear();
								continue;
							}
							else {
								continue;
							}
						}
					} else {
						checkStr = fileStr.Left(2);
						if (checkStr.Compare(_T("<=")) == 0) { // end of FD section
							m_FDF.Add(FD);

							bSectionFD = FALSE;
							bSectionFV = FALSE;
							continue;
						}
						else if (checkStr.Compare(_T("<-")) == 0) { // end of FV section
							FD.FV.push_back(FV);
							// Clear some fields that don't exist in every FV.
							// Some FV are data only, which don't have data for these fields.
							FV.FvName.Empty();
							FV.OccupiedSize.Empty();
							FV.Size.Empty();
							FV.FreeSize.Empty();

							FV.FVModules.clear();
							bProcessFVModules = FALSE;
							continue;
						}

						bProcessFV = TRUE;

						if (bProcessFVModules) {
							// format is "offset name (infFilename.inf)", check for ".inf)" at end of string
							if (fileStr.Find(_T(".inf)")) == fileStr.GetLength() - 5) {
								tokenArray.RemoveAll();

								curPos = 0;
								// is a token found?
								while ((resToken = fileStr.Tokenize(_T(" \t"), curPos)) != _T(""))
									tokenArray.Add(resToken);
								// found all 3 elements of a FV module
								if (tokenArray.GetSize() == 3) {
									FVModule.Offset = tokenArray.GetAt(0);
									FVModule.ModuleName = tokenArray.GetAt(1);
									FVModule.InfFileName = tokenArray.GetAt(2);
									// Replace backslash with slash so code that searches paths can expect one format to find.
									FVModule.InfFileName.Replace(_T('\\'), _T('/'));
									// add Module to FV array
									FV.FVModules.push_back(FVModule);
									continue;
								}
							}
						}

						if (bProcessFV && !fileStr.IsEmpty()) {
							// search for "FD Name:" which is the beginning of FD data
							if (fileStr.Find(_T("Fv Name:")) == 0) {
								// delete string to get to tokens
								fileStr.Delete(0, CString(_T("Fv Name:")).GetLength());
								fileStr.Trim();
								FV.FvName = fileStr;
								continue;
							}
							else if (fileStr.Find(_T("Base Address:")) == 0) {
								// delete string to get to tokens
								fileStr.Delete(0, CString(_T("Base Address:")).GetLength());
								fileStr.Trim();
								FV.BaseAddr = fileStr;
								continue;
							}
							else if (fileStr.Find(_T("Size:")) == 0) {
								// delete string to get to tokens
								fileStr.Delete(0, CString(_T("Size:")).GetLength());
								fileStr.Trim();
								FV.Size = fileStr;
								continue;
							}
							else if (fileStr.Find(_T("Occupied Size:")) == 0) {
								// delete string to get to tokens
								fileStr.Delete(0, CString(_T("Occupied Size:")).GetLength());
								fileStr.Trim();
								FV.OccupiedSize = fileStr;
								continue;
							}
							else if (fileStr.Find(_T("Free Size:")) == 0) {
								// delete string to get to tokens
								fileStr.Delete(0, CString(_T("Free Size:")).GetLength());
								fileStr.Trim();
								FV.FreeSize = fileStr;
								continue;
							}
							else if (fileStr.Find(_T("Offset")) != -1 && fileStr.Find(_T("Module")) != -1) {
								bProcessFV = FALSE;
								bProcessFVModules = TRUE;
								continue;
							}
							else {
								continue;
							}
						}
					}
				}
// *** END: FLASH section parse
// *** START: PCD section parse
				else if (bSectionPcd && !fileStr.IsEmpty()) {
					ch = fileStr.GetAt(0);
					if (ch == _T('<')) { // end of section
						bSectionPcd = FALSE;
						bProcessPcd = FALSE;
						continue;
					} else if (ch == _T('=')) {
						bProcessPcd = TRUE;
						continue;
					}

					if (bProcessPcd && ch != _T('\r') && ch != _T('=')) {
						if (fileStr.Find(_T(' ')) == -1) { // must be GUID namespace
							lvi.iItem = m_cvListPcdUse.GetItemCount();
							lvi.iSubItem = 0;
							lvi.mask =  LVIF_TEXT | LVIF_PARAM;
							lvi.pszText = fileStr.GetBuffer();
							lvi.lParam = PCD_LIST_ITEM_DATA_GUID; // indicates GUID item
							m_cvListPcdUse.InsertItem(&lvi);
							m_cvListPcdUse.SetRowStyle(m_cvListPcdUse.GetItemCount() - 1, STYLE_COLOR_ROW, false); // bold the GUID namespace
							m_cvListPcdUse.SetRowTextColor(m_cvListPcdUse.GetItemCount() - 1, RGB(255, 0, 0));
							// store GUID Namespace index so they can quickly be found, instead of searching the text
							// item was added above, so store count-1 because index is 0 based
							GuidIndexPcdCount.GuidIndex = m_cvListPcdUse.GetItemCount() - 1;
							GuidIndexPcdCount.PcdCount = 0;
							m_GuidIndecesForListPcdUse.Add(GuidIndexPcdCount);
						} else {
							// skip lines that contain DEC default info
							if (fileStr.Find(_T("DEC DEFAULT")) != -1) continue;
							// skip lines that contain DYNHII info, e.g. gSomeGuid : L"string": 0x0
							if (fileStr.Find(_T("L\"")) != -1 && fileStr.Find(_T("\":")) != -1) continue;

							// clear token array each time a line is processed for tokens
							tokenArray.RemoveAll();
							curPos = 0;
							tempStr = fileStr;
							resToken = fileStr.Tokenize(_T(":(){}= \t"), curPos);
							// is a token found?
							if (resToken != _T("")) {
								lvi.lParam = PCD_LIST_ITEM_DATA_PCD; // indicates default of no PCD override present
								// PCD override present?
								if (resToken.GetAt(0) == _T('*')) {
									if (resToken.GetAt(1) == _T('P'))
										tokenArray.Add(_T("DSC"));
									else if (resToken.GetAt(1) == _T('F'))
										tokenArray.Add(_T("FDF"));
									else if (resToken.GetAt(1) == _T('M'))
										tokenArray.Add(_T("Module"));
									else
										tokenArray.Add(_T(""));

									if (!tokenArray.GetAt(0).IsEmpty())
										lvi.lParam = PCD_LIST_ITEM_DATA_PCD_OVERRIDE; // indicates PCD override present

									// get next token, which is PCD name
									// must do this here in case the first Tokenize above was the PCD name, so that adding it below
									// will always have the PCD name token in the array index 1
									resToken = fileStr.Tokenize(_T(":(){}= \t"), curPos);
									if (resToken == _T("")) continue;
								} else
									// add empty item in array index 0 so it is always the Override indicator
									tokenArray.Add(_T(""));

								// add PCD name token
								tokenArray.Add(resToken);

								// next token for valid PCD line is PCD type
								resToken = fileStr.Tokenize(_T(":(){}= \t"), curPos);
								if (resToken == _T("")) continue;
								tokenArray.Add(resToken);

								// next token for valid PCD line is data type
								resToken = fileStr.Tokenize(_T(":(){}= \t"), curPos);
								if (resToken == _T("")) continue;
								tokenArray.Add(resToken);

								// next token for valid PCD line is value
								resToken = fileStr.Tokenize(_T(":(){}= \t"), curPos);
								if (resToken == _T("")) continue;
	
								// is it a string?
								if (resToken.Find(_T("L\"")) == 0) {
									tokenArray.Add(_T("string"));
								} else if (resToken.ReverseFind(_T(',')) == resToken.GetLength()-1) {
								// Is it a data stream, which is represented as values separated by ,
								// This is too long to display in the GUI, so display a special string.
									tokenArray.Add(_T("data stream"));
								} else {
									tokenArray.Add(resToken);
								}

								lvi.iItem = m_cvListPcdUse.GetItemCount();
								lvi.iSubItem = 0;
								lvi.mask =  LVIF_TEXT | LVIF_PARAM;
								resToken = _T("     ") + tokenArray.GetAt(1);
								lvi.pszText = resToken.GetBuffer();
								m_cvListPcdUse.InsertItem(&lvi);
								m_cvListPcdUse.SetItemText(lvi.iItem, 1, tokenArray.GetAt(2));
								m_cvListPcdUse.SetItemText(lvi.iItem, 2, tokenArray.GetAt(3));
								m_cvListPcdUse.SetItemText(lvi.iItem, 3, tokenArray.GetAt(4));
								m_cvListPcdUse.SetItemText(lvi.iItem, 4, tokenArray.GetAt(0));

								// increment GUID Namespace PCD counter
								m_GuidIndecesForListPcdUse[m_GuidIndecesForListPcdUse.GetCount() - 1].PcdCount++;
								GuidIndexPcdCount = m_GuidIndecesForListPcdUse[m_GuidIndecesForListPcdUse.GetCount() - 1];

								// Columns 5 and 6 filled in by searching for PCD match when PCD definitions were parsed by ParseDecFiles(),
								// which is called at the end of loading the build log.
								CString		GuidNameSpace = m_cvListPcdUse.GetItemText(GuidIndexPcdCount.GuidIndex, 0);
								CString		PcdName = tokenArray.GetAt(1);
								vector<t_PCD>::iterator	itPCD;

								// search for PCD match
								for (itPCD = m_vPCD.begin(); itPCD != m_vPCD.end(); itPCD++) {
									if (GuidNameSpace == itPCD->GuidNameSpace && PcdName == itPCD->PcdName) {
										m_cvListPcdUse.SetItemText(lvi.iItem, 5, itPCD->DecFileName);
										tempStr.Format(_T("%u"), itPCD->LineNum);
										m_cvListPcdUse.SetItemText(lvi.iItem, 6, tempStr);
										break;
									}
								}
							}
						}
					}
				}
// *** END: PCD section parse

				// search for "Platform Configuration Database Report"
				// This is the start of the PCD section.
				// created by "-Y PCD"
				if (fileStr.Find(_T("Platform Configuration Database Report")) == 0) {
					bSectionPcd = TRUE;
				}
				// search for "Firmware Device"
				// This is the start of the FLASH section.
				// created by "-Y FLASH"
				else if (fileStr.Find(_T("Firmware Device (FD)")) == 0) {
					bSectionFD = TRUE;
					m_pwndProgress->SetText(_T("Parsing Firmware Device ..."));
					m_pwndProgress->Show();
				}
				// search for "Module Summary"
				// This is the start of the Module Summary section.
				// created by "-Y LIBRARY -Y PCD -Y DEXPEX -Y BUILD_FLAGS -Y FIXED_ADDRESS" or any combination.
				else if (fileStr.Find(_T("Module Summary")) == 0) {
					bSectionModuleSummary = TRUE;
					m_pwndProgress->SetText(_T("Parsing Module Summary ..."));
					m_pwndProgress->Show();
				}
			}

			csf.Close();
		}

		// populate FDF tree control
		for (i = 0; i < m_FDF.GetCount(); i++) {
			hRoot = m_cvTreeFdf.GetTreeCtrl().InsertItem(m_FDF.GetAt(i).FdName);
			m_cvTreeFdf.SetItemText(hRoot, 2, m_FDF.GetAt(i).BaseAddr);
			m_cvTreeFdf.SetItemText(hRoot, 3, m_FDF.GetAt(i).Size);

			for (j = 0; j < (int) m_FDF.GetAt(i).FV.size(); j++) {
				hItem = m_cvTreeFdf.GetTreeCtrl().InsertItem(_T(""), hRoot);
//				m_cvTreeFdf.GetTreeCtrl().SetItemState(hItem, TVIF_STATE | TVIF_PARAM, TVIF_STATE | TVIF_PARAM);
				m_cvTreeFdf.GetTreeCtrl().SetItemData(hItem, i + (j << 16)); // FD index in low word, FV index in high word
				m_cvTreeFdf.SetItemText(hItem, 1, m_FDF.GetAt(i).FV.at(j).FvName);
				m_cvTreeFdf.SetItemText(hItem, 2, m_FDF.GetAt(i).FV.at(j).BaseAddr);
				m_cvTreeFdf.SetItemText(hItem, 3, m_FDF.GetAt(i).FV.at(j).Size);
				m_cvTreeFdf.SetItemText(hItem, 4, m_FDF.GetAt(i).FV.at(j).OccupiedSize);
				m_cvTreeFdf.SetItemText(hItem, 5, m_FDF.GetAt(i).FV.at(j).FreeSize);
			}

			m_cvTreeFdf.GetTreeCtrl().Expand(hRoot, TVE_EXPAND);
		}

		UpdateData(FALSE); // update GUI

		// autosize the columns after data has been added
		for (i = 0; i < m_cvListPcdUse.GetColumnCount(); i++)
			m_cvListPcdUse.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

		// autosize the columns after data has been added
		for (i = 0; i < m_cvListGuidModule.GetColumnCount(); i++)
			m_cvListGuidModule.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);

		// autosize the columns after data has been added
		for (i = 0; i < m_cvTreeFdf.GetHeaderCtrl().GetItemCount(); i++)
			m_cvTreeFdf.AdjustColumnWidth(i, TRUE);
		// set first visible item to top of tree
		m_cvTreeFdf.GetTreeCtrl().SelectSetFirstVisible(m_cvTreeFdf.GetTreeCtrl().GetRootItem());

		// autosize the columns after data has been added
		for (i = 0; i < m_cvTreeModuleSummary.GetHeaderCtrl().GetItemCount(); i++)
			m_cvTreeModuleSummary.AdjustColumnWidth(i, TRUE);
		// set first visible item to top of tree
		m_cvTreeModuleSummary.GetTreeCtrl().SelectSetFirstVisible(m_cvTreeModuleSummary.GetTreeCtrl().GetRootItem());

		delete m_pwndProgress;
		m_pwndProgress = NULL;
	}
}
