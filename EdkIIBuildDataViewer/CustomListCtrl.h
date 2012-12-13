/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// CustomListCtrl.h : header file

#pragma once

class CListCtrlEx : public CListCtrl
{
public:
	int GetColumnCount() const {return GetHeaderCtrl()->GetItemCount();};

	int GetSelectedItem() const {
		POSITION pos = GetFirstSelectedItemPosition();
		int index = -1;
		if (pos != NULL)
			index = GetNextSelectedItem(pos);
		return index;
	}
};


#define STYLE_COLOR_ROW		1

typedef struct _t_LIST_ITEM_STYLE
{
	DWORD		style;
	COLORREF	textColor;
  	COLORREF	bgColor;
} t_LIST_ITEM_STYLE;

class CCustomListCtrl : public CListCtrlEx
{
public:
	CCustomListCtrl();
	~CCustomListCtrl();

	void				SetRowTextColor(int nRow, COLORREF textColor, bool bRedraw = true);
	void				SetRowBgColor(int nRow, COLORREF bgColor, bool bRedraw = true);
	void				SetRowStyle(int nRow, DWORD nStyle, bool bRedraw = true);
	int					InsertItem(const LVITEM* pItem);
	int					InsertItem(int nItem, LPCTSTR lpszItem);
	int					InsertItem(UINT nMask, int nItem, LPCTSTR lpszItem, UINT nState, UINT nStateMask, int nImage, LPARAM lParam);
	BOOL				DeleteItem(int nItem);
	BOOL				DeleteAllItems();

private:
	CArray<t_LIST_ITEM_STYLE, t_LIST_ITEM_STYLE> m_itemStyle;

protected:
	afx_msg void OnCustomdraw(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_MESSAGE_MAP()
};
