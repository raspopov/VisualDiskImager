// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
This file is part of Visual Disk Imager

Copyright (C) 2020 Nikolay Raspopov <raspopov@cherubicsoft.com>

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

#include "pch.h"
#include "VisualDiskImager.h"
#include "VisualDiskImagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CVisualDiskImagerDlg dialog

IMPLEMENT_DYNAMIC(CVisualDiskImagerDlg, CDialogExSized)

CVisualDiskImagerDlg::CVisualDiskImagerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogExSized( IDD, pParent )
	, m_hIcon		( nullptr )
	, m_bCancel		( false )
	, m_nProgress	( -1 )
	, m_Mode		( MODE_STOP )
{
}

void CVisualDiskImagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogExSized::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BROWSE, m_wndBrowse);
	DDX_Control(pDX, IDC_DEVICES, m_wndDevices);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_LOG, m_wndLog);
	DDX_Control(pDX, IDC_WRITE_BUTTON, m_wndWriteButton);
	DDX_Control(pDX, IDC_REFRESH_BUTTON, m_wndRefreshButton);
	DDX_Control(pDX, IDC_VERIFY_CHECK, m_wndVerifyCheckbox);
	DDX_Control(pDX, IDC_VERIFY_BUTTON, m_wndVerifyButton);
}

BEGIN_MESSAGE_MAP(CVisualDiskImagerDlg, CDialogExSized)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_EN_CHANGE( IDC_BROWSE, &CVisualDiskImagerDlg::OnEnChangeBrowse )
	ON_CBN_SELCHANGE( IDC_DEVICES, &CVisualDiskImagerDlg::OnCbnSelchangeDevices )
	ON_BN_CLICKED( IDC_REFRESH_BUTTON, &CVisualDiskImagerDlg::OnBnClickedRefreshButton )
	ON_BN_CLICKED( IDC_WRITE_BUTTON, &CVisualDiskImagerDlg::OnBnClickedWriteButton )
	ON_BN_CLICKED( IDC_VERIFY_BUTTON, &CVisualDiskImagerDlg::OnBnClickedVerifyButton )
	ON_BN_CLICKED( IDC_EXIT_BUTTON, &CVisualDiskImagerDlg::OnBnClickedExitButton )
	ON_WM_SIZE()
	ON_MESSAGE( WM_LOG, &CVisualDiskImagerDlg::OnLog )
	ON_MESSAGE( WM_DONE, &CVisualDiskImagerDlg::OnDone )
	ON_MESSAGE( WM_ENUM, &CVisualDiskImagerDlg::OnEnum )
	ON_MESSAGE( WM_DEVICECHANGE, &CVisualDiskImagerDlg::OnDeviceChange )
	ON_NOTIFY( LVN_KEYDOWN, IDC_LOG, &CVisualDiskImagerDlg::OnLvnKeydownLog )
	ON_NOTIFY( NM_RCLICK, IDC_LOG, &CVisualDiskImagerDlg::OnNMRClickLog )
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CVisualDiskImagerDlg message handlers

