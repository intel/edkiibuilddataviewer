// Code located at http://www.codeproject.com/Articles/29016/XGroupBox-an-MFC-groupbox-control-to-display-text
// License located at http://www.codeproject.com/info/EULA.aspx

// XGroupBox.cpp  Version 1.0 - see article at www.codeproject.com
// http://www.codeproject.com/Articles/29016/XGroupBox-an-MFC-groupbox-control-to-display-text
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// Description:
//     XGroupBox is an MFC control that displays a flicker-free groupbox 
//     with text and/or icon.  You can use this to display either a 
//     standard groupbox or a header-only groupbox.
//
// History
//     Version 1.0 - 2008 September 2
//     - Initial public release
//
// License:
//     This software is released under the Code Project Open License (CPOL),
//     which may be found here:  http://www.codeproject.com/info/eula.aspx
//     You are free to use this software in any way you like, except that you 
//     may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this 
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XGroupBox.h"
#include "XVisualStyles.h"

#ifndef __noop
#if _MSC_VER < 1300
#define __noop ((void)0)
#endif
#endif

#undef TRACE
#undef TRACERECT
#define TRACE __noop
#define TRACERECT __noop

//=============================================================================
// if you want to see the TRACE output, uncomment this line:
//#include "XTrace.h"
//=============================================================================

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4996)			// disable bogus deprecation warning

//=============================================================================
static CXVisualStyles g_xpStyle;
COLORREF CXGroupBox::m_defaultGrayPalette[256] = { 0 };
//=============================================================================

//=============================================================================
// set resource handle (in case used in DLL)
//=============================================================================
#ifdef _USRDLL
#define AFXMANAGESTATE AfxGetStaticModuleState
#else
#define AFXMANAGESTATE AfxGetAppModuleState
#endif

//=============================================================================
BEGIN_MESSAGE_MAP(CXGroupBox, CStatic)
//=============================================================================
	//{{AFX_MSG_MAP(CXGroupBox)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//=============================================================================
CXGroupBox::CXGroupBox() :
//=============================================================================
	m_crText(GetSysColor(COLOR_BTNTEXT)),
	m_crBackground(GetSysColor(COLOR_BTNFACE)),
	m_crBorder(CLR_INVALID),
	m_nXMargin(DEFAULT_X_MARGIN),
	m_nYMargin(0),
	m_nYOffset(-1),
	m_hIcon(0),
	m_hGrayIcon(0),
	m_bGrayPaletteSet(FALSE),
	m_nIconSpacing(DEFAULT_ICON_SPACING),
	m_nIconSize(0),
	m_bDestroyIcon(FALSE),
	m_rect(CRect(0,0,0,0)),
	m_sizeText(CSize(0,0)),
	m_bThemed(FALSE),
	m_bEnableTheme(TRUE),
	m_bShowDisabledState(TRUE),
	m_eStyle(groupbox),
	m_eIconAlignment(left),
	m_eControlAlignment(left),
	m_eBorderStyle(etched)
{
	TRACE(_T("in CXGroupBox::CXGroupBox\n"));
}

//=============================================================================
CXGroupBox::~CXGroupBox()
//=============================================================================
{
	if (m_font.GetSafeHandle())
		m_font.DeleteObject();
	if (m_hIcon && m_bDestroyIcon)
		::DestroyIcon(m_hIcon);
	m_hIcon = 0;
	if (m_hGrayIcon)
		::DestroyIcon(m_hGrayIcon);
	m_hGrayIcon = 0;
}

//=============================================================================
void CXGroupBox::PreSubclassWindow() 
//=============================================================================
{
	TRACE(_T("in CXGroupBox::PreSubclassWindow\n"));

	// remove any border bits
	ModifyStyle(WS_BORDER|SS_SUNKEN, 0);
	ModifyStyleEx(WS_EX_CLIENTEDGE|WS_EX_STATICEDGE|WS_EX_DLGMODALFRAME|
					WS_EX_WINDOWEDGE, 0);

	// check theming
	m_bThemed = (g_xpStyle.IsThemeActive() && g_xpStyle.IsAppThemed()) ? 
					g_xpStyle.OpenThemeData(NULL, L"BUTTON") : NULL;
	if (m_bThemed)
	{
		TRACE(_T("themed\n"));
		m_crText = CLR_INVALID;
		HRESULT hr = g_xpStyle.GetThemeColor(BP_GROUPBOX, GBS_NORMAL, 
						TMT_TEXTCOLOR, &m_crText);
		if ((hr != S_OK) || (m_crText == CLR_INVALID))
		{
			TRACE(_T("WARNING - GetThemeColor failed\n"));
			m_crText = GetSysColor(COLOR_HIGHLIGHT);
		}
	}
	else
	{
		TRACE(_T("not themed\n"));
		m_crText = GetSysColor(COLOR_BTNTEXT);
	}

	// create font for this control
	CFont* pFont = GetSafeFont();
	ASSERT(pFont);

	if (pFont)
	{
		ASSERT(pFont->GetSafeHandle());

		// create the font for this control
		LOGFONT lf;
		pFont->GetLogFont(&lf);
		lf.lfCharSet = DEFAULT_CHARSET;
		if (m_font.GetSafeHandle())
			m_font.DeleteObject();
		VERIFY(m_font.CreateFontIndirect(&lf));
		//m_font.GetLogFont(&lf);
		//TRACE("font=%s\n", lf.lfFaceName);
	}

	CStatic::PreSubclassWindow();
}

