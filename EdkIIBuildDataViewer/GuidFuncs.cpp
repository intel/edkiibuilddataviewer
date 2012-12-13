/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// CheckGuids.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"

void CEDKIIBuildDataViewerDlg::CheckGuidsForDuplicates ()
/*++
  Function: CheckGuidsForDuplicates

  Parameters: none

  Purpose: right-click handler for 'GUID (variables)' radio selection; sets visible list row to
		   first declaration for item that is duplicated.

  Returns: none
--*/
{
	if (m_pwndProgress != NULL)
		delete m_pwndProgress;
	m_pwndProgress = new CXProgressWnd(this, _T("Checking GUIDs for duplicates"), FALSE, FALSE);
	m_pwndProgress->GoModal(this);

//	m_pwndProgress->SetText(_T("Detecting build configuration ..."));
//	m_pwndProgress->Show();

	delete m_pwndProgress;
	m_pwndProgress = NULL;
}

int CEDKIIBuildDataViewerDlg::SearchGuidPool (EFI_GUID Guid)
/*++
  Function: SearchGuidPool

  Parameters: none

  Purpose: Searches GUID pool for GUID.

  Returns:  index if found
           -1 if not found
--*/
{
	UINT32	i;
	for (i = 0; i < m_vGuidPool.size(); i++) {
		if (CompareGuid(&Guid, &m_vGuidPool[i].Guid) == 0) {
			return i;
		}
	}

	return -1;
}

int CEDKIIBuildDataViewerDlg::InsertGuidPool (EFI_GUID Guid, UINT8 GuidType, UINT32 *pIndex, CString filename, UINT32 lineNum)
/*++
  Function: InsertGuidPool

  Parameters: none

  Purpose: insert a GUID into the GUID pool.

  Returns:  1 if inserted and not a duplicate
            0 if inserted and is a duplicate
           -1 if not inserted
--*/
{
	t_GuidPool		GuidPool;

	GuidPool.Guid = Guid;
	GuidPool.GuidType = GuidType;
	GuidPool.Filename = filename;
	GuidPool.LineNum = lineNum;
/*
	vector<t_GuidPool>::iterator itGuidPool;
	for (itGuidPool = m_vGuidPool.begin(); m_vGuidPool != m_vGUID.end(); m_vGuidPool++) {
		if (CompareGuid(&Guid, &itGuidPool->Guid) == 0) {
			*pIndex = itGuidPool.
			break;
		}
	}
*/
	*pIndex = m_vGuidPool.size();

	UINT32	i;
	for (i = 0; i < m_vGuidPool.size(); i++) {
		if (CompareGuid(&Guid, &m_vGuidPool[i].Guid) == 0) {
			GuidPool.bDuplicate = TRUE;
			GuidPool.indexDuplicate = i;
			m_vGuidPool.push_back(GuidPool);
			return 0;
		}
	}

	GuidPool.bDuplicate = FALSE;
	m_vGuidPool.push_back(GuidPool);
	return 1;
}

