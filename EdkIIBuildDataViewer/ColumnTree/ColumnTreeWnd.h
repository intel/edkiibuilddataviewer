// Code located at http://www.mimec.org/components/mfc
// License located at http://doc.mimec.org/articles/mfc/mctree/index.html
// Code change blocks marked with // start modified code: and // end modified code

/*********************************************************************
* Multi-Column Tree View, version 1.4 (July 7, 2005)
* Copyright (C) 2003-2005 Michal Mecinski.
*
* You may freely use and modify this code, but don't remove
* this copyright note.
*
* THERE IS NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, FOR
* THIS CODE. THE AUTHOR DOES NOT TAKE THE RESPONSIBILITY
* FOR ANY DAMAGE RESULTING FROM THE USE OF IT.
*
* E-mail: mimec@mimec.org
* WWW: http://www.mimec.org
********************************************************************/

#pragma once

#include "ColumnTreeCtrl.h"


class CColumnTreeWnd : public CWnd
{
	DECLARE_DYNAMIC(CColumnTreeWnd)
public:
	CColumnTreeWnd();
	virtual ~CColumnTreeWnd();

public:
	enum ChildrenIDs { HeaderID = 1, TreeID = 2 };

	void UpdateColumns();
	void AdjustColumnWidth(int nColumn, BOOL bIgnoreCollapsed);

	CTreeCtrl& GetTreeCtrl() { return m_Tree; }
	CHeaderCtrl& GetHeaderCtrl() { return m_Header; }

/*
* The CColumnTreeCtrl license below applies to these 4 functions, which were inserted into this file:
*
* int InsertColumn(int nCol,LPCTSTR lpszColumnHeading, int nFormat=0, int nWidth=-1, int nSubItem=-1);
* BOOL DeleteColumn(int nCol);
* CString GetItemText(HTREEITEM hItem, int nSubItem);
* void SetItemText(HTREEITEM hItem, int nSubItem, LPCTSTR lpszText);
*
* CColumnTreeCtrl code downloaded from http://www.codeproject.com/KB/tree/CColumnTreeCtrl.aspx
*/

/*****************************************************************************
* CColumnTreeCtrl
* Version: 1.1 
* Date: February 18, 2008
* Author: Oleg A. Krivtsov
* E-mail: olegkrivtsov@mail.ru
* Based on ideas implemented in Michal Mecinski's CColumnTreeWnd class 
* (see copyright note below).
*
*****************************************************************************/

/*********************************************************
* Multi-Column Tree View
* Version: 1.1
* Date: October 22, 2003
* Author: Michal Mecinski
* E-mail: mimec@mimec.w.pl
* WWW: http://www.mimec.w.pl
*
* You may freely use and modify this code, but don't remove
* this copyright note.
*
* There is no warranty of any kind, express or implied, for this class.
* The author does not take the responsibility for any damage
* resulting from the use of it.
*
* Let me know if you find this code useful, and
* send me any modifications and bug reports.
*
* Copyright (C) 2003 by Michal Mecinski
*********************************************************/
	int InsertColumn(int nCol,LPCTSTR lpszColumnHeading, int nFormat=0, int nWidth=-1, int nSubItem=-1);
	BOOL DeleteColumn(int nCol);
	CString GetItemText(HTREEITEM hItem, int nSubItem);
	void SetItemText(HTREEITEM hItem, int nSubItem, LPCTSTR lpszText);

// start modified code: additional methods for retrieving control data
	int GetTreeItemData(UINT nItem, DWORD* pData, BOOL bWalkChildren = FALSE);
	int GetTreeItem(HTREEITEM hItem, UINT* pItem, BOOL bWalkChildren = FALSE);
	int GetTreeItem(HTREEITEM* phItem, UINT nItem, BOOL bWalkChildren = FALSE);
	int GetHitSubItem(CPoint point);
// end modified code

protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

protected:
	void UpdateScroller();
	void RepositionControls();
	int GetMaxColumnWidth(HTREEITEM hItem, int nColumn, int nDepth, BOOL bIgnoreCollapsed);

protected:
	CColumnTreeCtrl m_Tree;
	CHeaderCtrl m_Header;
	int m_cyHeader;
	int m_cxTotal;
	int m_xPos;
	CArray<int, int> m_arrColWidths;
	int m_xOffset;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnHeaderDividerDblClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
};
