// Code located at http://www.codeproject.com/KB/miscctrl/XProgressWnd.aspx
// License located at http://www.codeproject.com/info/EULA.aspx
// Code change blocks marked with // start modified code: and // end modified code

// XProgressWnd.cpp : implementation file
//
///////////////////////////////////////////////////////////////////////////////
// ORIGINAL HEADER BY CHRIS MAUNDER
///////////////////////////////////////////////////////////////////////////////
// Written by Chris Maunder (chrismaunder@codeguru.com)
// Copyright 1998-2002
//
// CProgressWnd is a drop-in popup progress window for use in
// programs that a time consuming. Check out the header file
// or the accompanying HTML doc file for details.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is not sold for
// profit without the authors written consent, and providing that this
// notice and the authors name is included. If the source code in 
// this file is used in any commercial application then an email to
// the me would be nice.
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to your
// computer, causes your pet cat to fall ill, increases baldness or
// makes you car start emitting strange noises when you start it up.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 
//
// Updated May 18 1998 - added PeekAndPump function to allow modal operation,
//                       with optional "Cancel on ESC" (Michael <mbh-ep@post5.tele.dk>)
//         Nov 27 1998 - Removed modal stuff from PeekAndPump
//         Dec 18 1998 - added WS_EX_TOPMOST to the creation flag
//         Apr 14 2002 - Added SaveCurrentSettings to OnCancel (Geert Delmeiren)
//         Apr 22 2002 - Minor mods by Luke Gabello
///////////////////////////////////////////////////////////////////////////////
//
// XProgressWnd.cpp  Version 2.0
//
// Modified by:  Hans Dietrich
//               hdietrich@gmail.com
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XProgressWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#pragma warning(disable : 4996)	// disable bogus deprecation warning

#ifndef __noop
#if _MSC_VER < 1300
#define __noop ((void)0)
#endif
#endif

#undef TRACE
#define TRACE __noop

//=============================================================================
// if you want to see the TRACE output, uncomment this line:
//#include "XTrace.h"
//=============================================================================

//=============================================================================
// control IDs
//=============================================================================
#define IDC_CANCEL		10
#define IDC_PROGRESS	11
#define IDC_AVI			12

//=============================================================================
// registry/INI defines
//=============================================================================
#define PROGRESSWND_SECTION	_T("ProgressWnd")
#define PROGRESSWND_X		_T("X")
#define PROGRESSWND_Y		_T("Y")

//=============================================================================
static BOOL IsVistaOrLater()
//=============================================================================
{
	BOOL rc = FALSE;

	OSVERSIONINFO osvi = { 0 };
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	if (GetVersionEx(&osvi))
	{
		if (osvi.dwMajorVersion >= 6)
			rc = TRUE;
	}

	return rc;
}

//=============================================================================
BEGIN_MESSAGE_MAP(CXProgressWnd, CWnd)
//=============================================================================
	//{{AFX_MSG_MAP(CXProgressWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_CANCEL, OnCancel)
END_MESSAGE_MAP()

//=============================================================================
CXProgressWnd::CXProgressWnd()
//=============================================================================
{
	CommonConstruct();
}

//=============================================================================
CXProgressWnd::CXProgressWnd(CWnd* pParent, 
						   LPCTSTR lpszTitle, 
// start modified code: added to allow dialog with only a message
						   BOOL bEnableCancel /* = TRUE */,
						   BOOL bEnableProgressBar /* = TRUE */,
// end modified code
						   LPCTSTR lpszAviId /*= NULL*/, 
						   int nAviHeight /*= 50*/, 
						   BOOL bSmooth /* = FALSE */)
//=============================================================================
{
	CommonConstruct();
	m_nAviHeight = nAviHeight;
	m_pParent = pParent;
	m_strTitle = lpszTitle;
	m_bSmooth = bSmooth;
//	Create(pParent, lpszTitle, lpszAviId, bSmooth);
// start modified code: added to allow dialog with only a message
	Create(pParent, lpszTitle, bEnableCancel, bEnableProgressBar, lpszAviId, bSmooth);
// end modified code
}

