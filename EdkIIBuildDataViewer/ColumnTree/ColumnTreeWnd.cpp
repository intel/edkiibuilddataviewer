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


#include "stdafx.h"
#include "ColumnTreeWnd.h"
#include <shlwapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#ifndef TVS_NOHSCROLL
#define TVS_NOHSCROLL 0x8000	// IE 5.0 or higher required
#endif

IMPLEMENT_DYNAMIC(CColumnTreeWnd, CWnd)

BEGIN_MESSAGE_MAP(CColumnTreeWnd, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_NOTIFY(HDN_ITEMCHANGED, HeaderID, OnHeaderItemChanged)
	ON_NOTIFY(HDN_DIVIDERDBLCLICK, HeaderID, OnHeaderDividerDblClick)
	ON_NOTIFY(NM_CUSTOMDRAW, TreeID, OnTreeCustomDraw)
END_MESSAGE_MAP()


CColumnTreeWnd::CColumnTreeWnd()
{
}

CColumnTreeWnd::~CColumnTreeWnd()
{
}


int CColumnTreeWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;

	// create tree and header controls as children
	m_Tree.Create(WS_CHILD | WS_VISIBLE | /*TVS_NOHSCROLL |*/ TVS_NOTOOLTIPS, CRect(), this, TreeID);
	m_Header.Create(WS_CHILD | WS_VISIBLE | HDS_FULLDRAG, CRect(), this, HeaderID);

	// set correct font for the header
	CFont* pFont = m_Tree.GetFont();
	m_Header.SetFont(pFont);

	// check if the common controls library version 6.0 is available
	BOOL bIsComCtl6 = FALSE;

	HMODULE hComCtlDll = LoadLibrary(_T("comctl32.dll"));

	if (hComCtlDll)
	{
		typedef HRESULT (CALLBACK *PFNDLLGETVERSION)(DLLVERSIONINFO*);

		PFNDLLGETVERSION pfnDllGetVersion = (PFNDLLGETVERSION)GetProcAddress(hComCtlDll, "DllGetVersion");

		if (pfnDllGetVersion)
		{
			DLLVERSIONINFO dvi;
			ZeroMemory(&dvi, sizeof(dvi));
			dvi.cbSize = sizeof(dvi);

			HRESULT hRes = (*pfnDllGetVersion)(&dvi);

			if (SUCCEEDED(hRes) && dvi.dwMajorVersion >= 6)
				bIsComCtl6 = TRUE;
		}

		FreeLibrary(hComCtlDll);
	}

	// calculate correct header's height
	CDC* pDC = GetDC();
	pDC->SelectObject(pFont);
	CSize szExt = pDC->GetTextExtent(_T("A"));
	m_cyHeader = szExt.cy + (bIsComCtl6 ? 7 : 4);
	ReleaseDC(pDC);

	// offset from column start to text start
	m_xOffset = bIsComCtl6 ? 9 : 6;

	m_xPos = 0;
	UpdateColumns();

	return 0;
}

void CColumnTreeWnd::OnPaint()
{
	// do nothing
	CPaintDC dc(this);
}

BOOL CColumnTreeWnd::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;
}

void CColumnTreeWnd::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	UpdateScroller();
	RepositionControls();
}

void CColumnTreeWnd::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.Width();

	int xLast = m_xPos;

	switch (nSBCode)
	{
	case SB_LINELEFT:
		m_xPos -= 15;
		break;
	case SB_LINERIGHT:
		m_xPos += 15;
		break;
	case SB_PAGELEFT:
		m_xPos -= cx;
		break;
	case SB_PAGERIGHT:
		m_xPos += cx;
		break;
	case SB_LEFT:
		m_xPos = 0;
		break;
	case SB_RIGHT:
		m_xPos = m_cxTotal - cx;
		break;
	case SB_THUMBTRACK:
		m_xPos = nPos;
		break;
	}

	if (m_xPos < 0)
		m_xPos = 0;
	else if (m_xPos > m_cxTotal - cx)
		m_xPos = m_cxTotal - cx;

	if (xLast == m_xPos)
		return;

	SetScrollPos(SB_HORZ, m_xPos);
	RepositionControls();
}


void CColumnTreeWnd::OnHeaderItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	UpdateColumns();

	m_Tree.Invalidate();
}

