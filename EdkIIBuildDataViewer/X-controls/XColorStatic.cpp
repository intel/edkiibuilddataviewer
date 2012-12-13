// Code located at http://www.codeproject.com/Articles/5242/XColorStatic-a-colorizing-static-control
// License located at http://www.codeproject.com/info/EULA.aspx
// Code change blocks marked with // start modified code: and // end modified code

// XColorStatic.cpp  Version 1.0
//
// Author:  Hans Dietrich
//          hdietrich2@hotmail.com
//
// This software is released into the public domain.
// You are free to use it in any way you like.
//
// This software is provided "as is" with no expressed
// or implied warranty.  I accept no liability for any
// damage or loss of business that this software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XColorStatic.h"
#include "FontSize.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CXColorStatic

BEGIN_MESSAGE_MAP(CXColorStatic, CStatic)
	//{{AFX_MSG_MAP(CXColorStatic)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
// start modified code: add vertical scrollbar
    ON_WM_VSCROLL()
// end modified code
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////
// ctor
CXColorStatic::CXColorStatic()
{
	m_rgbText       = GetSysColor(COLOR_BTNTEXT);
	m_rgbBackground = GetSysColor(COLOR_BTNFACE);
	m_pBrush        = new CBrush(m_rgbBackground);
	m_bBold         = FALSE;
// start modified code: add underline style
	m_bUnderline    = FALSE;
// end modified code
	m_hIcon         = NULL;
	m_nXMargin = m_nYMargin = 0;
// start modified code: add vertical scrollbar
// The amount of scroll to make when clicking the UP or DOWN arrows on the scroll bar.
	m_iLineHeight = 8;
// end modified code
}

///////////////////////////////////////////////////////////////////////////////
// dtor
CXColorStatic::~CXColorStatic()
{
	TRACE(_T("in CXColorStatic::~CXColorStatic\n"));

	if (m_font.GetSafeHandle())
		m_font.DeleteObject();

	if (m_pBrush)
	{
		m_pBrush->DeleteObject();
		delete m_pBrush;
	}
	m_pBrush = NULL;
}

///////////////////////////////////////////////////////////////////////////////
// PreSubclassWindow
void CXColorStatic::PreSubclassWindow() 
{
	TRACE(_T("in CXColorStatic::PreSubclassWindow\n"));
	
	// get current font
	CFont* pFont = GetFont();
	if (!pFont)
	{
		HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (hFont == NULL)
			hFont = (HFONT) GetStockObject(ANSI_VAR_FONT);
		if (hFont)
			pFont = CFont::FromHandle(hFont);
	}
	ASSERT(pFont);
	ASSERT(pFont->GetSafeHandle());

	// create the font for this control
	LOGFONT lf;
	pFont->GetLogFont(&lf);
	m_font.CreateFontIndirect(&lf);
}