//=============================================================================
void CXProgressWnd::CommonConstruct()
//=============================================================================
{
	m_wRenenableWnd       = NULL;
	m_pParent             = NULL;
	m_nNumTextLines       = 4;
	m_nPrevPos            = 0;
	m_nPrevPercent        = 0;
	m_nStep               = 1;
	m_nMinValue           = 0;
	m_nMaxValue           = 100;
	m_nAviHeight          = 50;
	m_strTitle            = _T("Progress");
	m_strCancelLabel      = _T(" Cancel ");
	m_strTimeLeftLabel    = _T("Estimated time left:");
	m_strHour             = _T("hour");
	m_strMin              = _T("min");
	m_strSec              = _T("sec");
	m_bCancelled          = FALSE;
	m_bModal              = FALSE;
	m_bFirstTime          = TRUE;
	m_bTimeLeft           = FALSE;
	m_bSmooth             = FALSE;
	m_bPersistentPosition = FALSE;   // saves and restores position automatically
	m_TextRect.SetRectEmpty();
	m_TimeLeftRect.SetRectEmpty();
}

//=============================================================================
CXProgressWnd::~CXProgressWnd()
//=============================================================================
{
	if (m_font.GetSafeHandle())
		m_font.DeleteObject();

	DestroyWindow();
}

//=============================================================================
BOOL CXProgressWnd::Create(CWnd *pParent, 
						  LPCTSTR lpszTitle, 
// start modified code: added to allow dialog with only a message
						  BOOL bEnableCancel /* = TRUE */,
						  BOOL bEnableProgressBar /* = TRUE */,
// end modified code
						  LPCTSTR lpszAviId /*= NULL*/,
						  BOOL bSmooth /* = FALSE */)
//=============================================================================
{
	BOOL bSuccess = FALSE;

	m_pParent = pParent;
	ASSERT(m_pParent);
	if (!m_pParent)
		return FALSE;

	m_strTitle = lpszTitle;
	m_nPercentComplete = 0;

	struct OLD_NONCLIENTMETRICS
	{
		UINT    cbSize;
		int     iBorderWidth;
		int     iScrollWidth;
		int     iScrollHeight;
		int     iCaptionWidth;
		int     iCaptionHeight;
		LOGFONT lfCaptionFont;
		int     iSmCaptionWidth;
		int     iSmCaptionHeight;
		LOGFONT lfSmCaptionFont;
		int     iMenuWidth;
		int     iMenuHeight;
		LOGFONT lfMenuFont;
		LOGFONT lfStatusFont;
		LOGFONT lfMessageFont;
	};

	// Get the system window message font for use in 
	// the cancel button and messages

	// the size of the NONCLIENTMETRICS struct increased 
	// in Vista by an int (iPaddedBorderWidth)
	const UINT cbProperSize = IsVistaOrLater() ? 
		sizeof(OLD_NONCLIENTMETRICS)+sizeof(int) :
		sizeof(OLD_NONCLIENTMETRICS);

	NONCLIENTMETRICS ncm;
	ncm.cbSize = cbProperSize;

	VERIFY(::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, cbProperSize, &ncm, 0));
    VERIFY(m_font.CreateFontIndirect(&(ncm.lfMessageFont)));

	// register window class
	CString csClassName = AfxRegisterWndClass(CS_OWNDC|CS_HREDRAW|CS_VREDRAW,
											  ::LoadCursor(NULL, IDC_APPSTARTING),
											  CBrush(::GetSysColor(COLOR_BTNFACE)));

	// create popup window
	bSuccess = CreateEx(WS_EX_DLGMODALFRAME/*|WS_EX_TOPMOST*/,	// Extended style
						csClassName,						// Classname
						m_strTitle,							// Title
						WS_POPUP|WS_BORDER|WS_CAPTION|WS_SYSMENU,		// style
						0,0,								// position - updated soon.
						390,130,							// Size - updated soon
						m_pParent->GetSafeHwnd(),			// handle to parent
						0,									// No menu
						NULL);	
	if (!bSuccess) 
		return FALSE;

	SetWindowPos(&wndTop, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);

	// now create the controls
	CRect TempRect(0,0,10,10);

// start modified code: added to allow dialog with only a message
	m_bEnableCancel = bEnableCancel;
	m_bEnableProgressBar = bEnableProgressBar;

	if (bEnableProgressBar)
	{
// end modified code
		DWORD dwProgressStyle = WS_CHILD|WS_VISIBLE;
#ifdef PBS_SMOOTH	
		if (bSmooth)
		   dwProgressStyle |= PBS_SMOOTH;
#endif
		bSuccess = m_wndProgress.Create(dwProgressStyle, TempRect, this, IDC_PROGRESS);
		if (!bSuccess) 
			return FALSE;
// start modified code: added to allow dialog with only a message
	}