void CColumnTreeWnd::OnHeaderDividerDblClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMHEADER* pNMHeader = (NMHEADER*)pNMHDR;

	AdjustColumnWidth(pNMHeader->iItem, TRUE);
}

void CColumnTreeWnd::OnTreeCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	NMCUSTOMDRAW* pNMCustomDraw = (NMCUSTOMDRAW*)pNMHDR;
	NMTVCUSTOMDRAW* pNMTVCustomDraw = (NMTVCUSTOMDRAW*)pNMHDR;

	switch (pNMCustomDraw->dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;

	case CDDS_ITEMPREPAINT:
		*pResult = CDRF_DODEFAULT | CDRF_NOTIFYPOSTPAINT;
		break;

	case CDDS_ITEMPOSTPAINT:
		{
			HTREEITEM hItem = (HTREEITEM)pNMCustomDraw->dwItemSpec;
			CRect rcItem = pNMCustomDraw->rc;

			if (rcItem.IsRectEmpty())
			{
				// nothing to paint
				*pResult = CDRF_DODEFAULT;
				break;
			}

			CDC dc;
			dc.Attach(pNMCustomDraw->hdc);

			CRect rcLabel;
			m_Tree.GetItemRect(hItem, &rcLabel, TRUE);

			COLORREF crTextBk = pNMTVCustomDraw->clrTextBk;
			COLORREF crText = pNMTVCustomDraw->clrText;
			COLORREF crWnd = GetSysColor(COLOR_WINDOW);

			// clear the original label rectangle
			CRect rcClear = rcLabel;
			if (rcClear.left > m_arrColWidths[0] - 1)
				rcClear.left = m_arrColWidths[0] - 1;
			dc.FillSolidRect(&rcClear, crWnd);

			int nColsCnt = m_Header.GetItemCount();

			// draw horizontal lines...
			int xOffset = 0;
			for (int i=0; i<nColsCnt; i++)
			{
				xOffset += m_arrColWidths[i];
				rcItem.right = xOffset-1;
				dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_RIGHT);
			}
			// ...and the vertical ones
			dc.DrawEdge(&rcItem, BDR_SUNKENINNER, BF_BOTTOM);

			CString strText = m_Tree.GetItemText(hItem);
			CString strSub;
			AfxExtractSubString(strSub, strText, 0, '\t');

			// calculate main label's size
			CRect rcText(0,0,0,0);
			dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_CALCRECT);
			rcLabel.right = min(rcLabel.left + rcText.right + 4, m_arrColWidths[0] - 4);

			CRect rcBack = rcLabel;
			if (GetWindowLong(m_Tree.m_hWnd, GWL_STYLE) & TVS_FULLROWSELECT)
			{
				int nWidth = 0;
				for (int i=0; i<nColsCnt; i++)
					nWidth += m_arrColWidths[i];
				rcBack.right = nWidth - 1;
				if (rcBack.left > m_arrColWidths[0] - 1)
					rcBack.left = m_arrColWidths[0] - 1;
			}

			if (rcBack.Width() < 0)
				crTextBk = crWnd;
			if (crTextBk != crWnd)	// draw label's background
				dc.FillSolidRect(&rcBack, crTextBk);

			// draw focus rectangle if necessary
			if (pNMCustomDraw->uItemState & CDIS_FOCUS)
				dc.DrawFocusRect(&rcBack);

			// draw main label
			rcText = rcLabel;
			rcText.DeflateRect(2, 1);
			dc.SetTextColor(crText);
			dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_END_ELLIPSIS);

			xOffset = m_arrColWidths[0];
			dc.SetBkMode(TRANSPARENT);

			if (!(GetWindowLong(m_Tree.m_hWnd, GWL_STYLE) & TVS_FULLROWSELECT))
				dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

			// draw other columns text
			for (int i=1; i<nColsCnt; i++)
			{
				if (AfxExtractSubString(strSub, strText, i, '\t'))
				{
					rcText = rcLabel;
					rcText.left = xOffset;
					rcText.right = xOffset + m_arrColWidths[i];
					rcText.DeflateRect(m_xOffset, 1, 2, 1);
					dc.DrawText(strSub, &rcText, DT_NOPREFIX | DT_END_ELLIPSIS);
				}
				xOffset += m_arrColWidths[i];
			}

			dc.Detach();
		}
		*pResult = CDRF_DODEFAULT;
		break;

	default:
		*pResult = CDRF_DODEFAULT;
	}
}

