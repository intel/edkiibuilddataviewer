/*++
  Copyright (c) 2012, Intel Corporation. All rights reserved.
  This program and the accompanying materials                          
  are licensed and made available under the terms and conditions of the BSD License         
  which accompanies this distribution.  The full text of the license may be found at        
  http://opensource.org/licenses/bsd-license.php                                            

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.    
--*/

// EDKIIBuildDataViewerDlg.cpp : implementation file

#include "stdafx.h"
#include "EDKIIBuildDataViewer.h"
#include "EDKIIBuildDataViewerDlg.h"
#include "AboutDlg.h"
#include "afxwin.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// see PreTranslateMessage() for hotkey definitions

/*++
  Function: CEDKIIBuildDataViewerDlg constructor

  Parameters: ptr to parent CWnd

  Purpose: main dialog constructor; called before OnInitDialog()

  Returns: none
--*/
CEDKIIBuildDataViewerDlg::CEDKIIBuildDataViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CEDKIIBuildDataViewerDlg::IDD, pParent)
	, m_pwndProgress(NULL)
	, m_searchExactMatch(FALSE)
	, m_searchCaseSensitive(FALSE)
	, m_editorName(_T(""))
{
//	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIcon = AfxGetApp()->LoadIcon(IDI_INTEL);
	InitBuildData(FALSE);
	ReadSettingsFromRegistry();
}

/*++
  Function: InitBuildData

  Parameters: bInitWinCtrl
				TRUE  - clear all list and trees controls
				FALSE - do not clear all list and trees controls

  Purpose: initialize build log default values for static controls, and clear data structures

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::InitBuildData(BOOL bInitWinCtrl /*= TRUE*/) {
	// set default controls
	m_radioView = 0;
	m_radioSearch = 0;

	// set default strings
	m_buildLog			= _T("");
	m_buildYLog			= _T("");
	m_workspace			= _T("<select EDK II build log file>");
	m_target			= _T("<select EDK II build log file>");
	m_targetArch		= _T("<select EDK II build log file>");
	m_toolChain			= _T("<select EDK II build log file>");
	m_packageDSC		= _T("<select EDK II build log file>");
	m_packageFDF		= _T("<select EDK II build log file>");
	m_buildOutputDir	= _T("<select EDK II build log file>");
	m_search			= _T("");
	m_searchStatus		= _T("");

	// if Windows controls are to be initialized ...
	if (bInitWinCtrl) {
		// clear control strings
		GetDlgItem(IDC_SELECT_Y_LOG)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_BUILD_LOG)->SetWindowTextW(_T(""));
		UpdateData(FALSE);

		// clear list controls
		m_cvListGuidModule.DeleteAllItems();
		m_cvListGuidModuleRef.DeleteAllItems();
		m_cvListGuidVar.DeleteAllItems();
		m_cvListPcdDef.DeleteAllItems();
		m_cvListPcdUse.DeleteAllItems();
		m_cvListInf.DeleteAllItems();
		m_cvListSource.DeleteAllItems();
		m_cvListFdf.DeleteAllItems();

		// clear tree controls
		m_cvTreeFdf.GetTreeCtrl().DeleteAllItems();
		m_cvTreeDec.GetTreeCtrl().DeleteAllItems();
		m_cvTreeModuleSummary.GetTreeCtrl().DeleteAllItems();
	}

	// clear internal data structures
	m_vGuidPool.clear();
	m_vGUID.clear();
	m_vPCD.clear();
	m_vInfBuild.clear();
	m_DecFindArray.RemoveAll();
	m_InfArray.RemoveAll();
	m_DEC.RemoveAll();
	m_FDF.RemoveAll();
	m_GuidIndecesForListPcdUse.RemoveAll();
}