// end modified code

	if (lpszAviId)
	{
		bSuccess = m_avi.Create(WS_CHILD|ACS_CENTER|ACS_TRANSPARENT|ACS_AUTOPLAY|WS_TABSTOP,
								TempRect, 
								this, 
								IDC_AVI);

		if (!bSuccess)
			return FALSE;

		UINT nID = (UINT) lpszAviId;

		if (HIWORD(nID))
			VERIFY(m_avi.Open(lpszAviId));		// open file
		else
			VERIFY(m_avi.Open(nID));			// open resource id
	}

// start modified code: added to allow dialog with only a message
	if (bEnableCancel)
	{
// end modified code
		bSuccess = m_CancelButton.Create(m_strCancelLabel, 
									 WS_CHILD|WS_VISIBLE|WS_TABSTOP| BS_PUSHBUTTON, 
									 TempRect, this, IDC_CANCEL);
		if (!bSuccess)
			return FALSE;

		m_CancelButton.SetFont(&m_font, TRUE);
// start modified code: added to allow dialog with only a message
	}
// end modified code

	// resize the whole thing according to the number of text lines, desired window
	// width and current font.
	SetWindowSize(m_nNumTextLines, 390);

	// center and show window
	if (m_bPersistentPosition)
		GetPreviousSettings();
	else
		CenterWindow();

	Show();

	return TRUE;
}

//=============================================================================
BOOL CXProgressWnd::GoModal(CWnd *pParent,
						   LPCTSTR lpszTitle /*=_T("Progress")"*/, 
// start modified code: added to allow dialog with only a message
						   BOOL bEnableCancel /* = TRUE */,
						   BOOL bEnableProgressBar /* = TRUE */,
// end modified code
						   LPCTSTR lpszAviId /*= NULL*/, 
						   BOOL bSmooth /*=FALSE*/)
//=============================================================================
{
	ASSERT(pParent);
	if (!pParent)
		return FALSE;

	m_pParent = pParent;

	if (!IsWindow(m_hWnd))
	{
//		if (!Create(m_pParent, lpszTitle, lpszAviId, bSmooth))
// start modified code: added to allow dialog with only a message
		if (!Create(m_pParent, lpszTitle, bEnableCancel, bEnableProgressBar, lpszAviId, bSmooth))
// end modified code
			return FALSE;
	}

	// Walk up the window chain to find the main parent wnd and disable it. 
	CWnd *pWnd = m_pParent;
	CWnd *prev = pWnd;
	for (;;) 
	{
		// if we have no parent (ie. the main window)
		// or if our parent is disabled, 
		// then this is the window that we will want to remember for reenabling
		if (!pWnd || !IsWindow(pWnd->m_hWnd) || !pWnd->IsWindowEnabled()) 
		{
			m_wRenenableWnd = prev;
			m_wRenenableWnd->EnableWindow(FALSE);
			break;
		}
		prev = pWnd;
		pWnd = pWnd->GetParent();
	}

	// Re-enable this window
	EnableWindow(TRUE);

	m_bModal = TRUE;

	return TRUE;
}
	