//=============================================================================
void CXGroupBox::OnPaint() 
//=============================================================================
{
	static BOOL bInPaint = FALSE;

	if (bInPaint)
		return;
	bInPaint = TRUE;
	CPaintDC dc(this); // device context for painting
	CRect rect;
	GetWindowRect(&rect);
	int w = rect.Width();
	int h = rect.Height();
	rect.left = 0;
	rect.top = 0;
	rect.right = w;
	rect.bottom = h;
	DrawItem(&dc, rect);

	bInPaint = FALSE;

	// Do not call CStatic::OnPaint() for painting messages
}

//=============================================================================
void CXGroupBox::DrawItem(CDC *pDC, CRect& rect) 
//=============================================================================
{
	TRACE(_T("in CXGroupBox::DrawItem\n"));

	ASSERT(pDC);
	if (!pDC)
		return;

#if 0  // -----------------------------------------------------------
	// get control alignment (for icon and text)
	DWORD dwStyle = GetStyle() & 0xFF;
	switch (dwStyle)
	{
		default:
		case SS_LEFT:
			m_eControlAlignment = left;
			break;

		case SS_CENTER:
			m_eControlAlignment = center;
			break;

		case SS_RIGHT:
			m_eControlAlignment = right;
			break;
	}
#endif // -----------------------------------------------------------

	CRect rectItem(rect);
	CRect rectDraw(rectItem);
	TRACERECT(rectItem);
	CRect rectText(rectDraw);
	CRect rectIcon(rectDraw);
	CRect rectFrame(rectDraw);
	CRect rectHeader(rectDraw);

	if (m_sizeText.cx == 0)
	{
		// get string width
		CString strText = _T("");
		GetWindowText(strText);

		CFont *pOldFont = pDC->SelectObject(&m_font);
		if (!strText.IsEmpty())
			m_sizeText = pDC->GetTextExtent(strText);
		if (pOldFont)
			pDC->SelectObject(pOldFont);
	}

	m_nYOffset = __max(m_sizeText.cy, m_nIconSize) + 1;
	TRACE(_T("..... m_nYOffset=%d  m_sizeText.cy=%d  m_nIconSize=%d\n"), m_nYOffset, m_sizeText.cy, m_nIconSize);
	rectText.bottom = rectText.top + m_nYOffset;
	rectHeader.bottom = rectHeader.top + m_nYOffset;
	m_nYOffset = m_nYOffset / 2;
	rectFrame.top  += m_nYOffset;
	rectIcon.top = rectFrame.top - m_nIconSize/2;
	rectIcon.bottom = rectIcon.top + m_nIconSize;

	// set up for double buffering
	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rectDraw.Width(), rectDraw.Height());
	CBitmap *pOldBitmap = memDC.SelectObject(&bmp);

	// fill in entire window
	memDC.FillSolidRect(&rectItem, m_crBackground);

	if (m_eStyle == groupbox)
	{
		TRACE(_T("groupbox\n"));
		// get parent DC and copy current contents - this will emulate 
		// transparent blt'ing

		CRect rectWindow;
		GetWindowRect(&rectWindow);
		GetParent()->ScreenToClient(&rectWindow);
		rectWindow.top += m_nYOffset*2;

		CDC *pParentDC = GetParent()->GetDC();
		memDC.BitBlt(0, m_nYOffset*2, rectWindow.Width(), rectWindow.Height(),
			pParentDC, rectWindow.left, rectWindow.top, SRCCOPY);
		VERIFY(ReleaseDC(pParentDC));
	}

	// select NULL brush for border drawing
	HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(memDC.m_hDC, hBrush);

	if (m_eStyle == groupbox)
	{
		// erase the old border that exists in the parent DC
		EraseBorder(&memDC, m_rect, m_crBackground);
		m_rect = rectFrame;
	}

	DrawBorder(&memDC, rectFrame);

	if (hOldBrush)
		::SelectObject(memDC.m_hDC, hOldBrush);

	// erase caption area where icon and text will be displayed -
	// this removes frame from that area
	EraseHeaderArea(&memDC, rectHeader, m_crBackground);

	rectText.left = rectHeader.left;
	rectIcon.left = rectHeader.left;

	DrawHeader(&memDC, rectText, rectIcon);

	// end double buffering
	pDC->BitBlt(0, 0, rectDraw.Width(), rectDraw.Height(),
		&memDC, 0, 0, SRCCOPY);			

	// swap back the original bitmap
	if (pOldBitmap)
		memDC.SelectObject(pOldBitmap);
	if (bmp.GetSafeHandle())
		bmp.DeleteObject();

	memDC.DeleteDC();
}

