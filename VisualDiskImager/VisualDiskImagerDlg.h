/*
This file is part of Visual Disk Imager

Copyright (C) 2020-2025 Nikolay Raspopov <raspopov@cherubicsoft.com>

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see < http://www.gnu.org/licenses/>.
*/

#pragma once

#include "Device.h"
#include "DialogExSized.h"

// Process modes
enum Mode { MODE_STOP = 0, MODE_WRITE, MODE_WRITE_VERIFY, MODE_VERIFY, MODE_READ };

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
	HICON				m_hIcon = nullptr;
	HACCEL				m_hAccels = nullptr;
	CImageList			m_Images;
	CToolTipCtrl		m_pToolTip;
	CButton				m_wndWriteButton;	// "Write"/"Stop" button
	CButton				m_wndVerifyButton;	// "Verify"/"Stop" button
	CButton				m_wndReadButton;	// "Read"/"Stop" button
	CButton				m_wndRefreshButton;	// "Refresh" button
	CMFCEditBrowseCtrl	m_wndBrowse;
	CComboBoxEx			m_wndDevices;		// Enumerated devices list
	CButton				m_wndVerifyCheckbox;
	CProgressCtrl		m_wndProgress;
	CListCtrl			m_wndLog;
	CString				m_Filename;
	ULONGLONG			m_Offset = 0;
	ULONGLONG			m_Size = 0;

	std::thread			m_Thread;
	volatile bool		m_bCancel = false;
	CDevices			Devices;
	int					m_nProgress = -1;	// 0-100% or -1
	Mode				m_Mode = MODE_STOP;

	void UpdateInterface();

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

	// Return selected device/volume
	const CItem * GetSelected() const;

	static void DeviceOperationThread(CVisualDiskImagerDlg* pThis, Mode mode, CString sFilename, CString sDevice, ULONGLONG offset, ULONGLONG size);
	void DeviceOperation(Mode mode, CString sFilename, CString sDevice, ULONGLONG offset, ULONGLONG size);
	bool DeviceOperationWrite(CDevice & device, CAtlFile & file, ULONGLONG offset, ULONGLONG size);
	bool DeviceOperationRead(CDevice & device, CAtlFile & file, ULONGLONG offset, ULONGLONG size, bool isVerify);

	// Clear the log (with animation)
	void ClearLog();

	// Copy log to clipboard
	void CopyLog();

	// Select all lines of log
	void SelectLogAll();

	inline void Progress(int progress) noexcept
	{
		m_nProgress = progress;
	}

	inline void Cancel(bool cancel = true) noexcept
	{
		m_bCancel = cancel;
	}

	inline bool isCancelled() const noexcept
	{
		return m_bCancel;
	}

	void DoDataExchange(CDataExchange* pDX) override;
	BOOL OnInitDialog() override;
	BOOL PreTranslateMessage(MSG* pMsg) override;
	void OnOK() override;
	void OnCancel() override;

	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnEnChangeBrowse();
	afx_msg void OnBnClickedBrowseButton();
	afx_msg void OnBnClickedRefreshButton();
	afx_msg void OnBnClickedWriteButton();
	afx_msg void OnBnClickedVerifyButton();
	afx_msg void OnBnClickedReadButton();
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