//=============================================================================
CXProgressWnd& CXProgressWnd::SetWindowSize(int nNumTextLines, int nWindowWidth /*=390*/)
//=============================================================================
{
	int nWidth = nWindowWidth;
	int nMargin = 10;

	CSize EdgeSize(::GetSystemMetrics(SM_CXEDGE), ::GetSystemMetrics(SM_CYEDGE));
	CSize CancelSize;

	CRect CancelRect, ProgressRect, AviRect;

 	AviRect.SetRectEmpty();
	if (IsWindow(m_avi.m_hWnd))
	{
		// we'll adjust the width later
		AviRect.SetRect(nMargin, nMargin, 
						nMargin + m_nAviHeight, nMargin + m_nAviHeight);
	}

	// set up a default size for the text area in case things go wrong
	m_TextRect.SetRect(nMargin, AviRect.bottom+nMargin, 
					   nWindowWidth-2*nMargin, AviRect.bottom+100+2*nMargin);

	m_TimeLeftRect.SetRectEmpty();

	// get DrawText to tell us how tall the text area will be (while we're at
	// it, we'll see how big the word "Cancel" is)
	CDC* pDC = GetDC();
	if (pDC)
	{
		CFont *pOldFont = pDC->SelectObject(&m_font);
		CString str = _T("M");
		for (int i = 0; i < nNumTextLines-1; i++)
			str += _T("\nM");
		pDC->DrawText(str, m_TextRect, DT_CALCRECT|DT_NOCLIP|DT_NOPREFIX);
		if (m_bTimeLeft)
			pDC->DrawText(_T("M"), 1, m_TimeLeftRect, 
					DT_CALCRECT|DT_NOCLIP|DT_NOPREFIX);
		nWidth = max(nWindowWidth, m_TextRect.Width()+2*nMargin);
		m_TextRect.right = nWidth - nMargin;
		CancelSize = pDC->GetTextExtent(m_strCancelLabel + _T("  "));
		CancelSize += CSize(EdgeSize.cx*11, EdgeSize.cy*5);
		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
	}

	AviRect.right = m_TextRect.right;

	// Work out how big (and where) the progress control should be
	ProgressRect.SetRect(m_TextRect.left,  m_TextRect.bottom + nMargin, 
						 m_TextRect.right, m_TextRect.bottom + nMargin + CancelSize.cy);

	if (m_bTimeLeft)
	{
		// Work out how big (and where) the time left text should be
		int h = m_TimeLeftRect.Height();
		m_TimeLeftRect.SetRect(m_TextRect.left,  ProgressRect.bottom + nMargin, 
							   m_TextRect.right, ProgressRect.bottom + nMargin + h);
	}
	else
	{
		m_TimeLeftRect = ProgressRect;
	}

	// work out how big (and where) the cancel button should be
	CancelRect.SetRect(ProgressRect.right - CancelSize.cx, m_TimeLeftRect.bottom + nMargin, 
					   ProgressRect.right, m_TimeLeftRect.bottom + nMargin + CancelSize.cy);

	// resize the main window to fit the controls
	CSize ClientSize(nWidth, nMargin + CancelRect.bottom);

	CRect WndRect, ClientRect;
	GetWindowRect(WndRect); 
	GetClientRect(ClientRect);
	WndRect.right  = WndRect.left + WndRect.Width()  - ClientRect.Width()  + ClientSize.cx;
	WndRect.bottom = WndRect.top  + WndRect.Height() - ClientRect.Height() + ClientSize.cy;
	MoveWindow(WndRect);

	// now reposition the controls...
	if (IsWindow(m_avi.m_hWnd))
		m_avi.MoveWindow(AviRect);
// start modified code: added to allow dialog with only a message
	if (m_bEnableProgressBar)
// end modified code
		m_wndProgress.MoveWindow(ProgressRect);
// start modified code: added to allow dialog with only a message
	if (m_bEnableCancel)
// end modified code
		m_CancelButton.MoveWindow(CancelRect);
	return *this;
}

//=============================================================================
CXProgressWnd& CXProgressWnd::Clear() 
//=============================================================================
{ 
	if (IsWindow(m_hWnd))
	{
		SetText(_T(""));
		SetPos(0);
		m_bCancelled = FALSE; 
		m_nPrevPos = 0;
		UpdateWindow();
	}

	return *this;
}

//=============================================================================
CXProgressWnd& CXProgressWnd::Hide()  
//=============================================================================
{ 
	if (IsWindow(m_hWnd)) 
	{
		if (IsWindowVisible())
		{
			ShowWindow(SW_HIDE);
			ModifyStyle(WS_VISIBLE, 0);
		}
	}

	return *this;
}

//=============================================================================
CXProgressWnd& CXProgressWnd::Show()  
//=============================================================================
{ 
	if (IsWindow(m_hWnd))
	{
		CDC *pDC = GetDC();
		ModifyStyle(0, WS_VISIBLE);
		ShowWindow(SW_SHOW);
		UpdateText(pDC, m_strText, m_TextRect);
		if (IsWindow(m_avi.m_hWnd))
			m_avi.ShowWindow(SW_SHOW);
// start modified code: added to allow dialog with only a message
		if (m_bEnableCancel)
// end modified code
			m_CancelButton.ShowWindow(SW_SHOW);
		if (m_bFirstTime)
		{
			RedrawWindow(NULL, NULL, RDW_ERASE|RDW_FRAME|RDW_INVALIDATE|RDW_UPDATENOW);
			m_dwStartProgressTicks = GetTickCount();
			m_bFirstTime = FALSE;
		}
		else if (m_bTimeLeft)
		{
			UpdateTimeLeft(pDC);
		}
		ReleaseDC(pDC);
	}
	return *this;
}