//=============================================================================
void CXGroupBox::DrawHeader(CDC *pDC, CRect& rectText, CRect& rectIcon)
//=============================================================================
{
	if (m_eIconAlignment == right)
	{
		// icon is on right

		// there is no spacing if left-aligned 'header'
		if ((m_eStyle == groupbox) || (m_eControlAlignment != left))
			rectText.left += 3;
		rectIcon.left = DrawText(pDC, rectText);
		if (m_sizeText.cx != 0)
			rectIcon.left += m_nIconSpacing;
		DrawIcon(pDC, rectIcon);
	}
	else
	{
		// icon is on left

		// there is no spacing if left-aligned 'header'
		if ((m_eStyle == groupbox) || (m_eControlAlignment != left))
			rectIcon.left += 3;
		rectIcon.right = rectIcon.left + m_nIconSize;
		rectText.left = DrawIcon(pDC, rectIcon);
		if (m_hIcon)
			rectText.left += m_nIconSpacing;
		DrawText(pDC, rectText);
	}
}

//=============================================================================
// returns rect where text + icon will be drawn
void CXGroupBox::GetHeaderRect(CRect& rect)
//=============================================================================
{
	CRect rectItem(rect);

	rect.top    += m_nYMargin;
	rect.bottom += m_nYMargin;

	int nTotalWidth = m_sizeText.cx;
	nTotalWidth += m_nIconSize;				// this will be 0 if no icon
	if ((m_sizeText.cx != 0) && m_hIcon)
		nTotalWidth += m_nIconSpacing;

	if (m_eStyle == groupbox)
	{
		TRACE(_T("groupbox\n"));
		nTotalWidth += 6;			// 3 pixels before and after
		if (m_eControlAlignment == left)
		{
			rect.left += m_nXMargin;
		}
		else if (m_eControlAlignment == center)
		{
			rect.left = rect.left + (rectItem.Width() - nTotalWidth) / 2;
		}
		else	// right
		{
			rect.left = rectItem.right - nTotalWidth - m_nXMargin;
		}
		rect.right = rect.left + nTotalWidth;
	}
	else	// header style
	{
		TRACE(_T("header\n"));
		nTotalWidth += 3;			// 3 pixels after
		if (m_eControlAlignment == left)
		{
			
		}
		else if (m_eControlAlignment == center)
		{
			nTotalWidth += 3;			// 3 pixels before
			rect.left = rect.left + (rectItem.Width() - nTotalWidth) / 2;
		}
		else	// right
		{
			rect.left = rectItem.right - nTotalWidth;// - m_nXMargin;
		}
		rect.right = rect.left + nTotalWidth;
	}
}

//=============================================================================
void CXGroupBox::EraseHeaderArea(CDC *pDC, CRect& rect, COLORREF crBackground)
//=============================================================================
{
	GetHeaderRect(rect);
	pDC->FillSolidRect(&rect, crBackground);
}

//=============================================================================
void CXGroupBox::EraseBorder(CDC *pDC, CRect& rect, COLORREF crBackground)
//=============================================================================
{
	if (!rect.IsRectEmpty())
	{
		CRect r(rect);
		CPen pen(PS_SOLID, 5, crBackground);
		CPen *pOldPen = pDC->SelectObject(&pen);
		r.DeflateRect(2, 2);
		pDC->Rectangle(&r);
		if (pOldPen)
			pDC->SelectObject(pOldPen);
	}
}

