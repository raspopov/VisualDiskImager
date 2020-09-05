
// VisualDiskImagerDlg.h : header file
//

#pragma once

#include "Device.h"
#include "DialogExSized.h"

// Process modes
enum Mode { MODE_STOP = 0, MODE_WRITE, MODE_WRITE_VERIFY, MODE_VERIFY };

#define WM_LOG		( WM_USER + 1 )	// Log window message
#define WM_DONE		( WM_USER + 2 )	// End of task message
#define WM_ENUM		( WM_USER + 3 )	// Enumerate devices message

// CVisualDiskImagerDlg dialog

class CVisualDiskImagerDlg : public CDialogExSized
{
	DECLARE_DYNAMIC(CVisualDiskImagerDlg)

public:
	CVisualDiskImagerDlg(CWnd* pParent = nullptr);

	enum { IDD = IDD_VISUALDISKIMAGER_DIALOG };

protected:
	HICON				m_hIcon;
	CImageList			m_Images;

	CButton				m_wndWriteButton;	// "Write"/"Stop" button
	CButton				m_wndVerifyButton;	// "Verify"/"Stop" button
	CButton				m_wndRefreshButton;		// "Refresh" button
	CMFCEditBrowseCtrl	m_wndBrowse;
	CComboBox			m_wndDevices;		// Enumerated devices list
	CButton				m_wndVerifyCheckbox;
	CProgressCtrl		m_wndProgress;
	CListCtrl			m_wndLog;

	std::thread			m_Thread;
	volatile bool		m_bCancel;
	CDevices			m_Devices;
	int					m_nProgress;		// 0-100% or -1
	Mode				m_Mode;

	// Start write/verify process
	void Start(Mode mode);

	// Stop process
	void Stop();

	// Is process started?
	inline bool IsStarted() const noexcept
	{
		 return m_Thread.joinable();
	}

	void UpdateSize() noexcept;

	// Clear enumerated devices list (i.e. m_wndDevices)
	void ClearDevices();

	// Enumerate disk volumes
	void EnumDevices(bool bSilent);

	// Return selected device
	CDevice* GetSelectedDevice() const;

	static void WriteDiskThread(CVisualDiskImagerDlg* pThis, LPCTSTR szFilename, LPCTSTR szDevice);
	void WriteDisk(LPCTSTR szFilename, LPCTSTR szDevice);

	// Clear the log (with animation)
	void ClearLog();

	// Copy log to clipboard
	void CopyLog();

	// Select all lines of log
	void SelectLogAll();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnEnChangeBrowse();
	afx_msg void OnBnClickedRefreshButton();
	afx_msg void OnBnClickedWriteButton();
	afx_msg void OnBnClickedVerifyButton();
	afx_msg void OnBnClickedExitButton();
	afx_msg void OnCbnSelchangeDevices();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnLog(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDone(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnEnum(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDeviceChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLvnKeydownLog(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMRClickLog(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer(UINT_PTR nIDEvent);

	DECLARE_MESSAGE_MAP()
};