BOOL CVisualDiskImagerDlg::OnInitDialog()
{
	CDialogExSized::OnInitDialog();

	CString sBinaryPath;
	GetModuleFileName( nullptr, sBinaryPath.GetBuffer( 1024 ), 1024 );
	sBinaryPath.ReleaseBuffer();

	CString sVersion;
	if ( DWORD dwSize = GetFileVersionInfoSize( sBinaryPath, &dwSize ) )
	{
		CAutoVectorPtr< BYTE > pBuffer( new BYTE[ dwSize ] );
		if ( pBuffer )
		{
			if ( GetFileVersionInfo( sBinaryPath, 0, dwSize, pBuffer ) )
			{
				VS_FIXEDFILEINFO* pTable = nullptr;
				if ( VerQueryValue( pBuffer, _T("\\"), (VOID**)&pTable, (UINT*)&dwSize ) )
				{
					sVersion.Format( _T("%u.%u.%u.%u"),
						(WORD)( pTable->dwFileVersionMS >> 16 ), (WORD)( pTable->dwFileVersionMS & 0xFFFF ),
						(WORD)( pTable->dwFileVersionLS >> 16 ), (WORD)( pTable->dwFileVersionLS & 0xFFFF ) );
				}
			}
		}
	}

	const bool bLoad = ( GetKeyState( VK_SHIFT ) >= 0 );

	m_hIcon = theApp.LoadIcon( IDR_MAINFRAME );

	SetWindowText( LoadString( AFX_IDS_APP_TITLE ) + _T(" ") + sVersion );

	SetIcon( m_hIcon, TRUE );		// Set big icon
	SetIcon( m_hIcon, FALSE );		// Set small icon

	const UINT icons[] = { IDI_ICON_ERROR, IDI_ICON_INFO, IDI_ICON_WARNING, IDI_ICON_ACTION };
	if ( ! m_Images.Create( 16, 16, ILC_COLOR32 | ILC_MASK, _countof( icons ), 0 ) )
	{
		m_Images.Create( 16, 16, ILC_COLOR | ILC_MASK, _countof( icons ), 0 );
	}
	for ( auto i : icons )
	{
		m_Images.Add( (HICON)LoadImage( AfxGetResourceHandle(), MAKEINTRESOURCE( i ), IMAGE_ICON, 16, 16, LR_SHARED ) );
	}

	m_wndLog.SetImageList( &m_Images, LVSIL_SMALL );
	m_wndLog.SetExtendedStyle( m_wndLog.GetExtendedStyle() | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_LABELTIP | LVS_EX_FULLROWSELECT );
	m_wndLog.InsertColumn( 0, _T(""), LVCFMT_LEFT );

	m_wndDevices.SetCueBanner( LoadString( IDS_DEVICE_SELECT ) );

	m_wndBrowse.EnableFileBrowseButton( nullptr, LoadString( IDS_FILE_FILTER ), OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST );
	m_wndBrowse.SetCueBanner( LoadString( IDS_FILE_SELECT )  );
	m_wndBrowse.SetWindowText( theApp.GetProfileString( REG_SETTINGS, REG_IMAGE ) );
	SHAutoComplete( m_wndBrowse.GetSafeHwnd(), SHACF_FILESYS_ONLY | SHACF_URLHISTORY | SHACF_URLMRU | SHACF_USETAB );

	m_wndProgress.SetRange32( 0, 100 );

	m_wndVerifyCheckbox.SetCheck( theApp.GetProfileInt( REG_SETTINGS, REG_VERIFY, TRUE ) ? BST_CHECKED : BST_UNCHECKED );

	DragAcceptFiles();

	if ( bLoad )
	{
		RestoreWindowPlacement();
	}

	UpdateSize();

	SetTimer( 1, 500, nullptr );

	PostMessage( WM_DONE );

	PostMessage( WM_ENUM );

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVisualDiskImagerDlg::OnDestroy()
{
	Stop();

	ClearDevices();

	CString strFilename;
	m_wndBrowse.GetWindowText( strFilename );
	theApp.WriteProfileString( REG_SETTINGS, REG_IMAGE, strFilename );

	theApp.WriteProfileInt( REG_SETTINGS, REG_VERIFY, ( m_wndVerifyCheckbox.GetCheck() == BST_CHECKED ) ? TRUE : FALSE );

	KillTimer( 1 );

	CDialogExSized::OnDestroy();
}

void CVisualDiskImagerDlg::OnPaint()
{
	if ( IsIconic ())
	{
		CPaintDC dc( this ); // device context for painting

		SendMessage( WM_ICONERASEBKGND, reinterpret_cast< WPARAM >( dc.GetSafeHdc() ), 0 );

		// Center icon in client rectangle
		CRect rect;
		GetClientRect( &rect );

		// Draw the icon
		dc.DrawIcon( ( rect.Width() - GetSystemMetrics( SM_CXICON ) + 1 ) / 2,
			( rect.Height() - GetSystemMetrics( SM_CYICON ) + 1 ) / 2, m_hIcon );
	}
	else
	{
		CDialogExSized::OnPaint();
	}
}

HCURSOR CVisualDiskImagerDlg::OnQueryDragIcon()
{
	return static_cast< HCURSOR >( m_hIcon );
}

void CVisualDiskImagerDlg::OnOK()
{
	// "ENTER" pressed

	OnBnClickedWriteButton();
}

void CVisualDiskImagerDlg::OnCancel()
{
	// "Esc" pressed

	if ( IsStarted() )
	{
		if ( m_Mode == MODE_WRITE || m_Mode == MODE_WRITE_VERIFY )
		{
			if ( AfxMessageBox( IDS_CANCEL_ASK, MB_ICONEXCLAMATION | MB_YESNO ) != IDYES )
			{
				return;
			}
		}

		CWaitCursor wc;

		// Wait till write end
		Stop();

		return;
	}

	CDialogExSized::OnCancel();
}

void CVisualDiskImagerDlg::OnBnClickedRefreshButton()
{
	ClearLog();

	PostMessage( WM_ENUM );
}

void CVisualDiskImagerDlg::OnBnClickedExitButton()
{
	OnCancel();
}

void CVisualDiskImagerDlg::OnBnClickedWriteButton()
{
	if ( IsStarted() )
	{
		// "Stop" button pressed
		if ( AfxMessageBox( IDS_CANCEL_ASK, MB_ICONEXCLAMATION | MB_YESNO ) == IDYES )
		{
			m_bCancel = true;

			m_wndWriteButton.EnableWindow( FALSE );
		}
	}
	else
	{
		// "Write" button pressed
		if ( AfxMessageBox( IDS_WRITE_PROMPT, MB_ICONEXCLAMATION | MB_YESNO ) == IDYES )
		{
			Start( ( m_wndVerifyCheckbox.GetCheck() == BST_CHECKED ) ? MODE_WRITE_VERIFY : MODE_WRITE );
		}
	}
}

void CVisualDiskImagerDlg::OnBnClickedVerifyButton()
{
	if ( IsStarted() )
	{
		// "Stop" button pressed
		m_bCancel = true;

		m_wndVerifyButton.EnableWindow( FALSE );
	}
	else
	{
		// "Verify" button pressed
		Start( MODE_VERIFY );
	}
}

void CVisualDiskImagerDlg::Start(Mode mode)
{
	if ( auto pdevice = GetSelectedDevice() )
	{
		CWaitCursor wc;

		CString strFilename;
		m_wndBrowse.GetWindowText( strFilename );

		ClearLog();

		switch ( m_Mode = mode )
		{
		case MODE_WRITE:
		case MODE_WRITE_VERIFY:
			m_wndWriteButton.SetWindowText( LoadString( IDS_CANCEL ) );
			m_wndVerifyButton.EnableWindow( FALSE );
			break;

		case MODE_VERIFY:
			m_wndWriteButton.EnableWindow( FALSE );
			m_wndVerifyButton.SetWindowText( LoadString( IDS_CANCEL ) );
			break;
		}

		m_wndRefreshButton.EnableWindow( FALSE );
		m_wndBrowse.EnableWindow( FALSE );
		m_wndDevices.EnableWindow( FALSE );
		m_wndVerifyCheckbox.EnableWindow( FALSE );

		m_nProgress = 0;
		m_wndProgress.ShowWindow( SW_SHOW );

		UpdateWindow();

		m_bCancel = false;
		m_Thread = std::thread( &CVisualDiskImagerDlg::WriteDiskThread, this, strFilename, pdevice->Name );
	}
}

LRESULT CVisualDiskImagerDlg::OnDone(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	Stop();

	m_wndWriteButton.SetWindowText( LoadString( IDS_WRITE ) );
	m_wndWriteButton.EnableWindow( TRUE );

	m_wndVerifyButton.SetWindowText( LoadString( IDS_VERIFY ) );
	m_wndVerifyButton.EnableWindow( TRUE );

	m_wndRefreshButton.EnableWindow( TRUE );
	m_wndBrowse.EnableWindow( TRUE );
	m_wndDevices.EnableWindow( TRUE );
	m_wndVerifyCheckbox.EnableWindow( TRUE );

	m_nProgress = -1;
	m_wndProgress.ShowWindow( SW_HIDE );

	UpdateWindow();

	return 0;
}

LRESULT CVisualDiskImagerDlg::OnEnum(WPARAM wParam, LPARAM /*lParam*/)
{
	CWaitCursor wc;

	EnumDevices( wParam );

	return 0;
}

void CVisualDiskImagerDlg::Stop()
{
	if ( IsStarted() )
	{
		m_bCancel = true;

		try
		{
			m_Thread.join();
		}
		catch (...)
		{
		}

		if ( IsStarted() )
		{
			m_Thread.detach();
		}
	}

	m_Mode = MODE_STOP;
}


void CVisualDiskImagerDlg::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szPath[ 1024 ] = {};
	if ( DragQueryFile( hDropInfo, 0, szPath, _countof( szPath ) ) )
	{
		m_wndBrowse.SetWindowText( CString( szPath ).Trim( _T(" \"") ) );

		DragFinish( hDropInfo );
		return;
	}

	CDialogExSized::OnDropFiles( hDropInfo );
}