///////////////////////////////////////////////////////////////////////////////
// OnPaint
void CXColorStatic::OnPaint()
{
	CPaintDC dc(this); // device context for painting
 
	dc.SaveDC();
 
	dc.SetTextColor(m_rgbText);
// start modified code: add transparent background
	if(!(GetExStyle() & WS_EX_TRANSPARENT))
	{
		dc.SetBkColor(m_rgbBackground);
		dc.SetBkMode(OPAQUE);
	}
	else
	{
		dc.SetBkMode(TRANSPARENT);
	}
// end modified code
	dc.SelectObject(m_pBrush);
 
	CRect rect;
	GetClientRect(rect);
 
	// cannot have both an icon and text
	if (m_hIcon)
	{
		int nIconX = ::GetSystemMetrics(SM_CXICON);
		int nIconY = ::GetSystemMetrics(SM_CYICON);
 
		rect.left = rect.left + (rect.Width() - nIconX) / 2;
		rect.top = rect.top + (rect.Height() - nIconY) / 2;
 
		dc.DrawIcon(rect.left, rect.top, m_hIcon);
	}
	else
	{
		dc.SelectObject(&m_font);

		// get static's text
		CString strText = _T("");
		GetWindowText(strText);

		UINT nFormat = 0;
		DWORD dwStyle = GetStyle();

		// set DrawText format from static style settings
		if (dwStyle & SS_CENTER)
			nFormat |= DT_CENTER;
		else if (dwStyle & SS_LEFT)
			nFormat |= DT_LEFT;
		else if (dwStyle & SS_RIGHT)
			nFormat |= DT_RIGHT;

		if (dwStyle & SS_CENTERIMAGE) // vertical centering ==> single line only
			nFormat |= DT_VCENTER | DT_SINGLELINE;
		else
			nFormat |= DT_WORDBREAK;

		// Added to expand tabs...
		if (strText.Find(_T('\t')) != -1)
			nFormat |= DT_EXPANDTABS;

		//
		// BEGIN SCROLL CODE
		//
		// Create a rect above our target rect that will not allow drawing to.
		// We will make 200 pixels above our target non-drawing. That should give us enough
		// room to scroll text upwards.
		RECT excluderect;
		excluderect.top = rect.top - 200;
		excluderect.bottom = rect.top;
		excluderect.left = rect.left;
		excluderect.right = rect.right;
		dc.ExcludeClipRect(&excluderect);

		// Draw the text accounting for any scrolling of the scroll bar.
		rect.top -= GetScrollPos( SB_VERT );
		//
		// END SCROLL CODE
		//

		rect.left += m_nXMargin;
		rect.top += m_nYMargin;
		dc.DrawText(strText, rect, nFormat);

		CRect rWithTextRect, rMainRect;
		GetClientRect(&rMainRect);
		rWithTextRect = rMainRect;

		dc.DrawText(strText, &rWithTextRect, DT_CALCRECT | DT_WORDBREAK);

		ResetScrollBar(&rMainRect, &rWithTextRect);
	}

	dc.RestoreDC(-1);
}

///////////////////////////////////////////////////////////////////////////////
// OnEraseBkgnd
BOOL CXColorStatic::OnEraseBkgnd(CDC* pDC) 
{
	CRect cr;
	GetClientRect(cr); 
	pDC->FillRect(&cr, m_pBrush);

	return TRUE; //CStatic::OnEraseBkgnd(pDC);
}

///////////////////////////////////////////////////////////////////////////////
// SetFont
void CXColorStatic::SetFont(LOGFONT *pLogFont, BOOL bRedraw /*= TRUE*/)
{
	ASSERT(pLogFont);
	if (!pLogFont)
		return;

	if (m_font.GetSafeHandle())
		m_font.DeleteObject();

	LOGFONT lf = *pLogFont;

	lf.lfWeight = m_bBold ? FW_BOLD : FW_NORMAL;
// start modified code: add underline style
	lf.lfUnderline = m_bUnderline ? TRUE : FALSE;
// end modified code

	m_font.CreateFontIndirect(&lf);

	if (bRedraw)
		RedrawWindow();
}

///////////////////////////////////////////////////////////////////////////////
// SetFont
void CXColorStatic::SetFont(LPCTSTR lpszFaceName, 
							int nPointSize, 
							BOOL bRedraw /*= TRUE*/)
{
	// null face name is ok - we will use current font

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	if (lpszFaceName == NULL)
	{
		CFont *pFont = GetFont();
		ASSERT(pFont);
		pFont->GetLogFont(&lf);
	}
	else
	{
		_tcsncpy_s(lf.lfFaceName, lpszFaceName, sizeof(lf.lfFaceName)/sizeof(TCHAR)-1);
	}

	lf.lfHeight = GetFontHeight(nPointSize);

	SetFont(&lf, bRedraw);
}

///////////////////////////////////////////////////////////////////////////////
// SetFont
void CXColorStatic::SetFont(CFont *pFont, BOOL bRedraw /*= TRUE*/)
{
	ASSERT(pFont);
	if (!pFont)
		return;

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	pFont->GetLogFont(&lf);

	SetFont(&lf, bRedraw);
}

///////////////////////////////////////////////////////////////////////////////
// SetTextColor
void CXColorStatic::SetTextColor(COLORREF rgb, BOOL bRedraw /*= TRUE*/) 
{ 
	m_rgbText = rgb; 
	if (bRedraw)
		RedrawWindow();
}