//=============================================================================
void CXGroupBox::DrawEtchedRectangle(CDC *pDC, CRect& rect, COLORREF cr)
//=============================================================================
{
	TRACE(_T("in CXGroupBox::DrawEtchedRectangle\n"));
	COLORREF crDark = IsWindowEnabled() ? cr : GetSysColor(COLOR_BTNSHADOW);
	CPen penDark(PS_SOLID, 1, crDark);
	CPen *pOldPen = pDC->SelectObject(&penDark);
		
	if (m_eBorderStyle == etched)
	{
		// border is 2 pixels, so deflate bottom and right sides by 1
		CPen penLight(PS_SOLID, 2, GetSysColor(COLOR_BTNHIGHLIGHT));	// 2 pixels wide
		pDC->SelectObject(&penLight);
		rect.DeflateRect(0, 0, 1, 1);
		rect.OffsetRect(1, 1);
		pDC->Rectangle(&rect);			// white rectangle, 2 pixels wide
		rect.OffsetRect(-1, -1);
	}

	pDC->SelectObject(&penDark);
	pDC->Rectangle(&rect);			// dark rectangle - overlay white rectangle

	if (pOldPen)
		pDC->SelectObject(pOldPen);
}

//=============================================================================
void CXGroupBox::DrawEtchedLine(CDC *pDC, CRect& rect, COLORREF cr)
//=============================================================================
{
	COLORREF crDark = IsWindowEnabled() ? cr : GetSysColor(COLOR_BTNSHADOW);
	CPen penDark(PS_SOLID, 1, crDark);
	CPen *pOldPen = pDC->SelectObject(&penDark);

	pDC->MoveTo(rect.left, rect.top);
	pDC->LineTo(rect.right, rect.top);

	if (m_eBorderStyle == etched)
	{
		CPen penLight(PS_SOLID, 1, GetSysColor(COLOR_BTNHIGHLIGHT));
		pDC->SelectObject(&penLight);
		pDC->MoveTo(rect.left, rect.top+1);
		pDC->LineTo(rect.right, rect.top+1);
	}

	if (pOldPen)
		pDC->SelectObject(pOldPen);
}

//=============================================================================
void CXGroupBox::DrawBorder(CDC *pDC, CRect& rect)
//=============================================================================
{
	if (m_eStyle == groupbox)
	{
		TRACE(_T("DrawBorder: groupbox\n"));

		// user color overrides themes
		if (m_crBorder != CLR_INVALID)
		{
			DrawEtchedRectangle(pDC, rect, m_crBorder);
		}
		else if (g_xpStyle.m_hTheme && m_bEnableTheme)
		{
			TRACE(_T("drawing with theme\n"));
			g_xpStyle.DrawThemeBackground(pDC->m_hDC,
				BP_GROUPBOX, GBS_NORMAL, &rect, NULL);
		}
		else
		{
			// not themed and no user color
			DrawEtchedRectangle(pDC, rect, GetSysColor(COLOR_BTNSHADOW));
		}
	}
	else
	{
		// header style

		TRACE(_T("DrawBorder: header\n"));
		if ((g_xpStyle.m_hTheme && m_bEnableTheme) || 
			(m_crBorder != CLR_INVALID))
		{
			COLORREF crBorder = m_crBorder;
			if (crBorder == CLR_INVALID)
				crBorder = GetSysColor(COLOR_BTNSHADOW);
			DrawEtchedLine(pDC, rect, crBorder);
		}
		else
		{
			TRACE(_T("not themed ====================================================\n"));
			// not themed and no user color - draw etched line

			DrawEtchedLine(pDC, rect, GetSysColor(COLOR_BTNSHADOW));
		}
	}
}

//=============================================================================
int CXGroupBox::DrawIcon(CDC *pDC, CRect& rect)
//=============================================================================
{
	int rc = rect.left;		// if no icon just return start position

	if (m_hIcon)
	{
		HICON hIcon = m_hIcon;

		if (!IsWindowEnabled() && m_bShowDisabledState)
		{
			if (!m_hGrayIcon)
				m_hGrayIcon = CreateGrayscaleIcon(m_hIcon);
			hIcon = m_hGrayIcon;
		}

		VERIFY(::DrawIconEx(pDC->m_hDC, rect.left, rect.top, hIcon, 
			m_nIconSize, m_nIconSize, 0, 0, DI_IMAGE|DI_MASK));

		rc = rect.right;	// return new left drawing margin
	}

	return rc;
}

//=============================================================================
int CXGroupBox::DrawText(CDC *pDC, CRect& rect)
//=============================================================================
{
	int rc = rect.left;		// if no string just return start position

	CString strText = _T("");
	GetWindowText(strText);
	TRACE(_T("strText=<%s>\n"), strText);

	if (!strText.IsEmpty())
	{
		COLORREF crText = m_crText;
		if (!IsWindowEnabled() && m_bShowDisabledState)
			crText = GetSysColor(COLOR_GRAYTEXT);
		pDC->SetTextColor(crText);
		pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
		pDC->SetBkMode(OPAQUE);

		CFont *pOldFont = pDC->SelectObject(&m_font);

		// always left aligned within drawing rect
		UINT nFormat = DT_LEFT | DT_VCENTER | DT_SINGLELINE;

		CRect rectText(rect);
		// get size of text so we can update rect
		CSize size = pDC->GetTextExtent(strText);
		rectText.right = rectText.left + size.cx;

		pDC->DrawText(strText, rectText, nFormat);

		rc = rectText.right;

		if (pOldFont)
			pDC->SelectObject(pOldFont);
	}

	return rc;
}