int CEDKIIBuildDataViewerDlg::InsertGuidPool (CString Guid, UINT8 GuidType, UINT32 *pIndex, CString filename, UINT32 lineNum)
/*++
  Function: InsertGuidPool

  Parameters: none

  Purpose: insert a GUID into the GUID pool.

  Returns:  1 if inserted and not a duplicate
            0 if inserted and is a duplicate
           -1 if not inserted
--*/
{
	CStringArray	tokenArray;
	int				curPos = 0;
	CString			resToken, last8Str;
	t_GuidPool		GuidPool;

	Guid.Trim();

	// GUID format is 1E73767F-8F52-4603-AEB4-F29B510B6766
	if (Guid.Find(_T('-')) != -1) {
		while ((resToken = Guid.Tokenize(_T("-"), curPos)) != _T(""))
			tokenArray.Add(resToken);
		if (tokenArray.GetSize() != 5)
			return -1;

		// Save last 2 strings into a single string, then loop over the 8 bytes.
		last8Str.Format(_T("%s%s"), tokenArray.GetAt(3), tokenArray.GetAt(4));
	// GUID format is { 0x8BE4DF61, 0x93CA, 0x11D2, { 0xAA, 0x0D, 0x00, 0xE0, 0x98, 0x03, 0x2B, 0x8C }}
	} else if (Guid.Find(_T('{')) != -1) {
		while ((resToken = Guid.Tokenize(_T("={}, \t"), curPos)) != _T("")) {
			// discard GUID name if it's part of string
			if (resToken.Find(_T("0x")) == 0 || resToken.Find(_T("0X")) == 0) {
				resToken.Delete(0, 2);
				tokenArray.Add(resToken);
			} else {
				GuidPool.GuidName = resToken;
			}
		}
		if (tokenArray.GetSize() != 11)
			return -1;

		// Save last 8 bytes into a single string, then loop over the 8 bytes.
		last8Str.Format(_T("%s%s%s%s%s%s%s%s"), tokenArray.GetAt(3), tokenArray.GetAt(4), tokenArray.GetAt(5), tokenArray.GetAt(6),
			tokenArray.GetAt(7), tokenArray.GetAt(8), tokenArray.GetAt(9), tokenArray.GetAt(10));
	} else
		return -1;

	UINT32			counter;

	// set Package Guid
	GuidPool.Guid.Data1 = _tcstoul(tokenArray.GetAt(0), 0, 16);
	GuidPool.Guid.Data2 = (UINT16) _tcstoul(tokenArray.GetAt(1), 0, 16);
	GuidPool.Guid.Data3 = (UINT16) _tcstoul(tokenArray.GetAt(2), 0, 16);

	for (counter = 0; counter < 8; counter++) {
		resToken = last8Str.Left(2);
		last8Str.Delete(0, 2);
		GuidPool.Guid.Data4[counter] = (UINT8) _tcstoul(resToken, 0, 16);
	}

	GuidPool.GuidType = GuidType;
	GuidPool.Filename = filename;
	GuidPool.LineNum = lineNum;

	*pIndex = m_vGuidPool.size();

	UINT32	i;
	for (i = 0; i < m_vGuidPool.size(); i++) {
		if (CompareGuid(&GuidPool.Guid, &m_vGuidPool[i].Guid) == 0) {
			GuidPool.bDuplicate = TRUE;
			GuidPool.indexDuplicate = i;
			m_vGuidPool.push_back(GuidPool);
			return 0;
		}
	}

	GuidPool.bDuplicate = FALSE;
	m_vGuidPool.push_back(GuidPool);
	return 1;
}

int CEDKIIBuildDataViewerDlg::CompareGuid (IN EFI_GUID *Guid1, IN EFI_GUID *Guid2)
/*++
  Function: CompareGuid

  Parameters: Guid1 - first GUID
              Guid2 - second GUID

  Purpose: Compares two GUIDs.

  Returns: 
    =  0  if Guid1 == Guid2
    != 0  if Guid1 != Guid2 
--*/
{
	INT32 *g1;
	INT32 *g2;
	INT32 r;

	//
	// Compare 32 bits at a time
	//
	g1  = (INT32 *) Guid1;
	g2  = (INT32 *) Guid2;

	r   = g1[0] - g2[0];
	r  |= g1[1] - g2[1];
	r  |= g1[2] - g2[2];
	r  |= g1[3] - g2[3];

	return r;
}

int CEDKIIBuildDataViewerDlg::GuidPoolValueAsString (CString &str, UINT32 index)
/*++
  Function: GuidPoolValueAsString

  Parameters: none

  Purpose: convert a GUID value in pool to string.

  Returns: 1 if converted, 0 if not.
--*/
{
	if (index > m_vGuidPool.size())
		return 0;

	str.Format(_T("%08X-%04X-%04X-%02X%02X%02X%02X%02X%02X%02X%02X"), m_vGuidPool[index].Guid.Data1, m_vGuidPool[index].Guid.Data2, m_vGuidPool[index].Guid.Data3,
		m_vGuidPool[index].Guid.Data4[0], m_vGuidPool[index].Guid.Data4[1], m_vGuidPool[index].Guid.Data4[2], m_vGuidPool[index].Guid.Data4[3], 
		m_vGuidPool[index].Guid.Data4[4], m_vGuidPool[index].Guid.Data4[5], m_vGuidPool[index].Guid.Data4[6], m_vGuidPool[index].Guid.Data4[7]);
	return 1;
}
