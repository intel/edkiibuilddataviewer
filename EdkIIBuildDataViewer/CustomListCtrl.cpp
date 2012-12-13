/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// CustomListCtrl.cpp : implementation file

#include "stdafx.h"
#include "CustomListCtrl.h"

CCustomListCtrl::CCustomListCtrl()
{
}

CCustomListCtrl::~CCustomListCtrl()
{
	m_itemStyle.RemoveAll();
}

BEGIN_MESSAGE_MAP(CCustomListCtrl, CListCtrl)
    ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomdraw)
END_MESSAGE_MAP()

int CCustomListCtrl::InsertItem(const LVITEM* pItem)
{
	t_LIST_ITEM_STYLE	lis;
	lis.style = 0;
	lis.bgColor = this->GetTextBkColor();
//	lis.bgColor = this->GetBkColor();
	lis.textColor = this->GetTextColor();
	m_itemStyle.Add(lis);
	return CListCtrl::InsertItem(pItem);
}

int CCustomListCtrl::InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam)
{
	LVITEM		lvitem;
	lvitem.mask = nMask;
	lvitem.iItem = nItem;
	lvitem.iSubItem = 0;
	lvitem.pszText = (LPWSTR) lpszItem;
	lvitem.cchTextMax = 0;
	if (nMask & LVIF_TEXT)
		lvitem.cchTextMax = _tcslen(lvitem.pszText);
	lvitem.state = nState;
	lvitem.stateMask = nStateMask;
	lvitem.iImage = nImage;
	lvitem.lParam = lParam;

	return InsertItem(&lvitem);
}

int CCustomListCtrl::InsertItem(int nItem, LPCTSTR lpszItem)
{
	return InsertItem(LVIF_TEXT, nItem, lpszItem, 0, 0, 0, 0);
}

void CCustomListCtrl::OnCustomdraw(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVCUSTOMDRAW* cd = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

	*pResult = CDRF_DODEFAULT;

	switch (cd->nmcd.dwDrawStage)
	{
		case CDDS_PREPAINT:
			*pResult = CDRF_NOTIFYITEMDRAW;
			break;

		case CDDS_ITEMPREPAINT:
			{
				int row = cd->nmcd.dwItemSpec;
				if (m_itemStyle.GetAt(row).style & STYLE_COLOR_ROW)
				{
					cd->clrTextBk = m_itemStyle.GetAt(row).bgColor;
					cd->clrText = m_itemStyle.GetAt(row).textColor;
				}
			}
			break;

        default:
			break;
    }
}

void CCustomListCtrl::SetRowStyle(int nRow, DWORD nStyle, bool bRedraw /*= true*/)
{
	if (nRow > this->GetItemCount()) return;

	m_itemStyle.GetAt(nRow).style = nStyle;

	if (bRedraw)
		CListCtrl::Update(nRow);
}

void CCustomListCtrl::SetRowTextColor(int nRow, COLORREF textColor, bool bRedraw /*= true*/)
{
	if (nRow > this->GetItemCount()) return;
	
	m_itemStyle.GetAt(nRow).textColor = textColor;
	if (bRedraw)
		CListCtrl::Update(nRow);
}

void CCustomListCtrl::SetRowBgColor(int nRow, COLORREF bgColor, bool bRedraw /*= true*/)
{
	if (nRow > this->GetItemCount()) return;

	m_itemStyle.GetAt(nRow).bgColor = bgColor;
	if (bRedraw)
		CListCtrl::Update(nRow);
}

BOOL CCustomListCtrl::DeleteItem(int nItem)
{
	if (nItem > this->GetItemCount()) return FALSE;

	m_itemStyle.RemoveAt(nItem);
	return CListCtrl::DeleteItem(nItem);
}

BOOL CCustomListCtrl::DeleteAllItems()
{
	m_itemStyle.RemoveAll();
	return CListCtrl::DeleteAllItems();
}
