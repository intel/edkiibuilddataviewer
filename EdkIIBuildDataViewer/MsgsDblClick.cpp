/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// EventsDblClick.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"

/*++
  Function: OnNMDblclkLaunchEditor

  Parameters: pNMHDR - ptr to CListCtrl click info
			  pResult - ptr to result value

  Purpose: Launch editor for double-clicked file.
		   All list/tree control double-click events are routed here for efficient code re-use.

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMDblclkLaunchEditor(NMHDR *pNMHDR, LRESULT *pResult)
{
	if (m_editorName.IsEmpty()) {
		m_chooseEditorDlg.DoModal();
		m_chooseEditorDlg.GetEditorData(m_editorExe, m_editorName, m_editorSwitches);
	}
	
	if (!m_editorName.IsEmpty()) {
		LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

		CString		tempStr, tempStr2;
		int			subItem;

		// get filename from control
		if (m_radioView == e_RadioViewInfUsedInBuild) {
			if (pNMItemActivate->iSubItem <= 1)
				tempStr = m_cvListInf.GetItemText(pNMItemActivate->iItem, 0);
			else
				tempStr = m_packageDSC;
		} else if (m_radioView == e_RadioViewPcdDefinitions)
			tempStr = m_cvListPcdDef.GetItemText(pNMItemActivate->iItem, 4);
		else if (m_radioView == e_RadioViewPcdUsedInBuild)
			tempStr = m_cvListPcdUse.GetItemText(pNMItemActivate->iItem, 5);
		else if (m_radioView == e_RadioViewGuidVariables)
			tempStr = m_cvListGuidVar.GetItemText(pNMItemActivate->iItem, 3);
		else if (m_radioView == e_RadioViewDecUsedInBuild) {
			// lParam=0 when the item does not have children, and 1 when the item has children.
			// If item has children AND column 0 was double-clicked, then the branch is expanding, so there is nothing to process.
			if (pNMItemActivate->lParam == 1)
				tempStr = m_cvTreeDec.GetItemText(m_cvTreeDec.GetTreeCtrl().GetSelectedItem(), 0);
			else
				tempStr = m_cvTreeDec.GetItemText(m_cvTreeDec.GetTreeCtrl().GetSelectedItem(), 1);
		}
		else if (m_radioView == e_RadioViewSourceUsedInBuild) {
			// is item an INF file?
			if (m_ListSourceItemData[pNMItemActivate->iItem].bIsInfFile) {
				// if not column 1 (Library Name) then open .INF file
				if (pNMItemActivate->iSubItem != 1)
					tempStr = m_cvListSource.GetItemText(pNMItemActivate->iItem, 0);
				else {
					t_ListSourceItemData x = m_ListSourceItemData[pNMItemActivate->iItem];
					// if a library implementation exists, then value will be >= 0
					if (m_ListSourceItemData[pNMItemActivate->iItem].indexDecLibClassArray != -1)
						tempStr = m_DecLibClassArray[m_ListSourceItemData[pNMItemActivate->iItem].indexDecLibClassArray].DecFileName;
				}
			} else {
				if (pNMItemActivate->iSubItem == 1)
					tempStr = m_cvListSource.GetItemText(pNMItemActivate->iItem, 1);
				// if column 1 not clicked, then use column 0 text
				else {
					tempStr = m_cvListSource.GetItemText(pNMItemActivate->iItem, 0);
					// remove whitespace on left side, in case string was indented in control with whitespace
					tempStr.TrimLeft();
					// get .INF filename associated with the source file
					tempStr2 = m_cvListSource.GetItemText(m_ListSourceItemData[pNMItemActivate->iItem].indexInfRow, 0);

					// find last / and remove rest of string from that index, then append the source filename
					tempStr2.Delete(tempStr2.ReverseFind(_T('/')) + 1, tempStr2.GetLength() + 1);
					tempStr = tempStr2 + tempStr;
				}
			}
		}

		// remove whitespace on left side, in case string was indented in control with whitespace
		tempStr.TrimLeft();
		// was item found defined in a file?
		if (!tempStr.IsEmpty()) {
			CString paramStr;
			paramStr.Format(_T("%s/%s"), m_workspace, tempStr);

			// copy switches, and look for "LineNum" keyword to replace with a value
//			MessageBox(_T("editorSwitches=") + m_editorSwitches, _T("DEBUG"));
			int x = m_editorSwitches.Find(_T("LineNum"));
			if (x == -1) {
				tempStr.Empty();
			} else {
				// create editor switches
				tempStr = m_editorSwitches;
				// check for LineNum keyword
				tempStr.Replace(_T("LineNum"), _T(""));

				// get line number from control
				if (m_radioView == e_RadioViewInfUsedInBuild) {
					subItem = pNMItemActivate->iSubItem;
					if (subItem <= 1)
						tempStr.Insert(x, _T("1"));
					else
						tempStr.Insert(x, m_cvListInf.GetItemText(pNMItemActivate->iItem, 2));
				} else if (m_radioView == e_RadioViewPcdDefinitions)
					tempStr.Insert(x, m_cvListPcdDef.GetItemText(pNMItemActivate->iItem, 5));
				else if (m_radioView == e_RadioViewPcdUsedInBuild)
					tempStr.Insert(x, m_cvListPcdUse.GetItemText(pNMItemActivate->iItem, 6));
				else if (m_radioView == e_RadioViewGuidVariables)
					tempStr.Insert(x, m_cvListGuidVar.GetItemText(pNMItemActivate->iItem, 4));
				else if (m_radioView == e_RadioViewDecUsedInBuild)
					tempStr.Insert(x, m_cvTreeDec.GetItemText(m_cvTreeDec.GetTreeCtrl().GetSelectedItem(), 2));
				else if (m_radioView == e_RadioViewSourceUsedInBuild) {
					// is item an INF file?
					if (m_ListSourceItemData[pNMItemActivate->iItem].bIsInfFile) {
						// if not column 1 (Library), then open at line 1
						if (pNMItemActivate->iSubItem != 1)
							tempStr.Insert(x, _T("1"));
						else {
							// if a library implementation exists, then value will be >= 0
							if (m_ListSourceItemData[pNMItemActivate->iItem].indexDecLibClassArray != -1) {
								CString	s;
								s.Format(_T("%u"), m_DecLibClassArray[m_ListSourceItemData[pNMItemActivate->iItem].indexDecLibClassArray].DecLineNum);
								tempStr.Insert(x, s);
							} else
								tempStr.Insert(x, _T("1"));
						}
					} else {			
						tempStr.Insert(x, _T("1"));
					}
				}
// save for reference: how to convert _T numerical string to a value
//					tempStr.Format(_T(" -n%lu"), _tcstoul(m_cvListGuidVar.GetItemText(pNMItemActivate->iItem, 4), 0, 10));
			}

//			tempStr.Empty(); // for testing no usage of switches
			// was a line number defined?
			if (!tempStr.IsEmpty()) {
				paramStr += _T(' ') + tempStr;
			}

//			MessageBox(paramStr);

			SHELLEXECUTEINFO ShExecInfo;
			ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
			ShExecInfo.fMask = NULL;
			ShExecInfo.hwnd = NULL;
			if (m_editorExe.IsEmpty()) {
				ShExecInfo.lpVerb = _T("");
				ShExecInfo.lpFile = paramStr;
				ShExecInfo.lpParameters = _T("");
			} else {
				ShExecInfo.lpVerb = _T("open");
				ShExecInfo.lpFile = m_editorExe;
				ShExecInfo.lpParameters = paramStr;
			}
			ShExecInfo.lpDirectory = NULL;
			ShExecInfo.nShow = SW_SHOW;
			ShExecInfo.hInstApp = NULL;
//			MessageBox(_T("About to launch editor=") + m_editorExe + _T("\nwith param=") + paramStr, _T("DEBUG"));
			if (!ShellExecuteEx(&ShExecInfo))
				MessageBox(_T("Could not launch editor"), _T("ERROR"), MB_ICONERROR);
		}
	}
	
	*pResult = 0;
}


/*++
  Function: OnNMDblclkList_ShowModuleTree

  Parameters: pNMHDR - ptr to CColumnTreeWnd click info
			  pResult - ptr to result value

  Purpose: switch to 'Modules used in build' for details for double-clicked item in 'GUID (modules)' and 'FDF layout' views

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMDblclkList_ShowModuleTree(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// detect which column was clicked
	LVHITTESTINFO lvhti;
	lvhti.pt = pNMItemActivate->ptAction;

	CWnd*		wndFocus = GetFocus();
	CListCtrl*	pListCtrl;

	if (wndFocus == &m_cvListFdf)
		pListCtrl = &m_cvListFdf;
	else if (wndFocus == &m_cvListGuidModule)
		pListCtrl = &m_cvListGuidModule;
	else
		pListCtrl = NULL;

	if (pListCtrl != NULL) {
		pListCtrl->SubItemHitTest(&lvhti);
  
		if (lvhti.flags & LVHT_ONITEMLABEL) {
			int		getTextItem = -1;
			// FDF Layout list:
			// Search by Module Name or INF will find the same item because the items are associated in the Module tree, so search by Module Name.
			if (pListCtrl == &m_cvListFdf && lvhti.iSubItem == 1 || lvhti.iSubItem == 2)
				getTextItem = 1;
			// GUID (module) list:
			// Search by Module Name.
			else if (pListCtrl == &m_cvListGuidModule)
				getTextItem = 1;

			if (getTextItem != -1) {
				m_search = pListCtrl->GetItemText(lvhti.iItem, getTextItem);
				m_radioView = e_RadioViewModuleUsedInBuild;
				m_radioSearch = RADIO_SEARCH_NAME_START;
				UpdateData(FALSE);  // save m_radioView value
				OnBnClickedRadioFileData();
				Search();
			}
		}
	}

	*pResult = 0;
}


/*++
  Function: OnNMDblclkTreeModule_FindItem

  Parameters: pNMHDR - ptr to CColumnTreeWnd click info
			  pResult - ptr to result value

  Purpose: switch to File/Data Type view with more details for double-clicked item in 'Modules used in build' view

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMDblclkTreeModule_FindItem(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// iItem is the HTREEITEM of the clicked item
	DWORD		data = m_cvTreeModuleSummary.GetTreeCtrl().GetItemData((HTREEITEM) pNMItemActivate->iItem);
	CString		itemStr, searchStr, tempStr;
	int			i, j, count;
	BOOL		bFlag;

	*pResult = 0;

	// lParam=0 when the item does not have children, and 1 when the item has children.
	// If item has children AND column 0 was double-clicked, then the branch is expanding, so there is nothing to process.
	if (pNMItemActivate->lParam == 1 && pNMItemActivate->iSubItem == 0) {
		return;
	}

	if (data == e_ModuleSummaryModuleName) {
//		MessageBox(_T("Module Name"));
		// if .INF column is double-clicked, then switch to "Source files used in build" list
		if  (pNMItemActivate->iSubItem == 1) {
			m_search = m_cvTreeModuleSummary.GetItemText((HTREEITEM) pNMItemActivate->iItem, pNMItemActivate->iSubItem);
			// strip path from INF because it is ignored in Search()
			m_search.Delete(0, 1 + m_search.ReverseFind(_T('/')));
			m_radioView = e_RadioViewSourceUsedInBuild;
			m_radioSearch = RADIO_SEARCH_NAME_START;
			m_searchInf.SetCheck(BST_CHECKED);
			m_searchLibrary.SetCheck(BST_UNCHECKED);
			m_searchSource.SetCheck(BST_UNCHECKED);
			UpdateData(FALSE);  // save m_radioView value
			OnBnClickedRadioFileData();
			Search();
		}
	} else if (data == e_ModuleSummaryPcd) {
//		MessageBox(_T("PCD"));
		// if PCD Name column is double-clicked, then switch to "PCDs used in build" list
		if  (pNMItemActivate->iSubItem == 0) {
			searchStr = m_cvTreeModuleSummary.GetItemText((HTREEITEM) pNMItemActivate->iItem, pNMItemActivate->iSubItem);
			searchStr.MakeLower();
			CString	guidStr = m_cvTreeModuleSummary.GetItemText((HTREEITEM) pNMItemActivate->iItem, 2);
			guidStr.MakeLower();

			m_radioView = e_RadioViewPcdUsedInBuild;
			m_radioSearch = RADIO_SEARCH_NAME_START;
			UpdateData(FALSE);  // save m_radioView value
			OnBnClickedRadioFileData();

			i = count = 0;
			bFlag = FALSE;
			do {
				// GUID Namespace matches, now find PCD match
				if (guidStr.CompareNoCase(m_cvListPcdUse.GetItemText(m_GuidIndecesForListPcdUse[i].GuidIndex, 0)) == 0) {
					for (j = 1; j <= (int) m_GuidIndecesForListPcdUse[i].PcdCount; j++) {
						itemStr = m_cvListPcdUse.GetItemText(m_GuidIndecesForListPcdUse[i].GuidIndex + j, 0);
						itemStr.TrimLeft();
						if (searchStr.CompareNoCase(itemStr) == 0) {
							bFlag = TRUE;
							int x = m_GuidIndecesForListPcdUse[i].GuidIndex + j;
							SetListTopIndex(&m_cvListPcdUse, m_GuidIndecesForListPcdUse[i].GuidIndex + j, TRUE);
							break;
						}
					}
				}

				count = 1 + m_GuidIndecesForListPcdUse[i++].PcdCount; // +1 to skip GUID item
			} while (!bFlag && count < m_cvListPcdUse.GetItemCount() && i < m_GuidIndecesForListPcdUse.GetCount() - 1);
		// if "Defined In" column is double-clicked, then switch to ".DEC files used in build" list
		} else if  (pNMItemActivate->iSubItem == 1) {
			m_search = m_cvTreeModuleSummary.GetItemText((HTREEITEM) pNMItemActivate->iItem, pNMItemActivate->iSubItem);
			m_radioView = e_RadioViewDecUsedInBuild;
			m_radioSearch = RADIO_SEARCH_NAME_START;
			UpdateData(FALSE);  // save m_radioView value
			OnBnClickedRadioFileData();
			Search();
		// if "GUID" column is double-clicked, then switch to "GUID variables used in build" list
		} else if  (pNMItemActivate->iSubItem == 2) {
			m_search = m_cvTreeModuleSummary.GetItemText((HTREEITEM) pNMItemActivate->iItem, pNMItemActivate->iSubItem);
			m_radioView = e_RadioViewGuidVariables;
			m_radioSearch = RADIO_SEARCH_NAME_START;
			UpdateData(FALSE);  // save m_radioView value
			OnBnClickedRadioFileData();
			Search();
		}
	} else if (data == e_ModuleSummaryLibrary) {
//		MessageBox(_T("Library"));
		// if Name column is double-clicked, then switch to "Source files used in build" list
		if  (pNMItemActivate->iSubItem == 0) {
			m_search = m_cvTreeModuleSummary.GetItemText((HTREEITEM) pNMItemActivate->iItem, pNMItemActivate->iSubItem);
			m_radioView = e_RadioViewSourceUsedInBuild;
			m_radioSearch = RADIO_SEARCH_NAME_START;
			m_searchInf.SetCheck(BST_UNCHECKED);
			m_searchLibrary.SetCheck(BST_CHECKED);
			m_searchSource.SetCheck(BST_UNCHECKED);
			UpdateData(FALSE);  // save m_radioView value
			OnBnClickedRadioFileData();
			Search();
		}
		// if .INF column is double-clicked, then switch to "Source files used in build" list
		else if  (pNMItemActivate->iSubItem == 1) {
			m_search = m_cvTreeModuleSummary.GetItemText((HTREEITEM) pNMItemActivate->iItem, pNMItemActivate->iSubItem);
			// strip path from INF because it is ignored in Search()
			m_search.Delete(0, 1 + m_search.ReverseFind(_T('/')));
			m_radioView = e_RadioViewSourceUsedInBuild;
			m_radioSearch = RADIO_SEARCH_NAME_START;
			m_searchInf.SetCheck(BST_CHECKED);
			m_searchLibrary.SetCheck(BST_UNCHECKED);
			m_searchSource.SetCheck(BST_UNCHECKED);
			UpdateData(FALSE);  // save m_radioView value
			OnBnClickedRadioFileData();
			Search();
		}
	} else if (data == e_ModuleSummaryDepEx) {
//		MessageBox(_T("DepEx"));
		// if Name column is double-clicked, then switch to "GUID (variables)" list
		if  (pNMItemActivate->iSubItem == 0) {
			m_search = m_cvTreeModuleSummary.GetItemText((HTREEITEM) pNMItemActivate->iItem, pNMItemActivate->iSubItem);
			m_radioView = e_RadioViewGuidVariables;
			m_radioSearch = RADIO_SEARCH_NAME_START;
			UpdateData(FALSE);  // save m_radioView value
			OnBnClickedRadioFileData();
			Search();
		}
	} else {
//		MessageBox(_T("unknown"));
	}
}


/*++
  Function: OnNMDblclkTreeModule

  Parameters: pNMHDR - ptr to CColumnTreeWnd click info
			  pResult - ptr to result value

  Purpose: switch to File/Data Type view with more details for double-clicked item in 'Modules used in build' view

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMDblclkTreeModule(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// iItem is the HTREEITEM of the clicked item
	DWORD		data = m_cvTreeDec.GetTreeCtrl().GetItemData((HTREEITEM) pNMItemActivate->iItem);

	*pResult = 0;

	// lParam=0 when the item does not have children, and 1 when the item has children.
	// If item has children AND column 0 was double-clicked, then the branch is expanding, so there is nothing to process.
	// Send double-click message when one of these conditions is met:
	//   o item has children and column 1 is double-clicked (Defined In column, don't expand but launch editor)
	//   o item has no children, and any column is double-clicked (find item in other view)
	if ((pNMItemActivate->lParam == 1 && pNMItemActivate->iSubItem == 1) || 
		(pNMItemActivate->lParam == 0)) {
		pNMItemActivate->hdr.code = MYMSGID_TREEMODULE_DBLCLK_FIND_ITEM;
		// Send double-click event with custom msg so double-clicking DEC file will launch it in editor.
		SendMessage(WM_NOTIFY, IDC_TREE_MODULE, (LPARAM) pNMItemActivate);

		// Double-click when item has children will toggle state.
		// We don't want that to happen, so toggle it again to put it back.
		if (pNMItemActivate->lParam == 1)
			m_cvTreeModuleSummary.GetTreeCtrl().Expand((HTREEITEM) pNMItemActivate->iItem, TVE_TOGGLE);
	}
}


/*++
  Function: OnNMDblclkTreeDec

  Parameters: pNMHDR - ptr to CColumnTreeWnd click info
			  pResult - ptr to result value

  Purpose: expand/collapse '.DEC files used in build' view, and open files in editor

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMDblclkTreeDec(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// iItem is the HTREEITEM of the clicked item
	DWORD		data = m_cvTreeDec.GetTreeCtrl().GetItemData((HTREEITEM) pNMItemActivate->iItem);

	*pResult = 0;

	// lParam=0 when the item does not have children, and 1 when the item has children.
	// If item has children AND column 0 was double-clicked, then the branch is expanding, so there is nothing to process.
	// Send double-click message when one of these conditions is met:
	//   o item has children and column 0 is double-clicked (.DEC column, don't expand but launch editor)
	//   o item has no children, and column 1 is double-clicked (.INF column, launch editor)
	//   o item has no children, and column 2 is double-clicked (.INF line # column, launch editor)
	if ((pNMItemActivate->lParam == 1 && pNMItemActivate->iSubItem == 0) || 
		(pNMItemActivate->lParam == 0 && (pNMItemActivate->iSubItem == 1 || pNMItemActivate->iSubItem == 2))) {
		pNMItemActivate->hdr.code = MYMSGID_TREEDEC_DBLCLK_LAUNCH_EDITOR;
		// Send double-click event with custom msg so double-clicking DEC file will launch it in editor.
		SendMessage(WM_NOTIFY, IDC_TREE_DEC, (LPARAM) pNMItemActivate);

		// Double-click when item has children will toggle state.
		// We don't want that to happen, so toggle it again to put it back.
		if (pNMItemActivate->lParam == 1)
			m_cvTreeDec.GetTreeCtrl().Expand((HTREEITEM) pNMItemActivate->iItem, TVE_TOGGLE);
	}
}