/*++
  Function: InitBuildReportData

  Parameters: none

  Purpose: initialize build report -y log checkboxes, and clear data structures

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::InitBuildReportData() {
	// clear all checks because it is not known what sections are present in the report
	m_ylogPcd.SetCheck(BST_UNCHECKED);
	m_ylogLibrary.SetCheck(BST_UNCHECKED);
	m_ylogFlash.SetCheck(BST_UNCHECKED);
	m_ylogDepex.SetCheck(BST_UNCHECKED);
	m_ylogBuildFlags.SetCheck(BST_UNCHECKED);
	m_ylogFixedAddress.SetCheck(BST_UNCHECKED);
	m_ylogExecutionOrder.SetCheck(BST_UNCHECKED);
}

/*++
  Function: CEDKIIBuildDataViewerDlg destructor

  Parameters: none

  Purpose: main dialog destructor; called before dialog OnOK() and OnCancel() to exit app

  Returns: none
--*/
CEDKIIBuildDataViewerDlg::~CEDKIIBuildDataViewerDlg()
{
	InitBuildData(FALSE);
	WriteSettingsToRegistry();
}
	

/*++
  Function: DoDataExchange

  Parameters: pDX - ptr to data exchange

  Purpose: maps variables to control IDs

  Returns: none
--*/
void CEDKIIBuildDataViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_BUILD_LOG, m_buildLog);
	DDX_Text(pDX, IDC_EDIT_Y_LOG, m_buildYLog);
	DDX_Text(pDX, IDC_STATIC_BUILD_OUTPUT_DIR, m_buildOutputDir);
	DDX_Text(pDX, IDC_STATIC_WORKSPACE, m_workspace);
	DDX_Text(pDX, IDC_STATIC_TARGET, m_target);
	DDX_Text(pDX, IDC_STATIC_TARGET_ARCH, m_targetArch);
	DDX_Text(pDX, IDC_STATIC_TOOL_CHAIN, m_toolChain);
	DDX_Text(pDX, IDC_STATIC_PACKAGE_DSC, m_packageDSC);
	DDX_Text(pDX, IDC_STATIC_PACKAGE_FDF, m_packageFDF);
	DDX_Radio(pDX, IDC_RADIO_GUID_MODULE, m_radioView);
	DDX_Radio(pDX, IDC_RADIO_GUID_NAME_START, m_radioSearch);
	DDX_Control(pDX, IDC_LIST_GUID_MODULE, m_cvListGuidModule);
	DDX_Control(pDX, IDC_LIST_GUID_MODULE_REF, m_cvListGuidModuleRef);
	DDX_Control(pDX, IDC_LIST_GUID_VAR, m_cvListGuidVar);
	DDX_Control(pDX, IDC_LIST_PCD_USE, m_cvListPcdUse);
	DDX_Control(pDX, IDC_LIST_PCD_DEF, m_cvListPcdDef);
	DDX_Control(pDX, IDC_LIST_INF, m_cvListInf);
	DDX_Control(pDX, IDC_LIST_SOURCE, m_cvListSource);
	DDX_Control(pDX, IDC_LIST_FDF, m_cvListFdf);
	DDX_Control(pDX, IDC_CHECK_Y_LOG_BUILD_FLAGS, m_ylogBuildFlags);
	DDX_Control(pDX, IDC_CHECK_Y_LOG_DEPEX, m_ylogDepex);
	DDX_Control(pDX, IDC_CHECK_Y_LOG_EXECUTION_ORDER, m_ylogExecutionOrder);
	DDX_Control(pDX, IDC_CHECK_Y_LOG_FIXED_ADDRESS, m_ylogFixedAddress);
	DDX_Control(pDX, IDC_CHECK_Y_LOG_FLASH, m_ylogFlash);
	DDX_Control(pDX, IDC_CHECK_Y_LOG_LIBRARY, m_ylogLibrary);
	DDX_Control(pDX, IDC_CHECK_Y_LOG_PCD, m_ylogPcd);
	DDX_Text(pDX, IDC_EDIT_SEARCH, m_search);
	DDX_Check(pDX, IDC_CHECK_SEARCH_MATCH, m_searchExactMatch);
	DDX_Check(pDX, IDC_CHECK_SEARCH_CASE_SENSITIVE, m_searchCaseSensitive);
	DDX_Text(pDX, IDC_EDITOR_NAME, m_editorName);
	DDX_Text(pDX, IDC_STATIC_SEARCH_STATUS, m_searchStatus);
	DDX_Control(pDX, IDC_RADIO_GUID_MODULE, m_radioViewBtn);
	DDX_Control(pDX, IDC_CHECK_SEARCH_INF_NAME, m_searchInf);
	DDX_Control(pDX, IDC_CHECK_SEARCH_LIBRARY_NAME, m_searchLibrary);
	DDX_Control(pDX, IDC_CHECK_SEARCH_SOURCE_NAME, m_searchSource);
	DDX_Control(pDX, IDC_STATIC_SEARCH_OPTIONS, m_staticSearchOptions);
	DDX_Control(pDX, IDC_STATIC_LEFTLIST, m_staticLeftList);
	DDX_Control(pDX, IDC_STATIC_RIGHTLIST, m_staticRightList);
	DDX_Control(pDX, IDC_STATIC_SOURCE_ITEMS, m_staticSourceItems);
	DDX_Control(pDX, IDC_GROUPBOX_SEARCH, m_XGroupBoxSearch);
	DDX_Control(pDX, IDC_GROUPBOX_BUILDCFG, m_XGroupBoxBuildCfg);
	DDX_Control(pDX, IDC_GROUPBOX_EDITOR, m_XGroupBoxEditor);
	DDX_Control(pDX, IDC_GROUPBOX_VIEWTYPES, m_XGroupBoxViewTypes);
	DDX_Control(pDX, IDC_INTEL_BUTTON, m_IntelLogo);
}