void CColumnTreeWnd::UpdateColumns()
{
	m_cxTotal = 0;
	m_arrColWidths.RemoveAll();

	HDITEM hditem;
	hditem.mask = HDI_WIDTH;
	int nCnt = m_Header.GetItemCount();

	// get column widths from the header control
	for (int i=0; i<nCnt; i++)
	{
		if (m_Header.GetItem(i, &hditem))
		{
			m_arrColWidths.Add(hditem.cxy);
			m_cxTotal += hditem.cxy;
			if (i==0)
				m_Tree.m_cxFirstCol = hditem.cxy;
		}
	}
	m_Tree.m_cxTotal = m_cxTotal;

	UpdateScroller();
	RepositionControls();
}

void CColumnTreeWnd::UpdateScroller()
{
	CRect rcClient;
	GetClientRect(&rcClient);
	int cx = rcClient.Width();

	int lx = m_xPos;

	if (m_xPos > m_cxTotal - cx)
		m_xPos = m_cxTotal - cx;
	if (m_xPos < 0)
		m_xPos = 0;

	SCROLLINFO scrinfo;
	scrinfo.cbSize = sizeof(scrinfo);
	scrinfo.fMask = SIF_PAGE | SIF_POS | SIF_RANGE;
	scrinfo.nPage = cx;
	scrinfo.nMin = 0;
	scrinfo.nMax = m_cxTotal;
	scrinfo.nPos = m_xPos;
	SetScrollInfo(SB_HORZ, &scrinfo);
}

void CColumnTreeWnd::RepositionControls()
{
	// reposition child controls
	if (m_Tree.m_hWnd)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		int cx = rcClient.Width();
		int cy = rcClient.Height();

		// move to a negative offset if scrolled horizontally
		int x = 0;
		if (cx < m_cxTotal)
		{
			x = GetScrollPos(SB_HORZ);
			cx += x;
		}
		m_Header.MoveWindow(-x, 0, cx, m_cyHeader);
		m_Tree.MoveWindow(-x, m_cyHeader, cx, cy-m_cyHeader);
	}
}

BOOL CColumnTreeWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	if (CWnd::OnNotify(wParam, lParam, pResult))
		return TRUE;

	// route notifications from CTreeCtrl to the parent window
	if (wParam == TreeID)
	{
/*
// old code, doesnt' report subItem double-click
		NMHDR* pNMHDR = (NMHDR*)lParam;
		pNMHDR->hwndFrom = m_hWnd;
		pNMHDR->idFrom = GetDlgCtrlID();
		*pResult = GetParent()->SendMessage(WM_NOTIFY, pNMHDR->idFrom, lParam);
*/
// start modified code: report subItem double-click for point in message against header X coords
		// NMITEMACTIVATE is sent as lParam from child control instead of NMHDR. This allows more data to be propagated up.
		LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(lParam);

		if (pNMItemActivate->hdr.code == NM_DBLCLK) {
			RECT rect;
			m_Tree.GetItemRect(m_Tree.GetRootItem(), &rect, TRUE);

			// If the double-click is outside of the first column's text, then set iItem to the HTREEITEM of the selected item
			// because it wasn't set by the tree's double-click handler.  This value can be used by the parent control to determine
			// which HTREEITEM was clicked, and iSubItem indicates the column #.  This allows double-click of tree control columns
			// to be processed by the app.
			if (pNMItemActivate->ptAction.x < rect.left) {
				pNMItemActivate->iItem = (int) m_Tree.GetSelectedItem();
			}

			pNMItemActivate->iSubItem = GetHitSubItem(pNMItemActivate->ptAction);
		}

		pNMItemActivate->hdr.hwndFrom = m_hWnd;
		pNMItemActivate->hdr.idFrom = GetDlgCtrlID();
		*pResult = GetParent()->SendMessage(WM_NOTIFY, pNMItemActivate->hdr.idFrom, lParam);
// end modified code
	}
	return TRUE;
}

