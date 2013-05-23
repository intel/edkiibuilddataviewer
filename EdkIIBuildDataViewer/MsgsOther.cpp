/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// EventsOther.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"

/*++
  Function: OnBnClickedRadioFileData

  Parameters: none

  Purpose: switch visible controls when a File/Data Type radio button is clicked

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData()
{
	// 2 lists per radio button, Data (left side) and Reference (right side)
	// use some IDs twice even though there is 1 list so that the *2 scheme can be used to show/hide 2 windows at a time
	// The radio button IDs should be listed in the same order as the radio controls in the Dialog Tab Order.
	// Each radio button should have its ON_BN_CLICKED event routed to this function.
	UINT32		idLists[e_RadioView_MAX*2] = {
		IDC_LIST_GUID_MODULE, IDC_LIST_GUID_MODULE_REF,
		IDC_LIST_GUID_VAR, IDC_LIST_GUID_VAR,
		IDC_LIST_PCD_DEF, IDC_LIST_PCD_DEF,
		IDC_LIST_PCD_USE, IDC_LIST_PCD_USE,
		IDC_TREE_MODULE, IDC_TREE_MODULE,
		IDC_LIST_INF, IDC_LIST_INF,
		IDC_TREE_DEC, IDC_TREE_DEC,
		IDC_LIST_SOURCE, IDC_LIST_SOURCE,
		IDC_TREE_FDF, IDC_LIST_FDF};
	UINT32		i;

	UpdateData(TRUE);  // retrieve m_radioView value

	// show/hide list controls based on radio selection
	for (i = 0; i < e_RadioView_MAX*2; i+=2) {
		if (i/2 == m_radioView) {
			GetDlgItem(idLists[i])->ShowWindow(SW_SHOW);
			GetDlgItem(idLists[i+1])->ShowWindow(SW_SHOW);
		} else {
			GetDlgItem(idLists[i])->ShowWindow(SW_HIDE);
			GetDlgItem(idLists[i+1])->ShowWindow(SW_HIDE);
		}
	}

	BOOL	bState = FALSE;
	if (m_radioView == e_RadioViewSourceUsedInBuild)
		bState = TRUE;
	GetDlgItem(IDC_CHECK_SEARCH_INF_NAME)->EnableWindow(bState);
	GetDlgItem(IDC_CHECK_SEARCH_LIBRARY_NAME)->EnableWindow(bState);
	GetDlgItem(IDC_CHECK_SEARCH_SOURCE_NAME)->EnableWindow(bState);

	// Update static text above each list control, based on radio selection.
	// m_radioView will have 0-based index to match Dialog Tab Order.
	switch (m_radioView) {
	case e_RadioViewGuidModule://list
		m_staticLeftList.SetWindowTextW(_T("GUID module list"));
		m_staticRightList.SetWindowTextW(_T("Selected GUID module referenced in (* = GUID definition file)"));
		if (!m_buildReportLibrary.GetCheck() && !m_buildReportPcd.GetCheck() && !m_buildReportDepex.GetCheck() && !m_buildReportBuildFlags.GetCheck() && !m_buildReportFixedAddress.GetCheck()) {
			MessageBox(_T("Run build with \"-Y LIBRARY -Y -PCD -Y DEXPEX -Y BUILD_FLAGS -Y FIXED_ADDRESS -y filename.txt\" to generate full Module Summary data, or only the flags for the desired Module Summary data."), _T("WARNING - build report file not specified"), MB_ICONWARNING);
			GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(_T(""));
		} else
			GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(m_fileGuidModule);
		break;
	case e_RadioViewGuidVariables://list
		m_staticLeftList.SetWindowTextW(_T("GUID variable list"));
		m_staticRightList.SetWindowTextW(_T("Selected GUID variable referenced in (* = GUID definition file); right-click a row with Duplicate=Yes to jump to definition"));
		GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(_T(".DEC files found in [Packages] section in each .INF; [Guids], [Protocols], [Ppis] sections"));
		break;
	case e_RadioViewPcdDefinitions://list
		m_staticLeftList.SetWindowTextW(_T("PCD definition list"));
		m_staticRightList.SetWindowTextW(_T("Double-click a row to edit the file at the PCD definition; right-click to jump to first definition found for duplicate"));
		GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(_T(".DEC files referenced by .INF files used in build"));
		break;
	case e_RadioViewPcdUsedInBuild://list
		m_staticLeftList.SetWindowTextW(_T("PCD usage list"));
		m_staticRightList.SetWindowTextW(_T("Double-click a row to edit the file at the PCD definition; left-click to copy to clipboard"));
		if (!m_buildReportPcd.GetCheck()) {
			MessageBox(_T("Run build with \"-Y PCD -y filename.txt\" to generate PCD variable list."), _T("WARNING - missing build log file"), MB_ICONWARNING);
			GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(_T(""));
		} else
			GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(m_filePcd);
		break;
	case e_RadioViewModuleUsedInBuild://tree
		m_staticLeftList.SetWindowTextW(_T("Module usage list"));
		m_staticRightList.SetWindowTextW(_T("Double-click an INF, Library, PCD, or Depex to find that item in its View"));
		if (!m_buildReportLibrary.GetCheck() && !m_buildReportPcd.GetCheck() && !m_buildReportDepex.GetCheck() && !m_buildReportBuildFlags.GetCheck() && !m_buildReportFixedAddress.GetCheck()) {
			MessageBox(_T("Run build with \"-Y LIBRARY -Y -PCD -Y DEXPEX -Y BUILD_FLAGS -Y FIXED_ADDRESS -y filename.txt\" to generate full Module Summary data, or only the flags for the desired Module Summary data."), _T("WARNING - build report file not specified"), MB_ICONWARNING);
			GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(_T(""));
		} else
			GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(m_fileModuleSummary);
		break;
	case e_RadioViewInfUsedInBuild://list
		m_staticLeftList.SetWindowTextW(_T(".INF files list"));
		m_staticRightList.SetWindowTextW(_T("Double-click: .INF file to edit it; DSC Line # to edit it"));
		GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(m_fileInf);
		break;
	case e_RadioViewDecUsedInBuild://tree
		m_staticLeftList.SetWindowTextW(_T(".DEC files list"));
		m_staticRightList.SetWindowTextW(_T("Double-click: .DEC file to edit it; .INF file or INF Line # to edit it at the line #"));
		GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(_T(".DEC files found in [Packages] section in each .INF"));
		break;
	case e_RadioViewSourceUsedInBuild://list
		m_staticLeftList.SetWindowTextW(_T("Source files list"));
		m_staticRightList.SetWindowTextW(_T("Double-click: .INF to edit it; Library to edit DEC file at its use; source file to edit it"));
		GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(_T("Sections of each .INF found in build log"));
		break;
	case e_RadioViewFdfLayout://tree
		m_staticLeftList.SetWindowTextW(_T("FDF layout"));
		m_staticRightList.SetWindowTextW(_T("Click a tree row to display FV modules; double-click a list row to view details of item"));
		if (!m_buildReportFlash.GetCheck()) {
			MessageBox(_T("Run build with \"-Y FLASH -y filename.txt\" to generate FDF layout."), _T("WARNING - build report file not specified"), MB_ICONWARNING);
			GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(_T(""));
		} else
			GetDlgItem(IDC_EDIT_LEFT_LIST)->SetWindowTextW(m_fileFlash);
		break;
	}
}


/*++
  Function: OnBnClickedCheckBuildReport

  Parameters: none

  Purpose: ignore build report found section checkboxes, as they are information only

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::OnBnClickedCheckBuildReport()
{
	// Ignore all clicks; boxes will be checked when -y log file is parsed.
	// I did not want to set the Disabled property because it grays it, which isn't as easy to read.
	return;
}


/*++
  Function: SetListTopIndex

  Parameters: pListCtrl - ptr to list control
			  nRow - row index to make visible
			  bSelect - TRUE to mark item selected, FALSE to not mark item selected

  Purpose: make a row in a list control visible and selected.

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::SetListTopIndex(CListCtrl* pListCtrl, UINT nRow, BOOL bSelect)
{
	if (bSelect) {
		// clear selected item
		pListCtrl->SetItemState(pListCtrl->GetSelectionMark(), ~LVIS_SELECTED, LVIS_SELECTED);
	}

	int topIndex = nRow;
	// ensure item is visible by setting the top visible index in the list
	pListCtrl->EnsureVisible(topIndex, FALSE);

	if (bSelect) {
		// set selected item
		pListCtrl->SetItemState(topIndex, LVIS_SELECTED, LVIS_SELECTED);
		pListCtrl->SetSelectionMark(topIndex);
	}

	RECT rect;
	if (pListCtrl->GetItemRect(topIndex, &rect, LVIR_LABEL)) {
	    CSize size;
		size.cx = 0;
		size.cy = rect.bottom - rect.top;
		size.cy *= topIndex - pListCtrl->GetTopIndex();
		if (topIndex != pListCtrl->GetTopIndex())
			pListCtrl->Scroll(size);
    }

	pListCtrl->SetFocus();
}


/*++
  Function: SetTreeTopIndex

  Parameters: pTreeCtrl - ptr to tree control
			  hItem - row index to make visible
			  bSelect - TRUE to mark item selected, FALSE to not mark item selected

  Purpose: make a row in a tree control visible and selected.

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::SetTreeTopIndex(CTreeCtrl* pTreeCtrl, HTREEITEM hItem, BOOL bSelect)
{
	if (bSelect) {
		pTreeCtrl->SetItemState(pTreeCtrl->GetSelectedItem(), ~TVIS_SELECTED, TVIS_SELECTED);
/*
		// clear selected items
		HTREEITEM hItem;
		for (hItem = pTreeCtrl->GetRootItem(); hItem != NULL; hItem = pTreeCtrl->GetNextVisibleItem(hItem)) {
			if (pTreeCtrl->GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED)  {
				pTreeCtrl->SetItemState(hItem, 0, TVIS_SELECTED);
			}
		}
*/
	}

	pTreeCtrl->SetFocus();
	// ensure item is visible by setting the top visible index in the tree