BEGIN_MESSAGE_MAP(CEDKIIBuildDataViewerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_BN_CLICKED(IDC_SELECT_BUILD_LOG, &CEDKIIBuildDataViewerDlg::OnBnClickedSelectBuildLog)
	ON_BN_CLICKED(IDC_SELECT_Y_LOG, &CEDKIIBuildDataViewerDlg::OnBnClickedSelectYLog)
	ON_BN_CLICKED(IDC_RADIO_GUID_MODULE, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_RADIO_GUID_VAR, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_RADIO_PCD_USE, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_RADIO_PCD_DEF, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_RADIO_MODULE_USE, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_RADIO_INF, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_RADIO_DEC, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_RADIO_SOURCE, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_RADIO_FDFLAYOUT, &CEDKIIBuildDataViewerDlg::OnBnClickedRadioFileData)
	ON_BN_CLICKED(IDC_CHECK_Y_LOG_PCD, &CEDKIIBuildDataViewerDlg::OnBnClickedCheckYLog)
	ON_BN_CLICKED(IDC_CHECK_Y_LOG_LIBRARY, &CEDKIIBuildDataViewerDlg::OnBnClickedCheckYLog)
	ON_BN_CLICKED(IDC_CHECK_Y_LOG_FLASH, &CEDKIIBuildDataViewerDlg::OnBnClickedCheckYLog)
	ON_BN_CLICKED(IDC_CHECK_Y_LOG_DEPEX, &CEDKIIBuildDataViewerDlg::OnBnClickedCheckYLog)
	ON_BN_CLICKED(IDC_CHECK_Y_LOG_BUILD_FLAGS, &CEDKIIBuildDataViewerDlg::OnBnClickedCheckYLog)
	ON_BN_CLICKED(IDC_CHECK_Y_LOG_FIXED_ADDRESS, &CEDKIIBuildDataViewerDlg::OnBnClickedCheckYLog)
	ON_BN_CLICKED(IDC_CHECK_Y_LOG_EXECUTION_ORDER, &CEDKIIBuildDataViewerDlg::OnBnClickedCheckYLog)
	ON_BN_CLICKED(IDC_CHANGE_EDITOR, &CEDKIIBuildDataViewerDlg::OnBnClickedChangeEditor)
	ON_BN_CLICKED(IDC_SEARCH_BUTTON, &CEDKIIBuildDataViewerDlg::OnClickedSearchButton)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_INF, &CEDKIIBuildDataViewerDlg::OnNMDblclkLaunchEditor)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PCD_USE, &CEDKIIBuildDataViewerDlg::OnNMDblclkLaunchEditor)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_PCD_DEF, &CEDKIIBuildDataViewerDlg::OnNMDblclkLaunchEditor)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_GUID_VAR, &CEDKIIBuildDataViewerDlg::OnNMDblclkLaunchEditor)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_SOURCE, &CEDKIIBuildDataViewerDlg::OnNMDblclkLaunchEditor)