void CColumnTreeWnd::AdjustColumnWidth(int nColumn, BOOL bIgnoreCollapsed)
{
// start modified code: determine max width by scanning all root and children items
	HTREEITEM hItem = m_Tree.GetRootItem();
	int nMaxWidth = 0;
	while (hItem)
	{
		int nSubWidth = GetMaxColumnWidth(hItem, nColumn, 0, bIgnoreCollapsed);
		if (nSubWidth > nMaxWidth)
			nMaxWidth = nSubWidth;
		hItem = m_Tree.GetNextSiblingItem(hItem);
	}
// end modified code

// old code below: scans children of first root level item for max width
//	int nMaxWidth = GetMaxColumnWidth(m_Tree.GetRootItem(), nColumn, 0, bIgnoreCollapsed);

	HDITEM hditem;
	hditem.mask = HDI_WIDTH;
	m_Header.GetItem(nColumn, &hditem);
	hditem.cxy = nMaxWidth + 20;
	m_Header.SetItem(nColumn, &hditem);
}

int CColumnTreeWnd::GetMaxColumnWidth(HTREEITEM hItem, int nColumn, int nDepth, BOOL bIgnoreCollapsed)
{
	int nMaxWidth = 0;

	CString strText = m_Tree.GetItemText(hItem);
	CString strSub;
	if (AfxExtractSubString(strSub, strText, nColumn, '\t'))
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);
		CFont* pOldFont = dc.SelectObject(m_Tree.GetFont());

		// calculate text width
		nMaxWidth = dc.GetTextExtent(strSub).cx;

		dc.SelectObject(pOldFont);
	}

	// add indent and image space if first column
	if (nColumn == 0)
	{
		int nIndent = nDepth;

		if (GetWindowLong(m_Tree.m_hWnd, GWL_STYLE) & TVS_LINESATROOT)
			nIndent++;

		int nImage, nSelImage;
		m_Tree.GetItemImage(hItem, nImage, nSelImage);
		if (nImage >= 0)
			nIndent++;

		nMaxWidth += nIndent * m_Tree.GetIndent();
	}

	if (!bIgnoreCollapsed || (m_Tree.GetItemState(hItem, TVIS_EXPANDED) & TVIS_EXPANDED))
	{
		// process child items recursively
		HTREEITEM hSubItem = m_Tree.GetChildItem(hItem);
		while (hSubItem)
		{
			int nSubWidth = GetMaxColumnWidth(hSubItem, nColumn, nDepth + 1, bIgnoreCollapsed);
			if (nSubWidth > nMaxWidth)
				nMaxWidth = nSubWidth;

			hSubItem = m_Tree.GetNextSiblingItem(hSubItem);
		}
	}

	return nMaxWidth;
}

int CColumnTreeWnd::InsertColumn(int nCol, LPCTSTR lpszColumnHeading, int nFormat, int nWidth, int nSubItem)
{
	// update the header control in upper-right corner
	// to make it look the same way as main header

	CHeaderCtrl& header = GetHeaderCtrl();

	HDITEM hditem;
	hditem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	hditem.fmt = nFormat;
	hditem.cxy = nWidth;
	hditem.pszText = (LPTSTR)lpszColumnHeading;
	int indx =  header.InsertItem(nCol, &hditem);

	if(m_Header.GetItemCount()>0) 
	{
		// if the main header contains items, 
		// insert an item to m_Header2
		hditem.pszText = _T("");
		hditem.cxy = GetSystemMetrics(SM_CXVSCROLL)+5;
		m_Header.InsertItem(0,&hditem);
	}
	UpdateColumns();
	
	return indx;
}

BOOL  CColumnTreeWnd::DeleteColumn(int nCol)
{
	// update the header control in upper-right corner
	// to make it look the same way as main header

	BOOL bResult = m_Header.DeleteItem(nCol);
	if(m_Header.GetItemCount()==0) 
	{
		m_Header.DeleteItem(0);
	}

	UpdateColumns();
	return bResult;
}

CString CColumnTreeWnd::GetItemText(HTREEITEM hItem, int nColumn)
{
	// retreive and return the substring from tree item's text
	CString szText = m_Tree.GetItemText(hItem);
	CString szSubItem;
	AfxExtractSubString(szSubItem,szText,nColumn,'\t');
	return szSubItem;
}

