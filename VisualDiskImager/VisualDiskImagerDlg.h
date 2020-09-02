
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

	CButton				m_wndStart;		// "Write"/"Cancel" button
	CButton				m_wndRefresh;	// "Refresh" button
	CMFCEditBrowseCtrl	m_wndBrowse;
	CComboBox			m_wndDevices;	// Enumerated devices list
	CProgressCtrl		m_wndProgress;
	CListCtrl			m_wndLog;
	std::thread			m_Thread;
	volatile bool		m_bCancel;

	std::deque< std::unique_ptr< CDevice > > m_Devices;

	void Cancel();

	void UpdateSize();

	// Clear enumerated devices list (i.e. m_wndDevices)
	void ClearDevices();

	// Set disk image file
	void SetFile(LPCTSTR szFilename = nullptr);

	// Enumerate disk volumes (bUseDefault = true - load default from registry)
	void EnumDevices(bool bUseDefault);

	// Return selected device
	CDevice* GetSelectedDevice() const;

	static void WriteDiskThread(CVisualDiskImagerDlg* pThis, LPCTSTR szFilename, CDevice* pdevice);
	void WriteDisk(LPCTSTR szFilename, CDevice* pdevice);

	void UnmountDisk(LPCTSTR szDeviceID);

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

	DECLARE_MESSAGE_MAP()
};

#define WM_LOG		( WM_USER + 1 )	// Log window message
#define WM_DONE		( WM_USER + 2 )	// End of task message