//=============================================================================
void CXProgressWnd::UpdateText(CDC *pDC, LPCTSTR lpszText, CRect& TextRect)
//=============================================================================
{
	if (!IsWindow(m_hWnd))
		return;

	if (TextRect.IsRectEmpty())
		return;

	ASSERT(lpszText);
	if (!lpszText)
		return;

	BOOL bReleaseDC = FALSE;
	if (!pDC)
	{
		pDC = GetDC();
		bReleaseDC = TRUE;
	}

	// double buffer the drawing
	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, TextRect.Width(), TextRect.Height());
	CBitmap *pOldBitmap = dcMem.SelectObject(&bmp);

	dcMem.SetBkColor(GetSysColor(COLOR_BTNFACE));
	dcMem.SetBkMode(OPAQUE);
	CRect rect(0, 0, TextRect.Width(), TextRect.Height());
	dcMem.FillSolidRect(&rect, GetSysColor(COLOR_BTNFACE));
	CFont *pOldFont = dcMem.SelectObject(&m_font);
	dcMem.DrawText(lpszText, -1, &rect, DT_LEFT);

	// end double buffering
	pDC->BitBlt(TextRect.left, TextRect.top, 
		TextRect.Width(), TextRect.Height(), &dcMem, 
		0, 0, SRCCOPY);
	dcMem.SelectObject(pOldFont);
	dcMem.SelectObject(pOldBitmap);
	bmp.DeleteObject();
	dcMem.DeleteDC();
	if (bReleaseDC)
		ReleaseDC(pDC);
}

//=============================================================================
void CXProgressWnd::UpdateTimeLeft(CDC *pDC)
//=============================================================================
{
// start modified code: added to allow dialog with only a message
	if (!m_bEnableProgressBar)
		return;
// end modified code

	if (!IsWindow(m_wndProgress.m_hWnd))
		return;

	if (m_TimeLeftRect.IsRectEmpty())
		return;

	// calculate time left
	int nLower, nUpper;
	m_wndProgress.GetRange(nLower, nUpper);
	double dRange = (double)nUpper - (double)nLower;
	double dPos = (double) abs(m_wndProgress.GetPos());
	DWORD dwNowTicks = GetTickCount();
	double dElapsedTime = (double) (dwNowTicks - m_dwStartProgressTicks);
	double dTotalSeconds = (dElapsedTime * dRange) / dPos;
	DWORD dwSecondsLeft = (DWORD) (((dTotalSeconds-dElapsedTime)/1000.) + 0.5);

	DWORD dwHour = 0;
	DWORD dwMin = 0;
	DWORD dwSec = 0;
	if ((dwSecondsLeft == 0) && (m_nPercentComplete < 100))
		dwSecondsLeft = 1;
	if (dwSecondsLeft > 3600)
		dwHour = dwSecondsLeft / 3600;
	if ((dwSecondsLeft - (dwHour * 3600)) > 60)
		dwMin = (dwSecondsLeft - (dwHour * 3600)) / 60;
	dwSec = dwSecondsLeft - (dwHour * 3600) - (dwMin * 60);

	CString s = _T("");
	if (dwHour)
		s.Format(_T("%s  %d %s %d %s"), 
			m_strTimeLeftLabel, dwHour, m_strHour, dwMin, m_strMin);
	else if (dwMin)
		s.Format(_T("%s  %d %s %d %s"), 
			m_strTimeLeftLabel, dwMin, m_strMin, dwSec, m_strSec);
	else
		s.Format(_T("%s  %d %s"), m_strTimeLeftLabel, dwSec, m_strSec);

	UpdateText(pDC, s, m_TimeLeftRect);
}

