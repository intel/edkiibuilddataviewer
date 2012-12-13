// Code located at http://www.codeproject.com/Articles/29016/XGroupBox-an-MFC-groupbox-control-to-display-text
// License located at http://www.codeproject.com/info/EULA.aspx

// XGroupBox.h  Version 1.0
// http://www.codeproject.com/Articles/29016/XGroupBox-an-MFC-groupbox-control-to-display-text
//
// Author:  Hans Dietrich
//          hdietrich@gmail.com
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

#ifndef XGROUPBOX_H
#define XGROUPBOX_H

//=============================================================================
class CXGroupBox : public CStatic
//=============================================================================
{
// Construction
public:
	CXGroupBox();
	virtual ~CXGroupBox();

// Attributes
public:
	enum { DEFAULT_X_MARGIN = 8,
		   DEFAULT_ICON_SPACING = 8,
		   DEFAULT_ICON_SIZE = 16 };

	enum CONTROL_STYLE { groupbox = 0, header };

	enum BORDER_STYLE { flat = 0, etched };

	enum ALIGNMENT { left = 0, center, right };

	CXGroupBox& SetAlignment(ALIGNMENT eAlign, BOOL bRedraw = TRUE);
	CXGroupBox& SetBold(BOOL bBold, BOOL bRedraw = TRUE);
	CXGroupBox& SetBorderColor(COLORREF crBorder, BOOL bRedraw = TRUE);
	CXGroupBox& SetBorderStyle(BORDER_STYLE eStyle, BOOL bRedraw = TRUE);
	CXGroupBox& SetControlStyle(CONTROL_STYLE eStyle, BOOL bRedraw = TRUE);
	CXGroupBox& SetDisabledStyle(BOOL bShowDisabledState, BOOL bRedraw = TRUE);
	CXGroupBox& SetFont(CFont *pFont, BOOL bRedraw = TRUE);
	CXGroupBox& SetFont(LOGFONT * pLogFont, BOOL bRedraw = TRUE);
	CXGroupBox& SetFont(LPCTSTR lpszFaceName, int nPointSize, BOOL bRedraw = TRUE);
	CXGroupBox& SetIcon(HICON hIcon, UINT nIconSize = 16, BOOL bRedraw = TRUE);
	CXGroupBox& SetIcon(UINT nIconId, UINT nIconSize = 16, BOOL bRedraw = TRUE);
	CXGroupBox& SetIconAlignment(ALIGNMENT eAlign, BOOL bRedraw = TRUE);
	CXGroupBox& SetIconSpacing(int nIconSpacing, BOOL bRedraw = TRUE);
	CXGroupBox& SetImageList(CImageList * pImageList, int nIconNo, BOOL bRedraw = TRUE);
	CXGroupBox& SetMargins(int nXMargin, int nYMargin, BOOL bRedraw = TRUE);
	CXGroupBox& SetTextColor(COLORREF cr, BOOL bRedraw = TRUE);
	CXGroupBox& SetWindowText(LPCTSTR lpszText, BOOL bRedraw = TRUE);

	ALIGNMENT GetAlignment()		{ return m_eControlAlignment; }
	BOOL GetBold();
	COLORREF GetBorderColor()		{ return m_crBorder; }
	BORDER_STYLE GetBorderStyle()	{ return m_eBorderStyle; }
	BOOL GetDisabledStyle()			{ return m_bShowDisabledState; }
	CFont* GetFont()				{ return &m_font; }
	BOOL GetFont(LOGFONT *pLF);
	CString GetFontFaceName();
	int GetFontPointSize();
	ALIGNMENT GetIconAlignment()	{ return m_eIconAlignment; }
	int GetIconSpacing()			{ return m_nIconSpacing; }
	void GetMargins(int& nXMargin, int& nYMargin)
	{ nXMargin = m_nXMargin; nYMargin = m_nYMargin; }
	CONTROL_STYLE GetControlStyle()	{ return m_eStyle; }
	COLORREF GetTextColor()			{ return m_crText; }

	CXGroupBox& EnableTheme(BOOL bEnable, BOOL bRedraw = TRUE);
	BOOL EnableWindow(BOOL bEnable = TRUE, BOOL bRecurseChildren = FALSE);

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CXGroupBox)
protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
protected:
	CFont			m_font;					// font used for text
	COLORREF		m_crText;				// text color
	COLORREF		m_crBackground;			// background color
	COLORREF		m_crBorder;				// user-specified border color
	int				m_nXMargin, m_nYMargin;	// space in pixels between border
											// and icon/text
	int				m_nYOffset;				// y offset of frame (text extends 
											// above frame)
	int				m_nIconSpacing;			// horizontal space in pixels 
											// between icon and text
	HICON			m_hIcon;				// icon handle
	HICON			m_hGrayIcon;			// disabled icon handle
	int				m_nIconSize;			// size of icon, default to 16
	BOOL			m_bDestroyIcon;			// TRUE = icon will be destroyed
											// (internal only)
	CRect			m_rect;
	CSize			m_sizeText;				// size of text from GetTextExtent()
	BOOL			m_bThemed;				// TRUE = themes are enabled
	BOOL			m_bEnableTheme;			// TRUE = theme will be used if 
											// enabled
	BOOL			m_bShowDisabledState;	// TRUE = gray text and icon will 
											// be displayed if control is 
											// disabled
	BOOL			m_bGrayPaletteSet;		// used for gray scale icon
	static COLORREF	m_defaultGrayPalette[256]; // used for gray scale icon
	CONTROL_STYLE	m_eStyle;				// disabled groupbox or header
	BORDER_STYLE	m_eBorderStyle;			// border line style - flat or etched;
											// applies only if no border color
											// and no theme
	ALIGNMENT		m_eIconAlignment;		// left (of text) or right;  note 
											// that control's style bits
											// control header alignment
	ALIGNMENT		m_eControlAlignment;	// alignment for control

	HICON		CreateGrayscaleIcon(HICON hIcon);
	HICON		CreateGrayscaleIcon(HICON hIcon, COLORREF *pPalette);
	void		DrawBorder(CDC *pDC, CRect& rect);
	void		DrawEtchedLine(CDC *pDC, CRect& rect, COLORREF cr);
	void		DrawEtchedRectangle(CDC *pDC, CRect& rect, COLORREF cr);
	void		DrawHeader(CDC *pDC, CRect& rectText, CRect& rectIcon);
	int			DrawIcon(CDC *pDC, CRect& rect);
	void		DrawItem(CDC *pDC, CRect& rect);
	int			DrawText(CDC *pDC, CRect& rect);
	int			EnableChildWindows(HWND hWnd, BOOL bEnable, BOOL bEnableParent);
	void		EraseBorder(CDC *pDC, CRect& rect, COLORREF crBackground);
	void		EraseHeaderArea(CDC *pDC, CRect& rect, COLORREF crBackground);
	int			GetFontHeight(int nPointSize);
	int			GetFontPointSize(int nHeight);
	void		GetHeaderRect(CRect& rect);
	CFont *		GetSafeFont();

	// Generated message map functions
	//{{AFX_MSG(CXGroupBox)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif //XGROUPBOX_H
