
// VisualDiskImagerDlg.h : header file
//

#pragma once

#include "Device.h"
#include "DialogExSized.h"

// CVisualDiskImagerDlg dialog

class CVisualDiskImagerDlg : public CDialogExSized
{
public:
	CVisualDiskImagerDlg(CWnd* pParent = nullptr);

	enum { IDD = IDD_VISUALDISKIMAGER_DIALOG };

protected:
	HICON				m_hIcon;
	CImageList			m_Images;

	CButton				m_wndWriteButton;	// "Write"/"Stop" button
	CButton				m_wndVerifyButton;	// "Verify"/"Stop" button
	CButton				m_wndRefresh;		// "Refresh" button
	CMFCEditBrowseCtrl	m_wndBrowse;
	CComboBox			m_wndDevices;		// Enumerated devices list
	CButton				m_wndVerifyCheckbox;
	CProgressCtrl		m_wndProgress;
	CListCtrl			m_wndLog;

	std::thread			m_Thread;
	volatile bool		m_bCancel;
	CDevices			m_Devices;

	void Start(bool bWrite);
	void Stop();

	void UpdateSize();

	// Clear enumerated devices list (i.e. m_wndDevices)
	void ClearDevices();

	// Set disk image file
	void SetFile(LPCTSTR szFilename = nullptr);

	// Enumerate disk volumes
	void EnumDevices();

	// Return selected device
	CDevice* GetSelectedDevice() const;

	static void WriteDiskThread(CVisualDiskImagerDlg* pThis, LPCTSTR szFilename, LPCTSTR szDevice, bool bWrite, bool bVerify);
	void WriteDisk(LPCTSTR szFilename, LPCTSTR szDevice, bool bWrite, bool bVerify);

	// Clear the log (with animation)
	void ClearLog();

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnEnChangeBrowse();
	afx_msg void OnBnClickedRefresh();
	afx_msg void OnCbnSelchangeDevices();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedVerifyButton();

	DECLARE_MESSAGE_MAP()
};

#define WM_LOG		( WM_USER + 1 )	// Log window message
#define WM_DONE		( WM_USER + 2 )	// End of task message