void CVisualDiskImagerDlg::OnEnChangeBrowse()
{
}

void CVisualDiskImagerDlg::ClearDevices()
{
	m_Devices.clear();

	m_wndDevices.ResetContent();
}

CDevice* CVisualDiskImagerDlg::GetSelectedDevice() const
{
	const int nIndex = m_wndDevices.GetCurSel();
	if ( nIndex != CB_ERR )
	{
		return reinterpret_cast< CDevice* >( m_wndDevices.GetItemDataPtr( nIndex ) );
	}

	return nullptr;
}

void CVisualDiskImagerDlg::OnCbnSelchangeDevices()
{
	auto pdevice = GetSelectedDevice();
	if ( pdevice )
	{
		theApp.WriteProfileString( REG_SETTINGS, REG_DEVICE, pdevice->Name );
	}
}

void CVisualDiskImagerDlg::ClearLog()
{
	m_wndLog.DeleteAllItems();

	UpdateWindow();

	Sleep( 250 );
}

LRESULT CVisualDiskImagerDlg::OnLog(WPARAM wParam, LPARAM lParam)
{
	CAutoPtr< CString > plog( reinterpret_cast< CString* >( lParam ) );

	const int index = m_wndLog.InsertItem( m_wndLog.GetItemCount(), *plog, static_cast< int >( wParam ) );
	m_wndLog.EnsureVisible( index, FALSE );
	m_wndLog.UpdateWindow();

	return 0;
}