// IDC_TREE_DEC needs double-click event so filtering can be done on per-column basis.
// Some of the columns require the editor to be launched instead of expanding the branch.
// In order to have 2 double-click events for a single control, one control must receive
// ON_NOTIFY, and that function sends a custom message to the other double-click events.
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_DEC, &CEDKIIBuildDataViewerDlg::OnNMDblclkTreeDec)
// Send double-click event with custom msg so double-clicking DEC file will launch it in editor.
	ON_NOTIFY(MYMSGID_TREEDEC_DBLCLK_LAUNCH_EDITOR, IDC_TREE_DEC, &CEDKIIBuildDataViewerDlg::OnNMDblclkLaunchEditor)

// IDC_TREE_MODULE needs double-click event so filtering can be done on per-column basis.
// Some of the columns require finding the item in another view instead of expanding the branch.
// In order to have 2 double-click events for a single control, one control must receive
// ON_NOTIFY, and that function sends a custom message to the other double-click events.
	ON_NOTIFY(NM_DBLCLK, IDC_TREE_MODULE, &CEDKIIBuildDataViewerDlg::OnNMDblclkTreeModule)
// Send double-click event with custom msg so double-clicking INF file will find the item.
	ON_NOTIFY(MYMSGID_TREEMODULE_DBLCLK_FIND_ITEM, IDC_TREE_MODULE, &CEDKIIBuildDataViewerDlg::OnNMDblclkTreeModule_FindItem)

	ON_NOTIFY(NM_CLICK, IDC_LIST_GUID_VAR, &CEDKIIBuildDataViewerDlg::OnNMClickListGuidVar)
	ON_NOTIFY(NM_CLICK, IDC_LIST_PCD_USE, &CEDKIIBuildDataViewerDlg::OnNMClickListPcdUse)
	ON_NOTIFY(NM_CLICK, IDC_LIST_PCD_DEF, &CEDKIIBuildDataViewerDlg::OnNMClickListPcdDef)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_GUID_VAR, &CEDKIIBuildDataViewerDlg::OnNMRClickListGuidVar)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_PCD_DEF, &CEDKIIBuildDataViewerDlg::OnNMRClickListPcdDef)
	ON_NOTIFY(TVN_SELCHANGED, IDC_TREE_FDF, &CEDKIIBuildDataViewerDlg::OnFdfLayoutTreeSelchanged)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FDF, &CEDKIIBuildDataViewerDlg::OnNMDblclkList_ShowModuleTree)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_GUID_MODULE, &CEDKIIBuildDataViewerDlg::OnNMDblclkList_ShowModuleTree)
#ifdef SYSTEM_WIDE_HOTKEY
	ON_WM_HOTKEY()
#endif
	ON_WM_SIZE()
	ON_WM_SIZING()
END_MESSAGE_MAP()

