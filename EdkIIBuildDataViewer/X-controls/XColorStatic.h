// Code located at http://www.codeproject.com/Articles/5242/XColorStatic-a-colorizing-static-control
// License located at http://www.codeproject.com/info/EULA.aspx
// Code change blocks marked with // start modified code: and // end modified code

// XColorStatic.h  Version 1.0
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

#ifndef XCOLORSTATIC_H
#define XCOLORSTATIC_H

/////////////////////////////////////////////////////////////////////////////
// CXColorStatic window

class CXColorStatic : public CStatic
{
// Construction
public:
	CXColorStatic();
	virtual ~CXColorStatic();

// Attributes
public:
	void SetBackgroundColor(COLORREF rgb, BOOL bRedraw = TRUE);
	void SetTextColor(COLORREF rgb, BOOL bRedraw = TRUE);
	void SetBold(BOOL bFlag, BOOL bRedraw = TRUE);
// start modified code: add underline style
	void SetUnderline(BOOL bFlag, BOOL bRedraw = TRUE);
// end modified code
	void SetFont(LPCTSTR lpszFaceName, int nPointSize, BOOL bRedraw = TRUE);
	void SetFont(LOGFONT * pLogFont, BOOL bRedraw = TRUE);
	void SetFont(CFont *pFont, BOOL bRedraw = TRUE);
	void SetIcon(HICON hIcon, BOOL bRedraw = TRUE);
	void SetMargins(int x, int y) { m_nXMargin = x; m_nYMargin = y; }
// start modified code: add vertical scrollbar
	void ShowText(CString strText);
// end modified code

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXColorStatic)
protected:
    virtual void PreSubclassWindow();
// start modified code: add vertical scrollbar
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
// end modified code
	//}}AFX_VIRTUAL

// Implementation
protected:
	CFont		m_font;
	COLORREF	m_rgbText;
	COLORREF	m_rgbBackground;
	CBrush *	m_pBrush;
	BOOL		m_bBold;
// start modified code: add underline style
	BOOL		m_bUnderline;
// end modified code
	int			m_nXMargin, m_nYMargin;
	HICON		m_hIcon;
// start modified code: add vertical scrollbar
	int			m_iLineHeight;

	void ResetScrollBar(CRect* a_pMainRect, CRect* a_pWithTextRect);
// end modified code

	// Generated message map functions
protected:
	//{{AFX_MSG(CXColorStatic)
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
// start modified code: add vertical scrollbar
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
// end modified code
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //XCOLORSTATIC_H