void CColumnTreeWnd::SetItemText(HTREEITEM hItem,int nColumn,LPCTSTR lpszItem)
{
	CString szText = m_Tree.GetItemText(hItem);
	CString szNewText, szSubItem;
	int i;
	for(i=0;i<m_Header.GetItemCount();i++)
	{
		AfxExtractSubString(szSubItem,szText,i,'\t');
		if(i!=nColumn) szNewText+=szSubItem+_T("\t");
		else szNewText+=CString(lpszItem)+_T("\t");
	}
	m_Tree.SetItemText(hItem,szNewText);
}

// start modified code: additional methods for retrieving control data
int CColumnTreeWnd::GetTreeItemData(UINT nItem, DWORD* pData, BOOL bWalkChildren /*= FALSE*/)
{
	UINT count = GetTreeCtrl().GetCount();
	if (count == 0) return -1;
	if (nItem + 1 > count) return -1;

	UINT i = 0;
	HTREEITEM hItem = GetTreeCtrl().GetRootItem();
	HTREEITEM hChild;
	while (1) {
		if (i++ == nItem) {
			*pData = GetTreeCtrl().GetItemData(hItem);
			return 0;
		}

		if (bWalkChildren) {
			if (GetTreeCtrl().ItemHasChildren(hItem)) {
				hChild = GetTreeCtrl().GetChildItem(hItem);
				while (1) {
					if (i++ == nItem) {
						*pData = GetTreeCtrl().GetItemData(hChild);
						return 0;
					}

					hChild = GetTreeCtrl().GetNextSiblingItem(hChild);
					if (hChild == NULL)
						break;
				}
			}
		}

		hItem = GetTreeCtrl().GetNextItem(hItem, TVGN_NEXT);
		if (hItem == NULL)
			break;
	}

	return -1;
}

int CColumnTreeWnd::GetTreeItem(HTREEITEM hItem, UINT* pItem, BOOL bWalkChildren /*= FALSE*/)
{
	UINT count = GetTreeCtrl().GetCount();
	if (count == 0) return -1;

	UINT i = 0;
	HTREEITEM hFindItem = GetTreeCtrl().GetRootItem();
	HTREEITEM hChild;
	while (1) {
		if (hFindItem == hItem) {
			*pItem = i;
			return 0;
		}
		i++;

		if (bWalkChildren) {
			if (GetTreeCtrl().ItemHasChildren(hFindItem)) {
				hChild = GetTreeCtrl().GetChildItem(hFindItem);
				while (1) {
					if (hChild == hItem) {
						*pItem = i;
						return 0;
					}
					i++;

					hChild = GetTreeCtrl().GetNextSiblingItem(hChild);
					if (hChild == NULL)
						break;
				}
			}
		}

		hFindItem = GetTreeCtrl().GetNextItem(hFindItem, TVGN_NEXT);
		if (hFindItem == NULL)
			break;
	}

	return -1;
}

int CColumnTreeWnd::GetTreeItem(HTREEITEM* phItem, UINT nItem, BOOL bWalkChildren /*= FALSE*/)
{
	UINT count = GetTreeCtrl().GetCount();
	if (count == 0) return -1;

	UINT i = 0;
	HTREEITEM hFindItem = GetTreeCtrl().GetRootItem();
	HTREEITEM hChild;
	while (1) {
		if (i == nItem) {
			*phItem = hFindItem;
			return 0;
		}
		i++;

		if (bWalkChildren) {
			if (GetTreeCtrl().ItemHasChildren(hFindItem)) {
				hChild = GetTreeCtrl().GetChildItem(hFindItem);
				while (1) {
					if (i == nItem) {
						*phItem = hChild;
						return 0;
					}
					i++;

					hChild = GetTreeCtrl().GetNextSiblingItem(hChild);
					if (hChild == NULL)
						break;
				}
			}
		}

		hFindItem = GetTreeCtrl().GetNextItem(hFindItem, TVGN_NEXT);
		if (hFindItem == NULL)
			break;
	}

	return -1;
}

int CColumnTreeWnd::GetHitSubItem(CPoint point)
{
	int	i, x = 0;
	for (i = 0; i < m_Header.GetItemCount(); i++) {
		if (point.x >= x && point.x <= x + m_arrColWidths[i]) {
			return i;
		}
		x += m_arrColWidths[i];
	}

	return -1;
}
// end modified code
