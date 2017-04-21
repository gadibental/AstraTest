
// IntelDialogDlg.h : header file
//

#pragma once

#include <memory>

class RealSenceController;

// CIntelDialogDlg dialog
class CIntelDialogDlg : public CDialogEx
{
// Construction
public:
	CIntelDialogDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_INTELDIALOG_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnClickedCheckShowDepth();
	afx_msg void OnClickedCheckShowColour();
	afx_msg void OnClickedCheckShowIr();

private:
	void UpdateShow();


private:
	BOOL m_ShowDepth;
	BOOL m_showColour;
	BOOL m_showIr;
	BOOL m_removeBG;

	std::shared_ptr<RealSenceController> m_theCamera;
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButtonSaveCsv();
	afx_msg void OnBnClickedButtonLearnBg();
	afx_msg void OnBnClickedCheckIgnoreBackground();
	afx_msg void OnBnClickedButtonBlueBg();
	afx_msg void OnBnClickedButtonCamCalib();
};