//=============================================================================
CXProgressWnd& CXProgressWnd::SetRange(int nLower, int nUpper, int nStep /* = 1 */)	
//=============================================================================
{
//	if (IsWindow(m_hWnd))
// start modified code: added to allow dialog with only a message
	if (m_bEnableProgressBar && IsWindow(m_hWnd))
// end modified code
	{
		// To take advantage of the Extended Range Values we use the PBM_SETRANGE32
		// message intead of calling CProgressCtrl::SetRange directly. If this is
		// being compiled under something less than VC 5.0, the necessary defines
		// may not be available.
#ifdef PBM_SETRANGE32
		ASSERT(-0x7FFFFFFF <= nLower && nLower <= 0x7FFFFFFF);
		ASSERT(-0x7FFFFFFF <= nUpper && nUpper <= 0x7FFFFFFF);
		m_wndProgress.SendMessage(PBM_SETRANGE32, (WPARAM) nLower, (LPARAM) nUpper);
#else
		ASSERT(0 <= nLower && nLower <= 65535);
		ASSERT(0 <= nUpper && nUpper <= 65535);
		m_wndProgress.SetRange(nLower, nUpper);
#endif

		m_nMaxValue = nUpper;
		m_nMinValue = nLower;
		m_nStep	 = nStep;

		m_wndProgress.SetStep(nStep);
	}
	return *this;
}

//=============================================================================
int CXProgressWnd::OffsetPos(int nPos)
//=============================================================================
{ 
	if (!IsWindow(m_hWnd)) 
		return m_nPrevPos;

	return SetPos(m_nPrevPos + nPos);  
}

//=============================================================================
int CXProgressWnd::StepIt()
//=============================================================================
{
	if (!IsWindow(m_hWnd)) 
		return m_nPrevPos;

	return SetPos(m_nPrevPos + m_nStep); 
}

//=============================================================================
int CXProgressWnd::SetStep(int nStep)
//=============================================================================
{
// start modified code: added to allow dialog with only a message
	if (!m_bEnableProgressBar) return -1;
// end modified code

	int nOldStep = m_nStep;
	m_nStep = nStep;
	if (!IsWindow(m_hWnd)) 
		return nOldStep;

	return m_wndProgress.SetStep(nStep);
}

//=============================================================================
int CXProgressWnd::SetPos(int nPos)					
//=============================================================================
{
#ifdef PBM_SETRANGE32
	ASSERT(-0x7FFFFFFF <= nPos && nPos <= 0x7FFFFFFF);
#else
	ASSERT(0 <= nPos && nPos <= 65535);
#endif

//	if (!IsWindow(m_hWnd))
// start modified code: added to allow dialog with only a message
	if (!m_bEnableProgressBar || !IsWindow(m_hWnd)) 
// end modified code
		return m_nPrevPos;

	Show();

	m_nPrevPos = nPos;

	if (m_nMaxValue > m_nMinValue)
		m_nPercentComplete = (int) (((nPos - m_nMinValue)*100.0)/(m_nMaxValue - m_nMinValue) + 0.5);
	else
		m_nPercentComplete = 0;

	if (m_nPercentComplete != m_nPrevPercent) 
	{
		m_nPrevPercent = m_nPercentComplete;
		CString strTitle;
		strTitle.Format(_T("%s [%d%%]"), m_strTitle, m_nPercentComplete);
		SetWindowText(strTitle);
	}
	return m_wndProgress.SetPos(nPos);		
}

//=============================================================================
CXProgressWnd& CXProgressWnd::SetText(LPCTSTR fmt, ...)
//=============================================================================
{
	ASSERT(fmt);
	if (fmt)
	{
		if (IsWindow(m_hWnd)) 
		{
			va_list args;
			TCHAR buffer[4096];

			va_start(args, fmt);
			_vsntprintf(buffer, sizeof(buffer)/sizeof(TCHAR)-1, fmt, args);
			va_end(args);
			buffer[sizeof(buffer)/sizeof(TCHAR)-1] = 0;
			m_strText = buffer;
		}
	}
	return *this;
}

//=============================================================================
BOOL CXProgressWnd::OnEraseBkgnd(CDC* pDC) 
//=============================================================================
{
	CRect rect;
	pDC->GetClipBox(&rect);	 // Erase the area needed
	pDC->FillSolidRect(&rect, GetSysColor(COLOR_BTNFACE));
	return TRUE;
}

