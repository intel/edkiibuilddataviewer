/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// Search.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"

/*++
  Function: ListCompareItem

  Parameters: pList		- ptr to list control
			  index		- index in list control to read
			  subItem	- list control subItem (column)
			  subItem2	- list control subItem (column)

  Purpose: Compares list control item at index:subItem or index:subItem2 to search string.
		   Search string is stored in m_search.

  Returns: -1 if item not found
			else subItem for match
--*/
int CEDKIIBuildDataViewerDlg::ListCompareItem(CCustomListCtrl* pList, int index, int subItem, int subItem2)
{
	if (m_search.IsEmpty()) return -1;
	if (subItem == -1) return -1;

	CString	searchStr = m_search;

	CString itemStr = pList->GetItemText(index, subItem);
	itemStr.TrimLeft(); // remove indentation whitespace if present
	
	// Special case: Source list subItem 0 may be INF filename (which has full path) or Source filename.
	if (pList == &m_cvListSource && subItem == 0) {
		// is item an INF file?
		if (m_ListSourceItemData[index].bIsInfFile) {
			// Remove the path so compares are against the INF filename.
			itemStr.Delete(0, itemStr.ReverseFind(_T('/')) + 1);
			// If INF not checked for search, then empty the string so compare fails.
			if (!m_searchInf.GetCheck())
				itemStr.Empty();
		} else {
			// If Source not checked for search, then empty the string so compare fails.
			if (!m_searchSource.GetCheck())
				itemStr.Empty();
		}
	}

	if (!m_searchCaseSensitive) {
		itemStr.MakeLower();
		searchStr.MakeLower();
	}

	if (m_searchExactMatch) {
		if (itemStr == searchStr) {
			m_searchStatus = _T("Match found at start of string");
			UpdateData(FALSE);
			SetListTopIndex(pList, index, TRUE);
			return subItem;
		}
	} else {
		if (itemStr.Find(searchStr) == 0) {
			m_searchStatus = _T("Match found at start of string");
			UpdateData(FALSE);
			SetListTopIndex(pList, index, TRUE);
			return subItem;
		}
	}

	if (subItem2 != -1) {
		itemStr = pList->GetItemText(index, subItem2);
		itemStr.TrimLeft(); // remove indentation whitespace if present

		if (!m_searchCaseSensitive) {
			itemStr.MakeLower();
			searchStr.MakeLower();
		}

		if (m_searchExactMatch) {
			if (itemStr == searchStr) {
				m_searchStatus = _T("Match found at start of string");
				UpdateData(FALSE);
				SetListTopIndex(pList, index, TRUE);
				return subItem2;
			}
		} else {
			if (itemStr.Find(searchStr) == 0) {
				m_searchStatus = _T("Match found at start of string");
				UpdateData(FALSE);
				SetListTopIndex(pList, index, TRUE);
				return subItem2;
			}
		}
	}

	m_searchStatus = _T("ERROR: Match not found");
	UpdateData(FALSE);
	return -1;
}


/*++
  Function: TreeWndCompareItem

  Parameters: pTreeWnd	- ptr to tree control
			  hti		- index in tree control to read
			  subItem	- tree control subItem (column)

  Purpose: Compares tree control item at hti:subItem to search string.
		   Search string is stored in m_search.

  Returns: -1 if item not found
			else subItem for match
--*/
int CEDKIIBuildDataViewerDlg::TreeWndCompareItem(CColumnTreeWnd* pTreeWnd, HTREEITEM hti, int subItem)
{
	if (m_search.IsEmpty()) return -1;

	CString	searchStr = m_search;

	CTreeCtrl*	pTreeCtrl = &pTreeWnd->GetTreeCtrl();

	CString itemStr = pTreeWnd->GetItemText(hti, subItem);
	itemStr.TrimLeft(); // remove indentation whitespace if present
	if (!m_searchCaseSensitive) {
		itemStr.MakeLower();
		searchStr.MakeLower();
	}

	if (m_searchExactMatch) {
		if (itemStr == searchStr) {
			m_searchStatus = _T("Match found at start of string");
			UpdateData(FALSE);
			SetTreeTopIndex(pTreeCtrl, hti, TRUE);
			return subItem;
		}
	} else {
		if (itemStr.Find(searchStr) == 0) {
			m_searchStatus = _T("Match found at start of string");
			UpdateData(FALSE);
			SetTreeTopIndex(pTreeCtrl, hti, TRUE);
			return subItem;
		}
	}

	return -1;
}


