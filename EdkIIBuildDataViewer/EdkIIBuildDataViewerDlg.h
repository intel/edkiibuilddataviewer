/*++
  Copyright (c) 2012-2013, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// EDKIIBuildDataViewerDlg.h : header file

#pragma once

#include "afxcmn.h"
#include "afxwin.h"
#include "ChooseEditor.h"
#include "CustomListCtrl.h"

// start: 3rd party code
#include "ColumnTreeWnd.h"
#include "XProgressWnd.h"
#include "XColorStatic.h"
#include "XGroupBox.h"
#include "XIcon.h"
#include "EasySize.h"
#include "XGetopt.h"
#include "argcargv.h"
// end: 3rd party code

#define	MYMSGID_TREEDEC_DBLCLK_LAUNCH_EDITOR		(WM_APP + 0)
#define	MYMSGID_TREEMODULE_DBLCLK_FIND_ITEM			(WM_APP + 1)

#define PCD_LIST_ITEM_DATA_GUID				-1
#define PCD_LIST_ITEM_DATA_PCD				0
#define PCD_LIST_ITEM_DATA_PCD_OVERRIDE		1

// enums for radio buttons for View File/Data Type
// The first radio button must be GROUP=TRUE and the rest GROUP=FALSE so the radio value is sequential.
// When changing the radio buttons, the order on the screen and the Tab Order must match the enums.
typedef enum {
	e_RadioViewGuidModule = 0,
	e_RadioViewGuidVariables,
	e_RadioViewPcdDefinitions,
	e_RadioViewPcdUsedInBuild,
	e_RadioViewModuleUsedInBuild,
	e_RadioViewInfUsedInBuild,
	e_RadioViewDecUsedInBuild,
	e_RadioViewSourceUsedInBuild,
	e_RadioViewFdfLayout,
	e_RadioView_MAX
} e_RadioViewType;

// enums for Module Summary types
typedef enum {
	e_ModuleSummaryModuleName = 0,
	e_ModuleSummaryPcd,
	e_ModuleSummaryLibrary,
	e_ModuleSummaryDepEx,
	e_ModuleSummaryBuildFlags,
	e_ModuleSummaryFixedAddress,
	e_ModuleSummary_MAX
} e_ModuleSummaryType;

// enums for GUID types
typedef enum {
	e_GuidTypeVariable = 0,
	e_GuidTypeProtocol,
	e_GuidTypePpi,
	e_GuidTypeFileInf,
	e_GuidTypePackageDec,
	e_GuidTypePackageDsc,
	e_GuidTypeFdf,
	e_GuidType_MAX
} e_GuidType;

#define RADIO_SEARCH_NAME_START				0
#define RADIO_SEARCH_VALUE_START			1

typedef struct {
  UINT32  Data1;
  UINT16  Data2;
  UINT16  Data3;
  UINT8   Data4[8];
} EFI_GUID;

typedef struct _t_GuidPool {
	EFI_GUID			Guid;
	UINT8				GuidType;
	CString				GuidName;
	CString				Filename;
	UINT32				LineNum;
	BOOL				bDuplicate;
	UINT32				indexDuplicate;
} t_GuidPool;

typedef struct _t_FVModule {
	CString				Offset;
	CString				ModuleName;
	CString				InfFileName;
} t_FVModule;

typedef struct _t_FV {
	CString				BaseAddr;
	CString				Size;
	CString				FvName;
	CString				OccupiedSize;
	CString				FreeSize;
	vector<t_FVModule>	FVModules;
} t_FV;

typedef struct _t_FD {
	CString				BaseAddr;
	CString				Size;
	CString				FdName;
	vector<t_FV>		FV;
} t_FD;

typedef struct _t_Dec {
	CString				DecFileName;
	UINT32				indexGuidPool;
	EFI_GUID			PackageGuid;
} t_DEC;

typedef struct _t_DecInInf {
	CString				DecFileName;
	UINT32				InfLineNum;
} t_DecInInf;

typedef struct _t_DecLibraryClass {
	CString				DecFileName;
	UINT32				DecLineNum;
	CString				LibName;
	CString				ImplementFileName;
} t_DecLibraryClass;

typedef struct _t_InfBuild {
	CString				InfFileName;
	CString				ModuleType;
	CString				LibraryClass;
	vector<CString>		vLibraryTypes;
	vector<t_DecInInf>	vPackages;
	CString				EntryPoint;
	UINT				indexDecLibClass;
} t_InfBuild;

typedef struct _t_GUID {
	UINT32				indexGuidPool;
} t_GUID;

typedef struct _t_PCD {
	CString				DecFileName;
	CString				GuidNameSpace;
	CString				PcdName;
	CString				Value;
	CString				Type;
	UINT32				TokenNum;
	UINT32				LineNum;
	BOOL				bDuplicate;
	UINT32				indexDuplicate;
} t_PCD;

typedef struct _t_GUIDIndex_PCDCount {
	UINT32				GuidIndex;
	UINT32				PcdCount;
} t_GUIDIndex_PCDCount;

typedef struct _t_ListSourceItemData {
	BOOL				bIsInfFile;
	UINT32				indexDecLibClassArray;
	UINT32				indexInfRow;
	UINT32				indexInf;
} t_ListSourceItemData;

// CEDKIIBuildDataViewerDlg dialog
class CEDKIIBuildDataViewerDlg : public CDialog
{
DECLARE_EASYSIZE

// Construction
public:
	CEDKIIBuildDataViewerDlg(CWnd* pParent = NULL);	// standard constructor
	~CEDKIIBuildDataViewerDlg();

// Dialog Data
	enum { IDD = IDD_EDKIIBUILDDATAVIEWER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON	m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	// user defined variables
	vector<t_GuidPool>	m_vGuidPool;
	vector<t_GUID>		m_vGUID;
	vector<t_PCD>		m_vPCD;
	vector<t_InfBuild>	m_vInfBuild;
	CStringArray		m_DecFindArray;
	CStringArray		m_InfArray;
	CArray<t_DEC, t_DEC>	m_DEC;
	CArray<t_FD, t_FD>		m_FDF;
	CArray<t_GUIDIndex_PCDCount, t_GUIDIndex_PCDCount>	m_GuidIndecesForListPcdUse;
	CArray<t_DecLibraryClass,t_DecLibraryClass>	m_DecLibClassArray;
	CArray<t_ListSourceItemData,t_ListSourceItemData> m_ListSourceItemData;

	CChooseEditor		m_chooseEditorDlg;
	CXProgressWnd		*m_pwndProgress;
	CString				m_editorExe;
	CString				m_editorSwitches;
	int					m_minsizeX, m_minsizeY;

	// user defined functions
	void				InitBuildData(BOOL bInitWinCtrl = TRUE);
	void				InitBuildReportData();
	int					ParseDecFiles();
	void				SetListTopIndex(CListCtrl* pListCtrl, UINT nRow, BOOL bSelect);
	void				SetTreeTopIndex(CTreeCtrl* pTreeCtrl, HTREEITEM hItem, BOOL bSelect);
	void				WriteSettingsToRegistry();
	void				ReadSettingsFromRegistry();
	void				Search(BOOL bDirForward = TRUE);
	int					ListCompareItem(CCustomListCtrl* pList, int index, int subItem, int subItem2);
	int					TreeWndCompareItem(CColumnTreeWnd* pTreeWnd, HTREEITEM hti, int subItem);
	CString				GetWorkspacePath();

	void				CheckGuidsForDuplicates();
	int					InsertGuidPool(EFI_GUID Guid, UINT8 GuidType, UINT32 *pIndex, CString filename, UINT32 lineNum);
	int					InsertGuidPool(CString Guid, UINT8 GuidType, UINT32 *pIndex, CString filename, UINT32 lineNum);
	int					CompareGuid (IN EFI_GUID *Guid1, IN EFI_GUID *Guid2);
	int					GuidPoolValueAsString (CString &str, UINT32 index);
	int					SearchGuidPool (EFI_GUID Guid);

protected:
	// control mapped variables
	int					m_radioView;
	int					m_radioSearch;
	CString				m_buildLog;
	CString				m_buildReport;
	CString				m_workspace;
	CString				m_target;
	CString				m_targetArch;
	CString				m_toolChain;
	CString				m_packageDSC;
	CString				m_packageFDF;
	CString				m_buildOutputDir;
	CString				m_search;
	BOOL				m_searchExactMatch;
	BOOL				m_searchCaseSensitive;
	CString				m_editorName;
	CString				m_searchStatus;
	CButton				m_radioViewBtn;

	CString				m_fileGuidModule;
	CString				m_filePcd;
	CString				m_fileInf;
	CString				m_fileFlash;
	CString				m_fileModuleSummary;

	CCustomListCtrl		m_cvListGuidModule;
	CCustomListCtrl		m_cvListGuidModuleRef;
	CCustomListCtrl		m_cvListGuidVar;
	CCustomListCtrl		m_cvListPcdDef;
	CCustomListCtrl		m_cvListPcdUse;
	CCustomListCtrl		m_cvListInf;
	CCustomListCtrl		m_cvListSource;
	CCustomListCtrl		m_cvListFdf;

	CColumnTreeWnd		m_cvTreeFdf;
	CColumnTreeWnd		m_cvTreeDec;
	CColumnTreeWnd		m_cvTreeModuleSummary;

	CXGroupBox			m_XGroupBoxSearch;
	CXGroupBox			m_XGroupBoxBuildCfg;
	CXGroupBox			m_XGroupBoxEditor;
	CXGroupBox			m_XGroupBoxViewTypes;

	CButton				m_buildReportBuildFlags;
	CButton				m_buildReportDepex;
	CButton				m_buildReportExecutionOrder;
	CButton				m_buildReportFixedAddress;
	CButton				m_buildReportFlash;
	CButton				m_buildReportLibrary;
	CButton				m_buildReportPcd;

	CButton				m_searchInf;
	CButton				m_searchLibrary;
	CButton				m_searchSource;

	CXColorStatic		m_staticSearchOptions;
	CXColorStatic		m_staticSourceItems;
	CXColorStatic		m_staticLeftList;
	CXColorStatic		m_staticRightList;

	CXIcon				m_IntelLogo;

public:
	// control and message mapped functions
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);

	afx_msg void OnBnClickedSelectBuildLog();
	afx_msg void OnBnClickedSelectBuildReport();
	afx_msg void OnBnClickedWriteSourceList();
	afx_msg void OnBnClickedRadioFileData();
	afx_msg void OnBnClickedCheckBuildReport();
	afx_msg void OnBnClickedChangeEditor();
	afx_msg void OnClickedSearchButton();

	afx_msg void OnNMClickListGuidVar(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListPcdUse(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListPcdDef(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMRClickListGuidVar(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickListPcdDef(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMDblclkLaunchEditor(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnFdfLayoutTreeSelchanged(NMHDR *pNMHDR, LRESULT *pResult);

	afx_msg void OnNMDblclkList_ShowModuleTree(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkTreeModule(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkTreeModule_FindItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMDblclkTreeDec(NMHDR *pNMHDR, LRESULT *pResult);

#ifdef SYSTEM_WIDE_HOTKEY
	afx_msg void OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2);
#endif
};
