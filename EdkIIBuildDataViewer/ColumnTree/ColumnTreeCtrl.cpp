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

#include "stdafx.h"
#include "ColumnTreeCtrl.h"


IMPLEMENT_DYNAMIC(CColumnTreeCtrl, CTreeCtrl)

CColumnTreeCtrl::CColumnTreeCtrl()
{
}

CColumnTreeCtrl::~CColumnTreeCtrl()
{
}


BEGIN_MESSAGE_MAP(CColumnTreeCtrl, CTreeCtrl)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()


void CColumnTreeCtrl::OnLButtonDown(UINT nFlags, CPoint point)
{
	// mask left click if outside the real item's label
	HandleMouse(WM_LBUTTONDOWN, nFlags, point);
}


void CColumnTreeCtrl::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	HandleMouse(WM_LBUTTONDBLCLK, nFlags, point);
}

void CColumnTreeCtrl::OnPaint()
{
	CPaintDC dc(this);

	CRect rcClient;
	GetClientRect(&rcClient);

	CDC dcMem;
	CBitmap bmpMem;

	// use temporary bitmap to avoid flickering
	dcMem.CreateCompatibleDC(&dc);
	if (bmpMem.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height()))
	{
		CBitmap* pOldBmp = dcMem.SelectObject(&bmpMem);

		// paint the window onto the memory bitmap
		CWnd::DefWindowProc(WM_PAINT, (WPARAM)dcMem.m_hDC, 0);

		// copy it to the window's DC
		dc.BitBlt(0, 0, rcClient.right, rcClient.bottom, &dcMem, 0, 0, SRCCOPY);

		dcMem.SelectObject(pOldBmp);

		bmpMem.DeleteObject();
	}
	dcMem.DeleteDC();
}

BOOL CColumnTreeCtrl::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;	// do nothing
}


void CColumnTreeCtrl::HandleMouse(UINT message, UINT nFlags, CPoint point)
{
	UINT fFlags;
	HTREEITEM hItem = HitTest(point, &fFlags);

// start modified code: placeholder for if an action is needed when the button is double-clicked
	// if expand/collapse button is clicked, then ...
	if (message == WM_LBUTTONDBLCLK && (fFlags & TVHT_ONITEMBUTTON)) {
	}
// end modified code

	// verify the hit result
	if (fFlags & (TVHT_ONITEMLABEL | TVHT_ONITEMRIGHT))
	{
		CRect rcItem;
		GetItemRect(hItem, &rcItem, TRUE);

		if (GetWindowLong(m_hWnd, GWL_STYLE) & TVS_FULLROWSELECT)
		{
			if (message == WM_LBUTTONDOWN)
				SetFocus();

			// ignore if outside all columns
			rcItem.right = m_cxTotal;
			if (!rcItem.PtInRect(point))
				return;

			// select or expand item
			if (message == WM_LBUTTONDOWN)
			{
				Select(hItem, TVGN_CARET);
			}
			else if (message == WM_LBUTTONDBLCLK)
			{
				// send the NM_DBLCLK notification
/*
// old code, doesnt' report subItem double-click
				NMHDR nmhdr;
				nmhdr.hwndFrom = m_hWnd;
				nmhdr.code = NM_DBLCLK;
				nmhdr.idFrom = GetDlgCtrlID();
				GetParent()->SendMessage(WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);
*/
// start modified code: report subItem double-click by notifying parent of X coord so it can do hittest against header X coords
				// Send NMITEMACTIVATE as lParam instead of NMHDR. This allows more data to be propagated up.
				NMITEMACTIVATE nmia;
				nmia.hdr.hwndFrom = m_hWnd;
				nmia.hdr.code = NM_DBLCLK;
				nmia.hdr.idFrom = GetDlgCtrlID();
				nmia.ptAction = point;
				nmia.iItem = (int) GetSelectedItem();
				// set lParam=0 when the item does not have children, and 1 when the item has children.
				nmia.lParam = ItemHasChildren(GetSelectedItem());
				GetParent()->SendMessage(WM_NOTIFY, nmia.hdr.idFrom, (LPARAM)&nmia);
// end modified code
				Expand(hItem, TVE_TOGGLE);
			}

			return;
		}
		else
		{
			// ignore if outside the first column
			rcItem.right = m_cxFirstCol;
			if (!rcItem.PtInRect(point))
			{
				if (message == WM_LBUTTONDOWN)
					SetFocus();
				return;
			}

			CString strSub;
			AfxExtractSubString(strSub, GetItemText(hItem), 0, _T('\t'));

			CDC* pDC = GetDC();
			pDC->SelectObject(GetFont());
			rcItem.right = rcItem.left + pDC->GetTextExtent(strSub).cx + 6;
			ReleaseDC(pDC);

			// ignore if outside the label's rectangle
			if (!rcItem.PtInRect(point))
			{
				if (message == WM_LBUTTONDOWN)
					SetFocus();
				return;
			}
		}
	}
	else
	{
		// check if the button or icon is hidden
		if (point.x >= m_cxFirstCol)
		{
			if (message == WM_LBUTTONDOWN)
				SetFocus();

			// ignore if outside all columns
			if (point.x > m_cxTotal)
				return;

			// select or expand item
			if (message == WM_LBUTTONDOWN)
			{
				Select(hItem, TVGN_CARET);
			}
			else if (message == WM_LBUTTONDBLCLK)
			{
				// send the NM_DBLCLK notification
				NMHDR nmhdr;
				nmhdr.hwndFrom = m_hWnd;
				nmhdr.idFrom = GetDlgCtrlID();
				nmhdr.code = NM_DBLCLK;
				GetParent()->SendMessage(WM_NOTIFY, nmhdr.idFrom, (LPARAM)&nmhdr);

				Expand(hItem, TVE_TOGGLE);
			}

			return;
		}
	}

	// pass message to the default procedure
	CWnd::DefWindowProc(message, nFlags, MAKELONG(point.x, point.y));
}