void CVisualDiskImagerDlg::UpdateSize() noexcept
{
	CRect rc;
	m_wndLog.GetWindowRect( &rc );
	m_wndLog.SetColumnWidth( 0, rc.Width() - 8 - GetSystemMetrics( SM_CXVSCROLL ) - 2 * GetSystemMetrics( SM_CXBORDER ) );
}

void CVisualDiskImagerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogExSized::OnSize( nType, cx, cy );

	if ( m_wndLog.m_hWnd )
	{
		UpdateSize();
	}
}

LRESULT CVisualDiskImagerDlg::OnDeviceChange(WPARAM wParam, LPARAM lParam)
{
	// Asynchronous call from WMI
	if ( wParam == DBT_DEVNODES_CHANGED )
	{
		if ( ! IsStarted() )
		{
			PostMessage( WM_ENUM, TRUE );
		}
	}
	return TRUE;
}

void CVisualDiskImagerDlg::SelectLogAll()
{
	m_wndLog.SetItemState( -1, LVIS_SELECTED, LVIS_SELECTED );
	m_wndLog.SetSelectionMark( 0 );
}

void CVisualDiskImagerDlg::CopyLog()
{
	CString sData;

	int nItem = -1;
	for ( int nSelected = m_wndLog.GetSelectedCount(); nSelected > 0; --nSelected )
	{
		nItem = m_wndLog.GetNextItem( nItem, LVNI_SELECTED );
		if ( nItem == -1 )
			break;

		sData += m_wndLog.GetItemText( nItem, 0 );
		sData += _T("\r\n");
	}

	if ( ! sData.IsEmpty() )
	{
		if ( OpenClipboard() )
		{
			if ( EmptyClipboard() )
			{
				const size_t nLen = static_cast< size_t >( sData.GetLength() + 1 ) * sizeof( TCHAR );
				if ( HGLOBAL hGlob = GlobalAlloc( GMEM_FIXED, nLen ) )
				{
					CopyMemory( hGlob, static_cast< LPCTSTR >( sData ), nLen );

					if ( SetClipboardData( CF_UNICODETEXT, hGlob ) == NULL )
					{
						// Œ¯Ë·Í‡
						GlobalFree( hGlob );
					}
				}
			}
			CloseClipboard();
		}
	}
}

void CVisualDiskImagerDlg::OnLvnKeydownLog(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLVKEYDOWN pLVKeyDow = reinterpret_cast< LPNMLVKEYDOWN >( pNMHDR );
	*pResult = 0;

	if ( GetKeyState( VK_CONTROL ) < 0 )
	{
		switch ( pLVKeyDow->wVKey )
		{
		case 'C':		// Ctrl+C
		case 'X':		// Ctrl+X
		case VK_INSERT:	// Ctrl+Insert
			CopyLog();
			break;

		case 'A':		// Ctrl+A
			SelectLogAll();
			break;
		}
	}

	if ( GetKeyState( VK_SHIFT ) < 0 )
	{
		switch ( pLVKeyDow->wVKey )
		{
		case VK_INSERT:	// Shift+Insert
			CopyLog();
			break;
		}
	}
}

void CVisualDiskImagerDlg::OnNMRClickLog(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>( pNMHDR );
	*pResult = 0;

	m_wndLog.SetFocus();
	m_wndLog.SetItemState( pNMItemActivate->iItem, LVIS_SELECTED, LVIS_SELECTED );

	CPoint cursor;
	GetCursorPos( &cursor );

	CMenu menu;
	if ( menu.CreatePopupMenu() )
	{
		if ( menu.AppendMenu( MF_ENABLED | MF_STRING, 1, LoadString( IDS_COPY ) ) )
		{
			if ( menu.TrackPopupMenu( TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_RIGHTBUTTON, cursor.x, cursor.y, this ) == 1 )
			{
				CopyLog();
			}
		}
	}
}

void CVisualDiskImagerDlg::OnTimer(UINT_PTR nIDEvent)
{
	const int nProgress =  m_nProgress;
	if ( nProgress >= 0 )
	{
		m_wndProgress.SetPos( nProgress );
	}

	CDialogExSized::OnTimer(nIDEvent);
}