/*
	// this code isn't scrolling and selecting the item, so use the code below
	if (bSelect)
		pTreeCtrl->Select(hItem, TVGN_FIRSTVISIBLE | TVGN_CARET);
	else
		pTreeCtrl->Select(hItem, TVGN_FIRSTVISIBLE);
*/
	pTreeCtrl->SelectSetFirstVisible(hItem);
	if (bSelect)
		pTreeCtrl->SelectItem(hItem);
}


/*++
  Function: OnFdfLayoutTreeSelchanged

  Parameters: pNMHDR - ptr to CListCtrl click info
			  pResult - ptr to result value

  Purpose: change FDF list data below tree as FV selection changes

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnFdfLayoutTreeSelchanged(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMTREEVIEW* pNMTreeView = (NMTREEVIEW*)pNMHDR;
	*pResult = 0;

	// which item was selected?
	HTREEITEM hItem = m_cvTreeFdf.GetTreeCtrl().GetSelectedItem();
	if (!hItem) return;

	CString tempStr = m_cvTreeFdf.GetItemText(hItem, 1);
	m_cvListFdf.DeleteAllItems();

	// is the row an FV?
	if (!tempStr.IsEmpty()) {
		UINT32	fdfIndex = m_cvTreeFdf.GetTreeCtrl().GetItemData(hItem) & 0xFFFF; // FDF index is in low word
		UINT32	fvIndex = (m_cvTreeFdf.GetTreeCtrl().GetItemData(hItem) >> 16); // FV index is in high word
		UINT	i;

		for (i = 0; i < m_FDF.GetAt(fdfIndex).FV.at(fvIndex).FVModules.size(); i++) {
			m_cvListFdf.InsertItem(i, m_FDF.GetAt(fdfIndex).FV.at(fvIndex).FVModules.at(i).Offset);
			m_cvListFdf.SetItemText(m_cvListFdf.GetItemCount() - 1, 1, m_FDF.GetAt(fdfIndex).FV.at(fvIndex).FVModules.at(i).ModuleName);
			tempStr = m_FDF.GetAt(fdfIndex).FV.at(fvIndex).FVModules.at(i).InfFileName;
			if (tempStr.GetAt(0) == _T('('))
				tempStr.Delete(0);
			if (tempStr.GetAt(tempStr.GetLength() - 1) == _T(')'))
				tempStr.Delete(tempStr.GetLength() - 1);
			m_cvListFdf.SetItemText(m_cvListFdf.GetItemCount() - 1, 2, tempStr);
		}
		// autosize the columns after data has been added
		for (i = 0; i < (UINT) m_cvListFdf.GetColumnCount(); i++) {
			m_cvListFdf.SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);
		}
	}

	UpdateData(FALSE); // now display FV details in control
	*pResult = 0;
}


/*++
  Function: OnBnClickedChangeEditor

  Parameters: none

  Purpose: change editor settings.

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::OnBnClickedChangeEditor()
{
	m_chooseEditorDlg.DoModal();
	m_chooseEditorDlg.GetEditorData(m_editorExe, m_editorName, m_editorSwitches);
	UpdateData(FALSE);
}