/*++
  Function: Search()

  Parameters: bDirForward 
				TRUE to search forward
				FALSE to search backward

  Purpose: Search list and trees controls for text.  m_radioView is used to determine control
		   to use, and m_search is used for text.  subItem to search is set based on value of
		   m_radioView.

		   PreTranslateMsg calls Search so that Enter key in search edit control is processed.

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::Search(BOOL bDirForward /*= TRUE*/)
{
	UpdateData(TRUE);

	if (m_search.IsEmpty()) return;

	int					searchSubItem, searchSubItem2;
	CCustomListCtrl*	pListCtrl;
	CColumnTreeWnd*		pTreeWnd;

	// set primary and secondary search subitems to -1 to indicate invalid usage
	searchSubItem = searchSubItem2 = -1;

	if (m_radioView == e_RadioViewGuidModule) {
		if (m_radioSearch == RADIO_SEARCH_NAME_START)
			searchSubItem = 1; // see OnInitDialog column creation in list control
		else if (m_radioSearch == RADIO_SEARCH_VALUE_START)
			searchSubItem = 2; // see OnInitDialog column creation in list control
		else
			return;
		pListCtrl = &m_cvListGuidModule;
	} else if (m_radioView == e_RadioViewGuidVariables) {
		// GUID Modules list should search name regardless of search type
		if (m_radioSearch == RADIO_SEARCH_VALUE_START)
			m_radioSearch = RADIO_SEARCH_NAME_START;

		if (m_radioSearch == RADIO_SEARCH_NAME_START)
			searchSubItem = 0; // see OnInitDialog column creation in list control
		else
			return;
		pListCtrl = &m_cvListGuidVar;
	} else if (m_radioView == e_RadioViewPcdDefinitions) {
		if (m_radioSearch == RADIO_SEARCH_NAME_START)
			searchSubItem = 1; // see OnInitDialog column creation in list control
		else if (m_radioSearch == RADIO_SEARCH_VALUE_START)
			searchSubItem = 3; // see OnInitDialog column creation in list control
		else
			return;
		pListCtrl = &m_cvListPcdDef;
	} else if (m_radioView == e_RadioViewPcdUsedInBuild) {
		if (m_radioSearch == RADIO_SEARCH_NAME_START)
			searchSubItem = 0; // see OnInitDialog column creation in list control
		else if (m_radioSearch == RADIO_SEARCH_VALUE_START)
			searchSubItem = 3; // see OnInitDialog column creation in list control
		else
			return;
		pListCtrl = &m_cvListPcdUse;
	} else if (m_radioView == e_RadioViewModuleUsedInBuild || m_radioView == e_RadioViewDecUsedInBuild) {
		// Module tree and .DEC files tree should search name regardless of search type
		if (m_radioSearch == RADIO_SEARCH_VALUE_START)
			m_radioSearch = RADIO_SEARCH_NAME_START;

		if (m_radioSearch == RADIO_SEARCH_NAME_START)
			searchSubItem = 0; // see OnInitDialog column creation in list control
		else
			return;
		if (m_radioView == e_RadioViewModuleUsedInBuild)
			pTreeWnd = &m_cvTreeModuleSummary;
		else if (m_radioView == e_RadioViewDecUsedInBuild)
			pTreeWnd = &m_cvTreeDec;
	} else if (m_radioView == e_RadioViewSourceUsedInBuild) {
		// Source files list should search name regardless of search type
		if (m_radioSearch == RADIO_SEARCH_VALUE_START)
			m_radioSearch = RADIO_SEARCH_NAME_START;

		if (m_radioSearch == RADIO_SEARCH_NAME_START) {
			if (!m_searchInf.GetCheck() && !m_searchLibrary.GetCheck() && !m_searchSource.GetCheck()) {
				MessageBox(_T("No Source items selected to search"), _T("ERROR"), MB_ICONERROR);
				return;
			}

			// set primary search subItem only if it is not yet set, and the search item is checked
			if (searchSubItem == -1 && m_searchInf.GetCheck())
				searchSubItem = 0;
			if (searchSubItem == -1 && m_searchLibrary.GetCheck())
				searchSubItem = 1;
			if (searchSubItem == -1 && m_searchSource.GetCheck())
				searchSubItem = 0;

			// set secondary search subItem only if it is not yet set, and the search item is checked
			if (searchSubItem2 == -1 && searchSubItem != 0 && m_searchInf.GetCheck())
				searchSubItem2 = 0;
			if (searchSubItem2 == -1 && searchSubItem != 3 && m_searchLibrary.GetCheck())
				searchSubItem2 = 1;
			if (searchSubItem2 == -1 && searchSubItem != 0 && m_searchSource.GetCheck())
				searchSubItem2 = 0;
		} else
			return;
		pListCtrl = &m_cvListSource;
	} else {
		MessageBox(_T("Search not supported for selected View type"), _T("ERROR"), MB_ICONERROR);
		return;
	}

	BOOL	bFlag = FALSE;
	int		i, res, indexMatch;

// *** START: list controls
	if (m_radioView == e_RadioViewGuidModule || m_radioView == e_RadioViewGuidVariables ||
		m_radioView == e_RadioViewPcdDefinitions || m_radioView == e_RadioViewPcdUsedInBuild ||
		m_radioView == e_RadioViewSourceUsedInBuild) {
		int		initialSel = pListCtrl->GetSelectionMark();

		if (bDirForward) {
			// determine starting selection mark for forward search
			// set to index after current selection, and check against max
			// if none selected, it will start at index 0 (beginning of list)
			int startSel = 1 + initialSel;
			if (startSel >= pListCtrl->GetItemCount())
				startSel = 0;

			// loop from selection to end of list
			for (i = startSel; i < pListCtrl->GetItemCount(); i++) {
				res = ListCompareItem(pListCtrl, i, searchSubItem, searchSubItem2);
				// if a match was found, then break loop
				if (res != -1) {
					bFlag = TRUE;
					indexMatch = i;
					break;
				}
			}
			// loop from beginning of list to selection; 2 loops so entire list is searched
			if (!bFlag) {
				for (i = 0; i <= startSel; i++) {
					res = ListCompareItem(pListCtrl, i, searchSubItem, searchSubItem2);
					// if a match was found, then break loop
					if (res != -1) {
						bFlag = TRUE;
						indexMatch = i;
						break;
					}
				}
			}
		} else {
			// determine starting selection mark for backward search
			// set to index before current selection, and check against 0
			// if none selected, it will start at end of list
			int startSel = -1 + initialSel;
			if (startSel < 0)
				startSel = pListCtrl->GetItemCount() - 1;

			// loop from selection to beginning of list
			for (i = startSel; i >= 0; i--) {
				res = ListCompareItem(pListCtrl, i, searchSubItem, searchSubItem2);
				// if a match was found, then break loop
				if (res != -1) {
					bFlag = TRUE;
					indexMatch = i;
					break;
				}
			}
			// loop from end of list to selection; 2 loops so entire list is searched
			if (!bFlag) {
				for (i = pListCtrl->GetItemCount() - 1; i >= startSel; i--) {
					res = ListCompareItem(pListCtrl, i, searchSubItem, searchSubItem2);
					// if a match was found, then break loop
					if (res != -1) {
						bFlag = TRUE;
						indexMatch = i;
						break;
					}
				}
			}
		}

		if (!bFlag)
			MessageBox(_T("ERROR: Match not found"), _T("Search result"), MB_ICONERROR);
		// if match was found, but it was at the initial selection, then a wrap occurred
		else if (indexMatch == initialSel) {
			m_searchStatus = _T("Match found at selected item due to wrap; no more matches found");
			UpdateData(FALSE);
		// ListCompareItem() writes a result string to m_searchStatus.
		// Write a different result string for Source radio because a match may have been found
		// on 1 of its 3 columns (INF/Library/Source file).
		} else if (m_radioView == e_RadioViewSourceUsedInBuild) {
			// If match found on subItem1 (Library name)
			if (res == 1)
				m_searchStatus = _T("Match found at start of string (Library name)");
			// If match found on subItem0,
			else if (res == 0) {
				// is item an INF file?
				if (m_ListSourceItemData[indexMatch].bIsInfFile)
					m_searchStatus = _T("Match found at start of string (INF filename)");
				else
					m_searchStatus = _T("Match found at start of string (Source filename)");
			}
			UpdateData(FALSE);
		}
// *** END: list controls
// *** START: tree controls
	} else if (m_radioView == e_RadioViewModuleUsedInBuild || m_radioView == e_RadioViewDecUsedInBuild) {
		// determine starting selection mark for forward/backward search
		CTreeCtrl*	pTreeCtrl = &pTreeWnd->GetTreeCtrl();
		HTREEITEM	hFindItem = pTreeCtrl->GetSelectedItem();
		HTREEITEM	hInitialSel = hFindItem;

		if (bDirForward) {
			// determine starting selection mark for forward search
			// set to index after current selection, and check against max
			// if none selected, it will start at index 0 (beginning of tree)
			if (hFindItem != NULL)
				hFindItem = pTreeCtrl->GetNextItem(hFindItem, TVGN_NEXT);
			if (hFindItem == NULL)
				hInitialSel = hFindItem = pTreeCtrl->GetRootItem();

			// get row number of selected item
			pTreeWnd->GetTreeItem(hFindItem, (UINT*) &indexMatch, TRUE);
			// loop from selection to end of tree
			while (hFindItem != NULL) {
				res = TreeWndCompareItem(pTreeWnd, hFindItem, searchSubItem);
				if (res != -1) {
					bFlag = TRUE;
					break;
				}
				hFindItem = pTreeCtrl->GetNextItem(hFindItem, TVGN_NEXT);
				indexMatch++;
			}

			// loop from beginning of tree to selection; 2 loops so entire tree is searched
			if (!bFlag) {
				indexMatch = 0;
				hFindItem = pTreeCtrl->GetRootItem();
				while (1) {
					res = TreeWndCompareItem(pTreeWnd, hFindItem, searchSubItem);
					if (res != -1) {
						bFlag = TRUE;
						break;
					}
					// break if search has iterated to initial selection
					if (hFindItem == hInitialSel)
						break;
					hFindItem = pTreeCtrl->GetNextItem(hFindItem, TVGN_NEXT);
					indexMatch++;
				}
			}
		} else {
			// determine starting selection mark for backward search
			// set to index before current selection, and check against 0
			// if none selected, it will start at end of tree
			if (hFindItem != NULL)
				hFindItem = pTreeCtrl->GetNextItem(hFindItem, TVGN_PREVIOUS);
			if (hFindItem == NULL)
				hInitialSel = hFindItem = pTreeCtrl->GetNextItem(pTreeCtrl->GetRootItem(), TVGN_LASTVISIBLE);

			// get row number of selected item
			pTreeWnd->GetTreeItem(hFindItem, (UINT*) &indexMatch, TRUE);
			// loop from selection to beginning of tree
			while (hFindItem != NULL) {
				res = TreeWndCompareItem(pTreeWnd, hFindItem, searchSubItem);
				if (res != -1) {
					bFlag = TRUE;
					break;
				}
				hFindItem = pTreeCtrl->GetNextItem(hFindItem, TVGN_PREVIOUS);
				indexMatch--;
			}

			// loop from end of tree to selection; 2 loops so entire tree is searched
			if (!bFlag) {
				indexMatch = pTreeCtrl->GetCount() - 1;
				hFindItem = pTreeCtrl->GetNextItem(pTreeCtrl->GetRootItem(), TVGN_LASTVISIBLE);
				while (1) {
					res = TreeWndCompareItem(pTreeWnd, hFindItem, searchSubItem);
					if (res != -1) {
						bFlag = TRUE;
						break;
					}
					// break if search has iterated to initial selection
					if (hFindItem == hInitialSel)
						break;
					hFindItem = pTreeCtrl->GetNextItem(hFindItem, TVGN_PREVIOUS);
					indexMatch--;
				}
			}
		}

		if (!bFlag)
			MessageBox(_T("Match not found"), _T("ERROR"), MB_ICONERROR);
		// if match was found, but it was at the initial selection, then a wrap occurred
		else if (hFindItem == hInitialSel) {
			m_searchStatus = _T("Match found at selected item due to wrap; no more matches found");
			UpdateData(FALSE);
		}
	}
// *** END: tree controls

//	GetDlgItem(IDC_EDIT_SEARCH)->SetFocus();
}


void CEDKIIBuildDataViewerDlg::OnClickedSearchButton()
{
	Search();
}