//=============================================================================
BOOL CXGroupBox::OnEraseBkgnd(CDC* /*pDC*/) 
//=============================================================================
{
	return TRUE;	// we draw everything
	//return CStatic::OnEraseBkgnd(pDC);
}

//=============================================================================
CXGroupBox& CXGroupBox::SetControlStyle(CONTROL_STYLE eStyle, 
										BOOL bRedraw /*= TRUE*/) 
//=============================================================================
{
	m_eStyle = eStyle;

	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetIconAlignment(ALIGNMENT eAlign, 
										 BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	m_eIconAlignment = eAlign;

	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetAlignment(ALIGNMENT eAlign, 
									 BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	m_eControlAlignment = eAlign;

	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetDisabledStyle(BOOL bShowDisabledState, 
										 BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	m_bShowDisabledState = bShowDisabledState;

	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CFont * CXGroupBox::GetSafeFont()
//=============================================================================
{
	// get current font
	CFont *pFont = CWnd::GetFont();

	if (pFont == 0)
	{
		// try to get parent font
		CWnd *pParent = GetParent();
		if (pParent && IsWindow(pParent->m_hWnd))
			pFont = pParent->GetFont();

		if (pFont == 0)
		{
			// no font, so get a system font
			HFONT hFont = (HFONT)::GetStockObject(DEFAULT_GUI_FONT);
			if (hFont == 0)
				hFont = (HFONT)::GetStockObject(SYSTEM_FONT);
			if (hFont == 0)
				hFont = (HFONT)::GetStockObject(ANSI_VAR_FONT);
			if (hFont)
				pFont = CFont::FromHandle(hFont);
		}
	}

	return pFont;
}

//=============================================================================
int CXGroupBox::GetFontHeight(int nPointSize)
//=============================================================================
{
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	ASSERT(hdc);
	int cyPixelsPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	::DeleteDC(hdc);

	int nHeight = -MulDiv(nPointSize, cyPixelsPerInch, 72);

	return nHeight;
}

//=============================================================================
int CXGroupBox::GetFontPointSize(int nHeight)
//=============================================================================
{
	HDC hdc = ::CreateDC(_T("DISPLAY"), NULL, NULL, NULL);
	ASSERT(hdc);
	int cyPixelsPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
	::DeleteDC(hdc);

	int nPointSize = MulDiv(nHeight, 72, cyPixelsPerInch);
	if (nPointSize < 0)
		nPointSize = -nPointSize;

	return nPointSize;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetBold(BOOL bBold, 
						BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	m_font.GetLogFont(&lf);

	lf.lfWeight = bBold ? FW_BOLD : FW_NORMAL;

	m_sizeText.cx = 0;		// force string width to be recalculated

	return SetFont(&lf, bRedraw);
}

//=============================================================================
BOOL CXGroupBox::GetBold()
//=============================================================================
{
	BOOL rc = FALSE;

	CFont *pFont = GetFont(); 
	if (pFont)
	{
		LOGFONT lf; 
		pFont->GetLogFont(&lf);
		rc = lf.lfWeight == FW_BOLD;
	}
	return rc;
}

//=============================================================================
CString CXGroupBox::GetFontFaceName()
//=============================================================================
{
	CString strFaceName = _T("MS Sans Serif");

	CFont *pFont = GetFont(); 
	if (pFont)
	{
		LOGFONT lf; 
		pFont->GetLogFont(&lf);
		strFaceName = lf.lfFaceName;
	}

	return strFaceName;
}

//=============================================================================
int CXGroupBox::GetFontPointSize()
//=============================================================================
{
	int rc = 8;

	CFont *pFont = GetFont(); 
	if (pFont)
	{
		LOGFONT lf; 
		pFont->GetLogFont(&lf);
		rc = GetFontPointSize(lf.lfHeight);
	}
	return rc;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetFont(LPCTSTR lpszFaceName, 
						int nPointSize, 
						BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	// null face name is ok - we will use current font

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));

	if ((lpszFaceName == NULL) || (lpszFaceName[0] == _T('\0')))
		m_font.GetLogFont(&lf);
	else
		_tcsncpy(lf.lfFaceName, lpszFaceName, sizeof(lf.lfFaceName)/sizeof(TCHAR)-1);

	lf.lfHeight = GetFontHeight(nPointSize);

	return SetFont(&lf, bRedraw);
}

//=============================================================================
CXGroupBox& CXGroupBox::SetFont(CFont *pFont, 
						BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	ASSERT(pFont);

	if (!pFont)
		pFont = GetSafeFont();

	if (pFont)
	{
		LOGFONT lf;
		memset(&lf, 0, sizeof(lf));

		pFont->GetLogFont(&lf);

		SetFont(&lf, bRedraw);
	}

	return *this;
}

//=============================================================================
BOOL CXGroupBox::GetFont(LOGFONT *pLF)
//=============================================================================
{
	BOOL rc = FALSE;

	ASSERT(pLF);

	if (pLF)
	{
		memset(pLF, 0, sizeof(LOGFONT));
		CFont *pFont = GetFont(); 
		if (pFont)
		{
			pFont->GetLogFont(pLF);
			rc = TRUE;
		}
	}
	return rc;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetFont(LOGFONT *pLogFont, 
						BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	TRACE(_T("in CXGroupBox::SetFont =================\n"));
	ASSERT(pLogFont);

	if (pLogFont)
	{
		if (m_font.GetSafeHandle())
			m_font.DeleteObject();
		VERIFY(m_font.CreateFontIndirect(pLogFont));

		m_sizeText.cx = 0;		// force string width to be recalculated

		if (bRedraw)
			RedrawWindow();
	}

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetMargins(int nXMargin, int nYMargin, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{ 
	m_nXMargin = nXMargin; 
	m_nYMargin = nYMargin; 

	if (bRedraw) 
		RedrawWindow(); 

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetIcon(HICON hIcon, 
						UINT nIconSize /*= 16*/, 
						BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	TRACE(_T("in CXGroupBox::SetIcon 1\n"));

	m_nIconSize = nIconSize;

	if (m_hIcon && m_bDestroyIcon)
		::DestroyIcon(m_hIcon);

	if (m_hGrayIcon)
		::DestroyIcon(m_hGrayIcon);
	m_hGrayIcon = 0;

	m_hIcon = hIcon;

	if (m_hIcon == 0)
		m_nIconSize = 0;

	m_bDestroyIcon = FALSE;

	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetIcon(UINT nIconId, 
						UINT nIconSize /*= 16*/, 
						BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	TRACE(_T("in CXGroupBox::SetIcon 2\n"));
	AFX_MANAGE_STATE(AFXMANAGESTATE());

	m_nIconSize = nIconSize;

	if (m_hIcon && m_bDestroyIcon)
		::DestroyIcon(m_hIcon);

	if (m_hGrayIcon)
		::DestroyIcon(m_hGrayIcon);
	m_hGrayIcon = 0;

	if (nIconId)
	{
		m_hIcon = (HICON) ::LoadImage(AfxGetInstanceHandle(), 
									MAKEINTRESOURCE(nIconId),
									IMAGE_ICON, m_nIconSize, m_nIconSize, 0);
		ASSERT(m_hIcon);
	}
	else
	{
		m_hIcon = 0;
	}

	if (m_hIcon == 0)
		m_nIconSize = 0;

	m_bDestroyIcon = TRUE;

	if (bRedraw)
		RedrawWindow();

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetImageList(CImageList * pImageList,
									 int nIconNo, 
									 BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	m_nIconSize = 0;

	if (m_hIcon && m_bDestroyIcon)
		::DestroyIcon(m_hIcon);
	m_hIcon = 0;

	if (m_hGrayIcon)
		::DestroyIcon(m_hGrayIcon);
	m_hGrayIcon = 0;

	ASSERT(pImageList);
	ASSERT(pImageList->m_hImageList);

	if (pImageList && pImageList->m_hImageList)
	{
		ASSERT(nIconNo < pImageList->GetImageCount());

		if (nIconNo < pImageList->GetImageCount())
		{
			// get image size from CImageList
			int cx = 0;
			::ImageList_GetIconSize(pImageList->m_hImageList, &cx, &m_nIconSize);

			m_hIcon = (HICON) pImageList->ExtractIcon(nIconNo);
			ASSERT(m_hIcon);

			if (m_hIcon == 0)
				m_nIconSize = 0;

			m_bDestroyIcon = TRUE;

			if (bRedraw)
				RedrawWindow();
		}
	}

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetIconSpacing(int nIconSpacing, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{ 
	m_nIconSpacing = nIconSpacing; 

	if (bRedraw) 
		RedrawWindow(); 

	return *this; 
}

//=============================================================================
CXGroupBox& CXGroupBox::SetWindowText(LPCTSTR lpszText, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	AFX_MANAGE_STATE(AFXMANAGESTATE());

	CString strText = _T("");

	m_sizeText.cx = 0;		// force string width to be recalculated

	// lpszText can be pointer to normal text string,
	// or it can be resource id constructed with
	// MAKEINTRESOURCE()

	if (lpszText)
	{
		// is this a text string or an id?
		if (HIWORD(lpszText) == 0)
		{
			// id
			UINT nId = LOWORD((UINT)(UINT_PTR)lpszText);
			VERIFY(strText.LoadString(nId));
		}
		else
		{
			// string
			strText = lpszText;
		}
	}

	CStatic::SetWindowText(strText); 

	if (bRedraw) 
		RedrawWindow();

	return *this;
}

//=============================================================================
CXGroupBox& CXGroupBox::SetTextColor(COLORREF cr, BOOL bRedraw /*= TRUE*/) 
//=============================================================================
{ 
	m_crText = cr; 

	if (bRedraw) 
		RedrawWindow(); 

	return *this; 
}

//=============================================================================
CXGroupBox& CXGroupBox::SetBorderColor(COLORREF cr, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	m_crBorder = cr; 

	if (bRedraw) 
		RedrawWindow(); 

	return *this; 
}

//=============================================================================
CXGroupBox& CXGroupBox::SetBorderStyle(BORDER_STYLE eStyle, 
									   BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	TRACE(_T("in CXGroupBox::SetBorderStyle: %d\n"), eStyle);

	m_eBorderStyle = eStyle;

	if (bRedraw) 
		RedrawWindow(); 

	return *this; 
}

//=============================================================================
//
// EnableChildWindows()
//
// Purpose:     This function enables/disables all the controls that are
//              completely contained within a parent.
//
// Parameters:  hWnd          - HWND of parent control
//              bEnable       - TRUE = enable controls within parent
//              bEnableParent - TRUE = also enable/disable parent window
//
// Returns:     int     - number of controls enabled/disabled.  If zero is
//                        returned, it means that no controls lie within the
//                        rect of the parent.
//
int CXGroupBox::EnableChildWindows(HWND hWnd, BOOL bEnable, BOOL bEnableParent)
{
	int rc = 0;

	if (bEnableParent)
		::EnableWindow(hWnd, bEnable);

	RECT rectWindow;
	::GetWindowRect(hWnd, &rectWindow);

	// get first child control

	HWND hWndChild = 0;
	HWND hWndParent = ::GetParent(hWnd);
	if (IsWindow(hWndParent))
		hWndChild = ::GetWindow(hWndParent, GW_CHILD);

	while (hWndChild)
	{
		RECT rectChild;
		::GetWindowRect(hWndChild, &rectChild);

		// check if child rect is entirely contained within window
		if ((rectChild.left   >= rectWindow.left) &&
			(rectChild.right  <= rectWindow.right) &&
			(rectChild.top    >= rectWindow.top) &&
			(rectChild.bottom <= rectWindow.bottom))
		{
			//TRACE(_T("found child window 0x%X\n"), hWndChild);
			::EnableWindow(hWndChild, bEnable);
			rc++;
		}

		// get next child control
		hWndChild = ::GetWindow(hWndChild, GW_HWNDNEXT);
	}

	// if any controls were affected, invalidate the parent rect
	if (rc && IsWindow(hWndParent))
	{
		::InvalidateRect(hWndParent, NULL, FALSE);
	}

	return rc;
}

//=============================================================================
BOOL CXGroupBox::EnableWindow(BOOL bEnable /*= TRUE*/, 
							  BOOL bRecurseChildren /*= FALSE*/)
//=============================================================================
{
	BOOL rc = CStatic::EnableWindow(bEnable);

	if (bRecurseChildren)
		EnableChildWindows(m_hWnd, bEnable, FALSE);

	return rc;
}

//=============================================================================
CXGroupBox& CXGroupBox::EnableTheme(BOOL bEnable, BOOL bRedraw /*= TRUE*/)
//=============================================================================
{
	m_bEnableTheme = bEnable;

	if (m_bEnableTheme && m_bThemed)
	{
		m_crBorder = CLR_INVALID;
		m_crText = CLR_INVALID;

		// get theme color for text
		HRESULT hr = g_xpStyle.GetThemeColor(BP_GROUPBOX, GBS_NORMAL, 
						TMT_TEXTCOLOR, &m_crText);
		if ((hr != S_OK) || (m_crText == CLR_INVALID))
		{
			TRACE(_T("WARNING - GetThemeColor failed\n"));
			m_crText = GetSysColor(COLOR_HIGHLIGHT);
		}
	}

	if (bRedraw) 
		RedrawWindow(); 

	return *this; 
}

//=============================================================================
// Author:  Gladstone, with fixes by poxui
//          http://www.codeproject.com/KB/graphics/Create_GrayscaleIcon.aspx
//
HICON CXGroupBox::CreateGrayscaleIcon(HICON hIcon, COLORREF* pPalette)
//=============================================================================
{
	HICON hGrayIcon = NULL;

	ASSERT(hIcon);

	if (hIcon == NULL)
		return NULL;

	HDC hdc = ::GetDC(NULL);

	BITMAPINFO bmpInfo = { 0 };
	bmpInfo.bmiHeader.biSize  = sizeof(BITMAPINFOHEADER);

	ICONINFO icInfo = { 0 };
	if (::GetIconInfo(hIcon, &icInfo))
	{
		if (::GetDIBits(hdc, icInfo.hbmColor, 0, 0, NULL, &bmpInfo, 
				DIB_RGB_COLORS) != 0)
		{
			// ===== fix by poxui to adjust size of BITMAPINFO
			const int size = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 
				bmpInfo.bmiHeader.biClrUsed;
			BITMAPINFO* pBmpInfo = (BITMAPINFO*) new BYTE[size+100];
			memset(pBmpInfo, 0, size+100);
			pBmpInfo->bmiHeader = bmpInfo.bmiHeader;
			pBmpInfo->bmiHeader.biCompression = BI_RGB;
			
			LPDWORD lpBits = NULL;
			if (::GetDIBits(hdc, icInfo.hbmColor, 0, bmpInfo.bmiHeader.biHeight, 
					lpBits, pBmpInfo, DIB_RGB_COLORS) != 0)
			{
				// ===== fix by poxui to force 32bit loading =====
				bmpInfo.bmiHeader.biPlanes = 1;
				bmpInfo.bmiHeader.biBitCount = 32;
				bmpInfo.bmiHeader.biSizeImage = bmpInfo.bmiHeader.biWidth * 4 * 
					bmpInfo.bmiHeader.biHeight;
				bmpInfo.bmiHeader.biClrUsed = 0;
				bmpInfo.bmiHeader.biClrImportant = 0;
				bmpInfo.bmiHeader.biCompression = BI_RGB;

				SIZE sz;
				sz.cx = bmpInfo.bmiHeader.biWidth;
				sz.cy = bmpInfo.bmiHeader.biHeight;
				DWORD c1 = sz.cx * sz.cy;
	
				lpBits = (LPDWORD)::GlobalAlloc(GMEM_FIXED, c1 * 4);
	
				if (lpBits && ::GetDIBits(hdc, icInfo.hbmColor, 0, sz.cy, 
						lpBits, &bmpInfo, DIB_RGB_COLORS) != 0)
				{
					LPBYTE lpBitsPtr = (LPBYTE)lpBits;
					UINT off  = 0;
	
					for (UINT i = 0; i < c1; i++)
					{
						DWORD dwBits = lpBitsPtr[0] + lpBitsPtr[1] + lpBitsPtr[2];
						off = (UINT)(255 - (dwBits / 3));
	
						if (lpBitsPtr[3] != 0 || off != 255)
						{
							if (off == 0)
								off = 1;
	
							lpBits[i] = pPalette[off] | (lpBitsPtr[3] << 24);
						}
	
						lpBitsPtr += 4;
					}
	
					ICONINFO icGrayInfo = { 0 };
					icGrayInfo.hbmColor = ::CreateCompatibleBitmap(hdc, sz.cx, sz.cy);
	
					if (icGrayInfo.hbmColor != NULL)
					{
						::SetDIBits(hdc, icGrayInfo.hbmColor, 0, sz.cy, lpBits, 
							&bmpInfo, DIB_RGB_COLORS);
	
						icGrayInfo.hbmMask = icInfo.hbmMask;
						icGrayInfo.fIcon   = TRUE;
	
						hGrayIcon = ::CreateIconIndirect(&icGrayInfo);
	
						::DeleteObject(icGrayInfo.hbmColor);
					}
	
					::GlobalFree(lpBits);
					lpBits = NULL;
				}
			}
			delete [] (BYTE*) pBmpInfo;
		}

		::DeleteObject(icInfo.hbmColor);
		::DeleteObject(icInfo.hbmMask);
	}

	::ReleaseDC(NULL, hdc);

	return hGrayIcon;
}

//=============================================================================
HICON CXGroupBox::CreateGrayscaleIcon(HICON hIcon)
//=============================================================================
{
	HICON hGrayIcon = NULL;

	ASSERT(hIcon);

	if (hIcon)
	{
		if (!m_bGrayPaletteSet)
		{
			for (int i = 0; i < 256; i++)
			{
				m_defaultGrayPalette[i] = RGB(255-i, 255-i, 255-i);
			}

			m_bGrayPaletteSet = TRUE;
		}

		hGrayIcon = CreateGrayscaleIcon(hIcon, m_defaultGrayPalette);
	}

	return hGrayIcon;
}
