
// IntelDialogDlg.cpp : implementation file
//

#include "stdafx.h"
#include "IntelDialog.h"
#include "IntelDialogDlg.h"
#include "afxdialogex.h"
#include "..\RealSenceCameraLibrary\RealSenceController.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}



BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CIntelDialogDlg dialog


CIntelDialogDlg::CIntelDialogDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_INTELDIALOG_DIALOG, pParent)
	, m_ShowDepth(false)
	, m_showColour(false)
	, m_showIr(false)
	, m_removeBG(false)
	, m_frameIndex(1)
	, m_numFramesPerSequance(100)
	, m_AutoIncrementFrame(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CIntelDialogDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_CHECK_SHOW_DEPTH, m_ShowDepth);
	DDX_Check(pDX, IDC_CHECK_SHOW_COLOUR, m_showColour);
	DDX_Check(pDX, IDC_CHECK_SHOW_IR, m_showIr);
	DDX_Check(pDX, IDC_CHECK_IGNORE_BACKGROUND, m_removeBG);
	DDX_Check(pDX, IDC_CHECK_AUTO_INC_SAVE, m_AutoIncrementFrame);
	DDX_Text(pDX, IDC_EDIT_FRAME_NUMBER, m_frameIndex);
	DDX_Text(pDX, IDC_EDIT_NUM_FRAMES_SEQ, m_numFramesPerSequance);
}

BEGIN_MESSAGE_MAP(CIntelDialogDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHECK_SHOW_DEPTH, &CIntelDialogDlg::OnClickedCheckShowDepth)
	ON_BN_CLICKED(IDC_CHECK_SHOW_COLOUR, &CIntelDialogDlg::OnClickedCheckShowColour)
	ON_BN_CLICKED(IDC_CHECK_SHOW_IR, &CIntelDialogDlg::OnClickedCheckShowIr)
	ON_BN_CLICKED(IDOK, &CIntelDialogDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_CSV, &CIntelDialogDlg::OnBnClickedButtonSaveCsv)
	ON_BN_CLICKED(IDC_BUTTON_LEARN_BG, &CIntelDialogDlg::OnBnClickedButtonLearnBg)
	ON_BN_CLICKED(IDC_CHECK_IGNORE_BACKGROUND, &CIntelDialogDlg::OnBnClickedCheckIgnoreBackground)
	ON_BN_CLICKED(IDC_BUTTON_BLUE_BG, &CIntelDialogDlg::OnBnClickedButtonBlueBg)
	ON_BN_CLICKED(IDC_BUTTON_CAM_CALIB, &CIntelDialogDlg::OnBnClickedButtonCamCalib)
	ON_BN_CLICKED(IDC_BUTTON_SAVE_SEQ, &CIntelDialogDlg::OnBnClickedButtonSaveSeq)
END_MESSAGE_MAP()

UINT RunInThread(LPVOID pParam)
{
	RealSenceController* camera = (RealSenceController*)pParam;
	camera->RunTillStopped();

	return 0;
}

// CIntelDialogDlg message handlers

BOOL CIntelDialogDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

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

	// TODO: Add extra initialization here
	m_theCamera.reset(new RealSenceController());

	AfxBeginThread(RunInThread, m_theCamera.get());

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CIntelDialogDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CIntelDialogDlg::OnPaint()
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
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CIntelDialogDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIntelDialogDlg::OnClickedCheckShowDepth()
{
	UpdateShow();
}


void CIntelDialogDlg::OnClickedCheckShowColour()
{
	UpdateShow();
}


void CIntelDialogDlg::OnClickedCheckShowIr()
{
	UpdateShow();
}

void CIntelDialogDlg::OnBnClickedCheckIgnoreBackground()
{
	UpdateShow();
}

void CIntelDialogDlg::UpdateShow()
{
	UpdateData(TRUE);
	m_theCamera->SetShowColour(m_showColour != FALSE);
	m_theCamera->SetShowDepth(m_ShowDepth != FALSE);
	m_theCamera->SetShowIR(m_showIr != FALSE);
	m_theCamera->SetRemoveBg(m_removeBG != FALSE);
}


void CIntelDialogDlg::OnBnClickedOk()
{
	m_theCamera->Stop();
	Sleep(1); // to let the thread finish

	CDialogEx::OnOK();
}


void CIntelDialogDlg::OnBnClickedButtonSaveCsv()
{
	UpdateData(TRUE);
	std::string outputFile;
	if (m_AutoIncrementFrame)
	{
		CString name;
		name.Format("D:\\GadiWork\\3DCameras\\FruitCaptures\\frame%02d.png", m_frameIndex);
		outputFile = (LPCSTR)name;
		m_frameIndex++;
	}
	else
	{
		CString sFilter = _T("png Files (*.png)|*.png|csv Files (*.csv)|*.csv|All Files (*.*)|*.*||");
		CFileDialog save(FALSE, _T("png"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, sFilter, this);
		if (IDOK != save.DoModal())
		{
			return;
		}
		outputFile = (LPCSTR)save.GetPathName();
	}

	m_theCamera->SaveNextFrame(outputFile);
	UpdateData(FALSE);
}


void CIntelDialogDlg::OnBnClickedButtonLearnBg()
{
	m_theCamera->LearnBG();
}




void CIntelDialogDlg::OnBnClickedButtonBlueBg()
{
	m_theCamera->UseBlueAsBG();
}


void CIntelDialogDlg::OnBnClickedButtonCamCalib()
{
	CFileDialog save(FALSE, _T("txt"), _T("*.txt"));
	if (IDOK != save.DoModal())
	{
		return;
	}
	std::string outputFile = (LPCSTR)save.GetPathName();
	m_theCamera->SaveCalibration(outputFile);
}




void CIntelDialogDlg::OnBnClickedButtonSaveSeq()
{
	UpdateData(TRUE);
	CFileDialog save(FALSE, _T("twx"), _T("*.twx"));
	if (IDOK != save.DoModal())
	{
		return;
	}
	std::string xmlFile = (LPCSTR)save.GetPathName();
	m_theCamera->SaveSequance(xmlFile, m_numFramesPerSequance);
}