//=============================================================================
void CXProgressWnd::OnPaint() 
//=============================================================================
{
	CPaintDC dc(this);
	
	UpdateText(&dc, m_strText, m_TextRect);

	if (m_bTimeLeft)
		UpdateTimeLeft(&dc);

	// Do not call CWnd::OnPaint() for painting messages
}

//=============================================================================
void CXProgressWnd::OnCancel() 
//=============================================================================
{
	if (m_bModal)
		PostMessage(WM_CLOSE);
	else
		Close();
}

//=============================================================================
void CXProgressWnd::OnClose() 
//=============================================================================
{
	Close();
}

//=============================================================================
void CXProgressWnd::Close() 
//=============================================================================
{
	if (m_bPersistentPosition)
		SaveCurrentSettings();

	m_bCancelled = TRUE;
	Hide();

	if (m_pParent && IsWindow(m_pParent->m_hWnd))
		m_pParent->SetForegroundWindow();
}

//=============================================================================
BOOL CXProgressWnd::DestroyWindow() 
//=============================================================================
{
	if (m_bPersistentPosition)
		SaveCurrentSettings();

	if (IsWindow(m_avi.m_hWnd))
	{
		m_avi.Stop();
		m_avi.Close();
	}

	if (m_bModal)
	{
		m_bModal = FALSE;

		if (m_wRenenableWnd && IsWindow(m_wRenenableWnd->m_hWnd))
			m_wRenenableWnd->EnableWindow(TRUE);
	}
	
	return CWnd::DestroyWindow();
}

//=============================================================================
// Message pumping function that can either be used to pump messages during
// long operations. This version will only pass messages to this window (and
// all child windows). (Thanks to Michael <mbh-ep@post5.tele.dk> for this)
CXProgressWnd& CXProgressWnd::PeekAndPump(BOOL bCancelOnESCkey /*= TRUE*/)
//=============================================================================
{
	if (IsWindow(m_hWnd) && m_bModal && (::GetFocus() != m_hWnd))
		SetFocus();

	MSG msg;
	while (!m_bCancelled && ::PeekMessage(&msg, NULL,0,0,PM_NOREMOVE)) 
	{
		if (bCancelOnESCkey && (msg.message == WM_CHAR) && (msg.wParam == VK_ESCAPE))
			OnCancel();

		// cancel button disabled is modal, so we fake it
		if (m_bModal && (msg.message == WM_LBUTTONUP))
		{
//			if (IsWindow(m_CancelButton.m_hWnd))
// start modified code: added to allow dialog with only a message
			if (m_bEnableCancel && IsWindow(m_CancelButton.m_hWnd))
// end modified code
			{
				CRect rect;
				m_CancelButton.GetWindowRect(rect);
				if (rect.PtInRect(msg.pt))
					OnCancel();
			}
		}
  
#if 0  // -----------------------------------------------------------
		if (!AfxGetApp()->PumpMessage()) 
		{
			::PostQuitMessage(0);
			break;
		}
#endif // -----------------------------------------------------------

		if ( ::GetMessage(&msg, NULL, 0, 0))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			::PostQuitMessage(0);
			break;
		}
	}
	return *this;
}

//=============================================================================
// restores the previous window size from the registry
void CXProgressWnd::GetPreviousSettings()
//=============================================================================
{
	if (!IsWindow(m_hWnd))
		return;

	int x = AfxGetApp()->GetProfileInt(PROGRESSWND_SECTION, PROGRESSWND_X, -1);
	int y = AfxGetApp()->GetProfileInt(PROGRESSWND_SECTION, PROGRESSWND_Y, -1);

	if (x >= 0 && x < GetSystemMetrics(SM_CXSCREEN) &&
		y >= 0 && y < GetSystemMetrics(SM_CYSCREEN))
	{
		SetWindowPos(NULL, x, y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	}
	else
	{
		CenterWindow();
	}
}

//=============================================================================
// saves the current window position in registry
void CXProgressWnd::SaveCurrentSettings()
//=============================================================================
{   
	if (!IsWindow(m_hWnd))
		return;

	CRect rect;
	GetWindowRect(rect);

	AfxGetApp()->WriteProfileInt(PROGRESSWND_SECTION, PROGRESSWND_X, rect.left);
	AfxGetApp()->WriteProfileInt(PROGRESSWND_SECTION, PROGRESSWND_Y, rect.top);
}
