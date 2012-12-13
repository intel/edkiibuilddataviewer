// Code located at http://www.codeproject.com/KB/miscctrl/XProgressWnd.aspx
// License located at http://www.codeproject.com/info/EULA.aspx
// Code change blocks marked with // start modified code: and // end modified code

// XProgressWnd.h : header file
//
///////////////////////////////////////////////////////////////////////////////
// ORIGINAL HEADER BY CHRIS MAUNDER
///////////////////////////////////////////////////////////////////////////////
// Written by Chris Maunder (chrismaunder@codeguru.com)
// Copyright 1998.
//
// CProgressWnd is a drop-in popup progress window for use in
// programs that a time consuming. Check out the accompanying HTML 
// doc file for details.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is not sold for
// profit without the authors written consent, and providing that this
// notice and the authors name is included. If the source code in 
// this file is used in any commercial application then an email to 
// me would be nice.
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
///////////////////////////////////////////////////////////////////////////////
//
// XProgressWnd.h  Version 2.0
//
// Modified by:  Hans Dietrich
//               hdietrich@gmail.com
//
///////////////////////////////////////////////////////////////////////////////

#ifndef XPROGRESSWND_H
#define XPROGRESSWND_H

#ifndef __AFXCMN_H__
#error AFXCMN.H MUST BE INCLUDED PRIOR TO THIS FILE
#endif

//=============================================================================
class CXProgressWnd : public CWnd
//=============================================================================
{
// Construction/Destruction
public:
	CXProgressWnd();
	CXProgressWnd(CWnd *pParent, LPCTSTR lpszTitle, 
// start modified code: added to allow dialog with only a message
		BOOL bEnableCancel = TRUE, BOOL bEnableProgressBar = TRUE,
// end modified code
		LPCTSTR lpszAviId = NULL, int nAviHeight = 50, 
		BOOL bSmooth = FALSE);
	virtual ~CXProgressWnd();

	BOOL Create(CWnd *pParent, LPCTSTR lpszTitle, 
// start modified code: added to allow dialog with only a message
		BOOL bEnableCancel = TRUE, BOOL bEnableProgressBar = TRUE,
// end modified code
		LPCTSTR lpszAviId = NULL, 
		BOOL bSmooth = FALSE);
	BOOL GoModal(CWnd *pParent, LPCTSTR lpszTitle =_T("Progress"), 
// start modified code: added to allow dialog with only a message
		BOOL bEnableCancel = TRUE, BOOL bEnableProgressBar = TRUE,
// end modified code
		LPCTSTR lpszAviId = NULL,  
		BOOL bSmooth = FALSE);

protected:
	void CommonConstruct();

// Attributes
public:
	BOOL			Cancelled() { return m_bCancelled; }
	CXProgressWnd&	EnablePersist(BOOL bFlag) 
					{ m_bPersistentPosition = bFlag; return *this; }
	CXProgressWnd&	EnableTimeLeft(BOOL bFlag) 
					{ 
						m_bTimeLeft = bFlag;
						SetWindowSize(m_nNumTextLines, 390);
						return *this; 
					}
	CXProgressWnd&	SetRange(int nLower, int nUpper, int nStep = 1);
	CXProgressWnd&	SetText(LPCTSTR fmt, ...);
	CXProgressWnd&	SetTimeLeftLabels(LPCTSTR lpszLabel, 
									  LPCTSTR lpszHour,
									  LPCTSTR lpszMin,
									  LPCTSTR lpszSec)
					{
						m_strTimeLeftLabel = lpszLabel;
						m_strHour = lpszHour;
						m_strMin  = lpszMin;
						m_strSec  = lpszSec;
						return *this;
					}
	CXProgressWnd&	SetWindowSize(int nNumTextLines, int nWindowWidth = 390);

// Operations
public:
	CXProgressWnd&	Clear();
	CXProgressWnd&	Hide();
	int				OffsetPos(int nPos);
	CXProgressWnd&	PeekAndPump(BOOL bCancelOnESCkey = TRUE);
	int				SetPos(int nPos);
	int				SetStep(int nStep);
	CXProgressWnd&	Show();
	int				StepIt();
    
// Implementation
protected:
	void Close();
	void GetPreviousSettings();
	void SaveCurrentSettings();
	void UpdateText(CDC *pDC, LPCTSTR lpszText, CRect& TextRect);
	void UpdateTimeLeft(CDC *pDC);

protected:
	CWnd *			m_pParent;
	CStatic			m_TimeLeft;
	CProgressCtrl	m_wndProgress;
	CButton			m_CancelButton;
	CWnd *			m_wRenenableWnd;
	CAnimateCtrl	m_avi;
	int				m_nAviHeight;
	CRect			m_TextRect;
	CRect			m_TimeLeftRect;

// start modified code: added to allow dialog with only a message
	BOOL			m_bEnableCancel;
	BOOL			m_bEnableProgressBar;
// end modified code
	BOOL			m_bCancelled;
	BOOL			m_bModal;
	BOOL			m_bPersistentPosition;
	BOOL			m_bTimeLeft;
	BOOL			m_bFirstTime;
	BOOL			m_bSmooth;
	int				m_nPrevPos, m_nPrevPercent;
	int				m_nStep;
	int				m_nMaxValue, m_nMinValue;
	int				m_nNumTextLines;
	int				m_nPercentComplete;
	DWORD			m_dwStartProgressTicks;
	CString			m_strTitle;
	CString			m_strText;
	CString			m_strCancelLabel;
	CString			m_strTimeLeftLabel;
	CString			m_strHour, m_strMin, m_strSec;
	CFont			m_font;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXProgressWnd)
public:
	virtual BOOL DestroyWindow();
	//}}AFX_VIRTUAL


// Generated message map functions
protected:
	//{{AFX_MSG(CXProgressWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnCancel();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


#endif //XPROGRESSWND_H
