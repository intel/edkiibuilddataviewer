/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// EventsClick.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"

/*++
  Function: OnNMRClickListGuidVar

  Parameters: pNMHDR - ptr to CListCtrl click info
			  pResult - ptr to result value

  Purpose: right-click handler for 'GUID (variables)' radio selection; sets visible list row to
		   first declaration for item that is duplicated.

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMRClickListGuidVar(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (m_vGuidPool[m_vGUID[pNMItemActivate->iItem].indexGuidPool].bDuplicate) {
		// There could be more GUIDs in the pool than in the list, so subtract list index from GUID Pool index to get the index difference.
		// Set the list to the GUID Pool duplicate index minus the index difference.
		UINT32 indexDiff = m_vGUID[pNMItemActivate->iItem].indexGuidPool - m_vGuidPool[m_vGUID[pNMItemActivate->iItem].indexGuidPool].indexDuplicate;
		SetListTopIndex(&m_cvListGuidVar, pNMItemActivate->iItem - indexDiff, TRUE);
	}
	*pResult = 0;
}


/*++
  Function: OnNMRClickListPcdDef

  Parameters: pNMHDR - ptr to CListCtrl click info
			  pResult - ptr to result value

  Purpose: right-click handler for 'PCD definitions' radio selection; sets visible list row to
		   first declaration for item that is duplicated.

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMRClickListPcdDef(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (m_vPCD.at(pNMItemActivate->iItem).bDuplicate)
		SetListTopIndex(&m_cvListPcdDef, m_vPCD[pNMItemActivate->iItem].indexDuplicate, TRUE);
	*pResult = 0;
}


/*++
  Function: OnNMClickListGuidVar

  Parameters: pNMHDR - ptr to CListCtrl click info
			  pResult - ptr to result value

  Purpose: 
// TODO fix clipboard copy

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMClickListGuidVar(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CString t = m_cvListGuidVar.GetItemText(pNMItemActivate->iItem, 0);

	HGLOBAL hglbCopy;	
	// Open the clipboard, and empty it. 
	if (OpenClipboard()) {
		EmptyClipboard(); 

		// Allocate a global memory object for the text.
		hglbCopy = GlobalAlloc(GMEM_MOVEABLE, t.GetLength() * sizeof(TCHAR));
		if (hglbCopy != NULL)
		{
			// Lock the handle and copy the text to the buffer. 
			memcpy(GlobalLock(hglbCopy), t.GetBuffer(1024), t.GetLength() * sizeof(TCHAR)); 
			GlobalUnlock(hglbCopy); 
			// Place the handle on the clipboard. 
			SetClipboardData(CF_TEXT, hglbCopy);
		} else
			CloseClipboard();
	}

	*pResult = 0;
}


/*++
  Function: OnNMClickListPcdDef

  Parameters: pNMHDR - ptr to CListCtrl click info
			  pResult - ptr to result value

  Purpose: 
// TODO fix clipboard copy

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMClickListPcdDef(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CString t = m_cvListPcdDef.GetItemText(pNMItemActivate->iItem, 0);
	HGLOBAL hglbCopy;	
	// Open the clipboard, and empty it. 
	if (OpenClipboard()) {
		EmptyClipboard(); 

		// Allocate a global memory object for the text.
		hglbCopy = GlobalAlloc(GMEM_MOVEABLE, t.GetLength() * sizeof(TCHAR));
		if (hglbCopy != NULL)
		{
			// Lock the handle and copy the text to the buffer. 
			memcpy(GlobalLock(hglbCopy), t.GetBuffer(1024), t.GetLength() * sizeof(TCHAR)); 
			GlobalUnlock(hglbCopy); 
			// Place the handle on the clipboard. 
			SetClipboardData(CF_TEXT, hglbCopy);
		} else
			CloseClipboard();
	}

	*pResult = 0;
}


/*++
  Function: OnNMClickListPcdUse

  Parameters: pNMHDR - ptr to CListCtrl click info
			  pResult - ptr to result value

  Purpose: 
// TODO fix clipboard copy

  Returns: *pResult=0 if event handled
--*/
void CEDKIIBuildDataViewerDlg::OnNMClickListPcdUse(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	CString t = m_cvListPcdUse.GetItemText(pNMItemActivate->iItem, 0);
	*pResult = 0;
}