BEGIN_EASYSIZE_MAP(CEDKIIBuildDataViewerDlg)
//	EASYSIZE(control,left,top,right,bottom,options)
	EASYSIZE(IDC_LIST_GUID_MODULE,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)

	// set RHS rect bottom to same as LHS because it is smaller in resource editor to allow for definition of other hidden list controls.
	EASYSIZE(IDC_STATIC_RIGHTLIST,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
	EASYSIZE(IDC_LIST_GUID_MODULE_REF,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_BORDER,0)

	// make lists as wide as the 2 controls, since there is only 1 list control for these
	EASYSIZE(IDC_LIST_INF,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_LIST_GUID_VAR,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_LIST_PCD_USE,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_LIST_PCD_DEF,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_LIST_SOURCE,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)

	// make lists as wide as the 2 controls, since there is only 1 list control for these, and allow variable height
	EASYSIZE(IDC_LIST_FDF,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)

	// make trees as wide as the 2 controls, since there is only 1 list control for these, and fixed height (bottom doesn't resize)
	EASYSIZE(IDC_TREE_FDF,ES_BORDER,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

	// make trees as wide as the 2 controls, since there is only 1 list control for these, and allow variable height
	EASYSIZE(IDC_TREE_DEC,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_TREE_MODULE,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
END_EASYSIZE_MAP


/*++
  Function: OnInitDialog

  Parameters: none

  Purpose: initializes main app dialog

  Returns: none
--*/
BOOL CEDKIIBuildDataViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_radioView = 0;
	m_radioSearch = 0;

	CRect	rect, rect2;
	// get GUID Module (Left Hand Side, LHS) rect
	GetDlgItem(IDC_LIST_GUID_MODULE)->GetWindowRect(&rect);
	ScreenToClient(&rect);
	// get GUID Module Ref (Right Hand Side, RHS) rect
	GetDlgItem(IDC_LIST_GUID_MODULE_REF)->GetWindowRect(&rect2);
	ScreenToClient(&rect2);
	// set RHS rect bottom to same as LHS because it is smaller in resource editor to allow for definition of other hidden list controls.
	rect2.bottom = rect.bottom;
	GetDlgItem(IDC_LIST_GUID_MODULE_REF)->SetWindowPos(&CWnd::wndTop, rect2.left, rect2.top, rect2.right - rect2.left, rect2.Height(), 0);

	// make lists as wide as the 2 controls, since there is only 1 list control for these
	GetDlgItem(IDC_LIST_INF)->SetWindowPos(&CWnd::wndTop, rect.left, rect.top, rect2.right - rect.left, rect.Height(), 0);
	GetDlgItem(IDC_LIST_GUID_VAR)->SetWindowPos(&CWnd::wndTop, rect.left, rect.top, rect2.right - rect.left, rect.Height(), 0);
	GetDlgItem(IDC_LIST_PCD_USE)->SetWindowPos(&CWnd::wndTop, rect.left, rect.top, rect2.right - rect.left, rect.Height(), 0);
	GetDlgItem(IDC_LIST_PCD_DEF)->SetWindowPos(&CWnd::wndTop, rect.left, rect.top, rect2.right - rect.left, rect.Height(), 0);
	GetDlgItem(IDC_LIST_SOURCE)->SetWindowPos(&CWnd::wndTop, rect.left, rect.top, rect2.right - rect.left, rect.Height(), 0);

	// make lists as wide as the 2 controls, since there is only 1 list control for these, but varied height
	GetDlgItem(IDC_LIST_FDF)->SetWindowPos(&CWnd::wndTop, rect.left, (int) (rect.top + rect.Height() * 0.50 + 7), rect2.right - rect.left, (int) (rect.Height() - rect.Height() * 0.50), 0);

   // create the multi-column tree window
	RECT	rcTreeWnd;
	rcTreeWnd.left = rect.left;
	rcTreeWnd.top = rect.top;
	rcTreeWnd.right = rect2.right;
	rcTreeWnd.bottom = rect.top + (int) (rect.Height() * 0.50);

	// FDF tree: destroy placeholder and recreate window at desired position so custom control OnCreate() will be called
	GetDlgItem(IDC_TREE_FDF)->DestroyWindow();
	m_cvTreeFdf.CreateEx(WS_EX_CLIENTEDGE, NULL, NULL, WS_CHILD, rcTreeWnd, this, IDC_TREE_FDF);
	// adjust tree's style
	m_cvTreeFdf.GetTreeCtrl().ModifyStyle(0, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_FULLROWSELECT | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS);

	// DEC tree: destroy placeholder and recreate window at desired position so custom control OnCreate() will be called
	GetDlgItem(IDC_TREE_DEC)->DestroyWindow();
	rcTreeWnd.bottom = rect.top + rect.Height();
	m_cvTreeDec.CreateEx(WS_EX_CLIENTEDGE, NULL, NULL, WS_CHILD, rcTreeWnd, this, IDC_TREE_DEC);
	// adjust tree's style
	m_cvTreeDec.GetTreeCtrl().ModifyStyle(0, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_FULLROWSELECT | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS);

	// Module Summary tree: destroy placeholder and recreate window at desired position so custom control OnCreate() will be called
	GetDlgItem(IDC_TREE_MODULE)->DestroyWindow();
	m_cvTreeModuleSummary.CreateEx(WS_EX_CLIENTEDGE, NULL, NULL, WS_CHILD, rcTreeWnd, this, IDC_TREE_MODULE);
	// adjust tree's style
	m_cvTreeModuleSummary.GetTreeCtrl().ModifyStyle(0, TVS_HASBUTTONS | TVS_HASLINES | TVS_LINESATROOT | TVS_FULLROWSELECT | TVS_DISABLEDRAGDROP | TVS_SHOWSELALWAYS);

	// IMPORTANT: if columns are changed, then you must also change the double-click events to read data from the correct column
	// add columns to list view controls
	m_cvListGuidModule.InsertColumn(0, _T("Dispatch #"), LVCFMT_LEFT, 50);
	m_cvListGuidModule.InsertColumn(1, _T("Module"), LVCFMT_LEFT, 200);
	m_cvListGuidModule.InsertColumn(2, _T("GUID"), LVCFMT_LEFT, 300);
	m_cvListGuidModule.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);
	m_cvListGuidModuleRef.InsertColumn(0, _T("Defined in"), LVCFMT_LEFT, 500);

	m_cvListGuidVar.InsertColumn(0, _T("GUID"), LVCFMT_LEFT, 200);
	m_cvListGuidVar.InsertColumn(1, _T("Usage"), LVCFMT_LEFT, 300);
	m_cvListGuidVar.InsertColumn(2, _T("Value"), LVCFMT_LEFT, 200);
	m_cvListGuidVar.InsertColumn(3, _T("Defined in"), LVCFMT_LEFT, 300);
	m_cvListGuidVar.InsertColumn(4, _T("Line #"), LVCFMT_LEFT, 50);
	m_cvListGuidVar.InsertColumn(5, _T("Duplicate?"), LVCFMT_LEFT, 50);
	m_cvListGuidVar.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

	m_cvListPcdDef.InsertColumn(0, _T("GUID Name"), LVCFMT_LEFT, 200);
	m_cvListPcdDef.InsertColumn(1, _T("PCD Name"), LVCFMT_LEFT, 200);
	m_cvListPcdDef.InsertColumn(2, _T("Data Type"), LVCFMT_LEFT, 100);
	m_cvListPcdDef.InsertColumn(3, _T("Value"), LVCFMT_LEFT, 100);
	m_cvListPcdDef.InsertColumn(4, _T("Defined in"), LVCFMT_LEFT, 300);
	m_cvListPcdDef.InsertColumn(5, _T("Line #"), LVCFMT_LEFT, 50);
	m_cvListPcdDef.InsertColumn(6, _T("Duplicate?"), LVCFMT_LEFT, 50);
	m_cvListPcdDef.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

	m_cvListPcdUse.InsertColumn(0, _T("PCD Name"), LVCFMT_LEFT, 200);
	m_cvListPcdUse.InsertColumn(1, _T("PCD Type"), LVCFMT_LEFT, 100);
	m_cvListPcdUse.InsertColumn(2, _T("Data Type"), LVCFMT_LEFT, 100);
	m_cvListPcdUse.InsertColumn(3, _T("Value"), LVCFMT_LEFT, 100);
	m_cvListPcdUse.InsertColumn(4, _T("Override?"), LVCFMT_LEFT, 100);
	m_cvListPcdUse.InsertColumn(5, _T("Defined in"), LVCFMT_LEFT, 300);
	m_cvListPcdUse.InsertColumn(6, _T("Line #"), LVCFMT_LEFT, 50);
	m_cvListPcdUse.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

	m_cvListInf.InsertColumn(0, _T("File"), LVCFMT_LEFT, 600);
	m_cvListInf.InsertColumn(1, _T("Arch"), LVCFMT_LEFT, 100);
	m_cvListInf.InsertColumn(2, _T("DSC Line #"), LVCFMT_LEFT, 100);
	m_cvListInf.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

	m_cvListSource.InsertColumn(0, _T("File"), LVCFMT_LEFT, 1000);
	m_cvListSource.InsertColumn(1, _T("Library / Implementation"), LVCFMT_LEFT, 200);
	m_cvListSource.InsertColumn(2, _T("Module Type"), LVCFMT_LEFT, 100);
	m_cvListSource.InsertColumn(3, _T("Entry Point"), LVCFMT_LEFT, 200);
	m_cvListSource.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);

	m_cvListFdf.InsertColumn(0, _T("Offset"), LVCFMT_LEFT, 100);
	m_cvListFdf.InsertColumn(1, _T("Module"), LVCFMT_LEFT, 200);
	m_cvListFdf.InsertColumn(2, _T(".INF Filename"), LVCFMT_LEFT, 400);
	m_cvListFdf.SendMessage(LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT | LVS_EX_AUTOSIZECOLUMNS);


	// FDF: insert columns to tree control
	CHeaderCtrl& header = m_cvTreeFdf.GetHeaderCtrl();
	HDITEM hditem;
	hditem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	hditem.fmt = HDF_LEFT | HDF_STRING;
	hditem.cxy = 100;
	hditem.pszText = _T("FD Name");			header.InsertItem(0, &hditem);
	hditem.pszText = _T("FV Name");			header.InsertItem(1, &hditem);
	hditem.pszText = _T("Base Addr");		header.InsertItem(2, &hditem);
	hditem.pszText = _T("Size");			header.InsertItem(3, &hditem);
	hditem.pszText = _T("Occupied Size");	header.InsertItem(4, &hditem);
	hditem.pszText = _T("Free Size");		header.InsertItem(5, &hditem);
	m_cvTreeFdf.UpdateColumns();

	// DEC: insert columns to tree control
	// define header2 because the copy constructor doesn't exist to do "header = m_cvTreeDec.GetHeaderCtrl();"
	CHeaderCtrl& header2 = m_cvTreeDec.GetHeaderCtrl();
	hditem.pszText = _T(".DEC File");						hditem.cxy = 200;	header2.InsertItem(0, &hditem);
	hditem.pszText = _T("Package GUID / .INF reference");	hditem.cxy = 600;	header2.InsertItem(1, &hditem);
	hditem.pszText = _T("INF Line #");						hditem.cxy = 100;	header2.InsertItem(2, &hditem);
	m_cvTreeDec.UpdateColumns();

	// Module Summary: insert columns to tree control
	// define header3 because the copy constructor doesn't exist to do "header = m_cvTreeDec.GetHeaderCtrl();"
	CHeaderCtrl& header3 = m_cvTreeModuleSummary.GetHeaderCtrl();
	hditem.pszText = _T("Module Name");		hditem.cxy = 100;	header3.InsertItem(0, &hditem);
	hditem.pszText = _T("Defined in");		hditem.cxy = 200;	header3.InsertItem(1, &hditem);
	hditem.pszText = _T("GUID");			hditem.cxy = 100;	header3.InsertItem(2, &hditem);
	hditem.pszText = _T("Size / Value");	hditem.cxy = 50;	header3.InsertItem(3, &hditem);
	hditem.pszText = _T("Type");			hditem.cxy = 50;	header3.InsertItem(4, &hditem);
	m_cvTreeModuleSummary.UpdateColumns();

	// get editor data
	m_chooseEditorDlg.GetEditorData(m_editorExe, m_editorName, m_editorSwitches);
	// set default states for dialog items
	m_searchLibrary.SetCheck(BST_CHECKED);
	// update controls on dialog
	UpdateData(FALSE);

	// initialize groupboxes
	m_XGroupBoxBuildCfg.SetTextColor(RGB(0,0,255), FALSE);
	m_XGroupBoxSearch.SetTextColor(RGB(255,0,0), FALSE);
	m_XGroupBoxEditor.SetTextColor(RGB(255,0,0), FALSE);
	m_XGroupBoxViewTypes.SetTextColor(RGB(255,0,0), FALSE);

	// initialize static text items
	m_staticSearchOptions.SetBold(TRUE);
	m_staticSearchOptions.SetUnderline(TRUE);
	m_staticSourceItems.SetBold(TRUE);
	m_staticSourceItems.SetUnderline(TRUE);
	m_staticLeftList.SetBold(TRUE);
	m_staticRightList.SetBold(TRUE);

	// initialize Intel logo icon
	m_IntelLogo.SetWindowText(_T(""), FALSE);
	m_IntelLogo.SetIcon(IDI_INTEL, 32);

#ifdef SYSTEM_WIDE_HOTKEY
	// register hot keys
	RegisterHotKey(m_hWnd, WM_MYMSG_CTRL_V, MOD_NOREPEAT | MOD_CONTROL, 'V');		// Ctrl-V: cycle View selection
	RegisterHotKey(m_hWnd, WM_MYMSG_CTRL_E, MOD_NOREPEAT | MOD_CONTROL, 'E');		// Ctrl-E: change Editor
	RegisterHotKey(m_hWnd, WM_MYMSG_F2, MOD_NOREPEAT | 0, VK_F2);					// F2: Search backward
	RegisterHotKey(m_hWnd, WM_MYMSG_F3, MOD_NOREPEAT | 0, VK_F3);					// F3: Search forward
#endif

	GetWindowRect(&rect);
	ScreenToClient(&rect);
	m_minsizeX = rect.Width();
	m_minsizeY = rect.Height();
	INIT_EASYSIZE;
	return TRUE;  // return TRUE unless you set the focus to a control
}

void CEDKIIBuildDataViewerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);
	UPDATE_EASYSIZE;
}

void CEDKIIBuildDataViewerDlg::OnSizing(UINT fwSide, LPRECT pRect)
{
	CDialog::OnSizing(fwSide, pRect);
	EASYSIZE_MINSIZE(m_minsizeX,m_minsizeY,fwSide,pRect);
}

void CEDKIIBuildDataViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CEDKIIBuildDataViewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CEDKIIBuildDataViewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


/*++
  Function: PreTranslateMessage

  Parameters: pMsg - ptr to message

  Purpose: allows processing of Windows messages before they are dispatched to handlers.
		   This app checks keypresses for hotkey functionality.

  Returns: TRUE if event handled, else return value of default PreTranslateMsg()
--*/
BOOL CEDKIIBuildDataViewerDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
#ifndef SYSTEM_WIDE_HOTKEY
		if (GetAsyncKeyState(VK_CONTROL)) {
			if (pMsg->wParam == _T('T')) {
//				AfxMessageBox(_T("Ctrl-T pressed"));

				m_radioView++;
				if (m_radioView >= e_RadioView_MAX)
					m_radioView = 0;
				UpdateData(FALSE);  // set m_radioView value
				OnBnClickedRadioFileData();
				return TRUE;
			}
			else if (pMsg->wParam == _T('E')) {
//				AfxMessageBox(_T("Ctrl-E pressed"));

				OnBnClickedChangeEditor();
				return TRUE;
			}
		}
		// Check if F2 key is pressed
		else if (pMsg->wParam == VK_F2) {
//			AfxMessageBox(_T("F2 pressed"));

			if (GetDlgItem(IDC_EDIT_SEARCH)->IsWindowEnabled())
				Search(FALSE);
			return TRUE;
		}
		// Check if F3 key is pressed
		else if (pMsg->wParam == VK_F3) {
//			AfxMessageBox(_T("F3 pressed"));

			if (GetDlgItem(IDC_EDIT_SEARCH)->IsWindowEnabled())
				Search();
			return TRUE;
		}