///////////////////////////////////////////////////////////////////////////////
// SetBold
void CXColorStatic::SetBold(BOOL bFlag, BOOL bRedraw /*= TRUE*/)
{ 
	m_bBold = bFlag;

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	CFont *pFont = GetFont();
	ASSERT(pFont);
	pFont->GetLogFont(&lf);

	lf.lfWeight = m_bBold ? FW_BOLD : FW_NORMAL;
// start modified code: add underline style
	lf.lfUnderline = m_bUnderline ? TRUE : FALSE;
// end modified code

	SetFont(&lf, bRedraw);
}

// start modified code: add underline style
///////////////////////////////////////////////////////////////////////////////
// SetUnderline
void CXColorStatic::SetUnderline(BOOL bFlag, BOOL bRedraw /*= TRUE*/)
{ 
	m_bUnderline = bFlag;

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	CFont *pFont = GetFont();
	ASSERT(pFont);
	pFont->GetLogFont(&lf);

	lf.lfWeight = m_bBold ? FW_BOLD : FW_NORMAL;
	lf.lfUnderline = m_bUnderline ? TRUE : FALSE;

	SetFont(&lf, bRedraw);
}
// end modified code

///////////////////////////////////////////////////////////////////////////////
// SetBackgroundColor
void CXColorStatic::SetBackgroundColor(COLORREF rgb, BOOL bRedraw /*= TRUE*/) 
{ 
	m_rgbBackground = rgb; 
	if (m_pBrush)
	{
		m_pBrush->DeleteObject();
		delete m_pBrush;
	}
	m_pBrush = new CBrush(m_rgbBackground);
	if (bRedraw)
		RedrawWindow();
}

///////////////////////////////////////////////////////////////////////////////
// SetIcon
void CXColorStatic::SetIcon(HICON hIcon, BOOL bRedraw /*= TRUE*/)
{
	ASSERT(hIcon);

	m_hIcon = hIcon;
	if (bRedraw)
		RedrawWindow();
}

// start modified code: add vertical scrollbar
void CXColorStatic::ShowText(CString strText)
{
	SetWindowText(strText);
	RedrawWindow();
 
	// Reset the scroll bar position back to 0.
	SetScrollPos(SB_VERT, 0);
}
 
void CXColorStatic::ResetScrollBar(CRect* a_pMainRect, CRect* a_pWithTextRect)
{
	// Need for scrollbars?
	if(a_pMainRect->Height() >= a_pWithTextRect->Height())
	{
		ShowScrollBar( SB_VERT, FALSE ); // Hide it
		SetScrollPos( SB_VERT, 0 );
	}
	else
	{
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_PAGE | SIF_RANGE;
		si.nPage = a_pMainRect->Height();
		si.nMax = a_pWithTextRect->Height();
		si.nMin = 0 ;

		SetScrollInfo(SB_VERT, &si);
 
		EnableScrollBarCtrl( SB_VERT, TRUE );
	}
}
 
void CXColorStatic::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	int iScrollBarPos = GetScrollPos( SB_VERT );

	CRect rFrame;
	GetClientRect( rFrame );

	switch( nSBCode )
	{
	case SB_LINEUP:
		iScrollBarPos = max( iScrollBarPos - m_iLineHeight, 0 );
		break;

	case SB_LINEDOWN:
		iScrollBarPos = min( iScrollBarPos + m_iLineHeight,
		GetScrollLimit( SB_VERT ) );
		break;

	case SB_PAGEUP:
		iScrollBarPos = max( iScrollBarPos - rFrame.Height(), 0 );
		break;

	case SB_PAGEDOWN:
		iScrollBarPos = min( iScrollBarPos + rFrame.Height(),
		GetScrollLimit( SB_VERT ) );
		break;

	case SB_THUMBTRACK:
	case SB_THUMBPOSITION:
		iScrollBarPos = nPos;
		break;
	}

	SetScrollPos( SB_VERT, iScrollBarPos );
	Invalidate();
}
 
LRESULT CXColorStatic::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if ( message == WM_NCHITTEST || message == WM_NCLBUTTONDOWN || message == WM_NCLBUTTONDBLCLK )
		return ::DefWindowProc( m_hWnd, message, wParam, lParam );

	return CStatic::WindowProc(message, wParam, lParam);
}
// end modified code