#endif

		// Check if Return key is pressed
		if (pMsg->wParam == VK_RETURN) {
			// Check if focus belongs to edit control
			if (GetFocus() == GetDlgItem(IDC_EDIT_SEARCH)) {
//				AfxMessageBox(_T("Return pressed"));
				Search();
				return TRUE;
			}
			// prevent dialog default button OK (activated on Return) from exiting app
			return TRUE;
		}
	}
	
	return CDialog::PreTranslateMessage(pMsg);
}


#ifdef SYSTEM_WIDE_HOTKEY
void CEDKIIBuildDataViewerDlg::OnHotKey(UINT nHotKeyId, UINT nKey1, UINT nKey2)
{
	if (nHotKeyId == WM_MYMSG_CTRL_V) {
//		AfxMessageBox(_T("Ctrl-V pressed"));

		m_radioView++;
		if (m_radioView >= e_RadioView_MAX)
			m_radioView = 0;
		UpdateData(FALSE);  // set m_radioView value
		OnBnClickedRadioFileData();
		return;
	} else if (nHotKeyId == WM_MYMSG_CTRL_E) {
		OnBnClickedChangeEditor();
		return;
	} else if (nHotKeyId == WM_MYMSG_F2) {
//		AfxMessageBox(_T("F2 pressed"));
		if (GetDlgItem(IDC_EDIT_SEARCH)->IsWindowEnabled())
			Search(FALSE);
		return;
	} else if (nHotKeyId == WM_MYMSG_F3) {
//		AfxMessageBox(_T("F3 pressed"));
		if (GetDlgItem(IDC_EDIT_SEARCH)->IsWindowEnabled())
			Search();
		return;
	}

	CDialog::OnHotKey(nHotKeyId, nKey1, nKey2);
}
#endif
