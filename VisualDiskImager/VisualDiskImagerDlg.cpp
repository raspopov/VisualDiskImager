// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
{
}

void CVisualDiskImagerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogExSized::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BROWSE, m_wndBrowse);
	DDX_Control(pDX, IDC_DEVICES, m_wndDevices);
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	DDX_Control(pDX, IDC_LOG, m_wndLog);
	DDX_Control(pDX, IDOK, m_wndWriteButton);
	DDX_Control(pDX, IDC_REFRESH, m_wndRefresh);
	DDX_Control(pDX, IDC_VERIFY, m_wndVerifyCheckbox);
	DDX_Control(pDX, IDC_VERIFY_BUTTON, m_wndVerifyButton);
}

BEGIN_MESSAGE_MAP(CVisualDiskImagerDlg, CDialogExSized)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_DROPFILES()
	ON_EN_CHANGE( IDC_BROWSE, &CVisualDiskImagerDlg::OnEnChangeBrowse )
	ON_CBN_SELCHANGE( IDC_DEVICES, &CVisualDiskImagerDlg::OnCbnSelchangeDevices )
	ON_BN_CLICKED( IDC_REFRESH, &CVisualDiskImagerDlg::OnBnClickedRefresh )
	ON_WM_SIZE()
	ON_MESSAGE( WM_LOG, &CVisualDiskImagerDlg::OnLog )
	ON_MESSAGE( WM_DONE, &CVisualDiskImagerDlg::OnDone )
	ON_BN_CLICKED(IDC_VERIFY_BUTTON, &CVisualDiskImagerDlg::OnBnClickedVerifyButton)
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
	SHAutoComplete( m_wndBrowse.GetSafeHwnd(), SHACF_FILESYS_ONLY | SHACF_URLHISTORY | SHACF_URLMRU | SHACF_USETAB );

	m_wndProgress.SetRange32( 0, 100 );

	m_wndVerifyCheckbox.SetCheck( theApp.GetProfileInt( REG_SETTINGS, REG_VERIFY, TRUE ) ? BST_CHECKED : BST_UNCHECKED );

	DragAcceptFiles();

	if ( bLoad )
	{
		RestoreWindowPlacement();
	}

	UpdateSize();

	if ( bLoad )
	{
		SetFile( theApp.GetProfileString( REG_SETTINGS, REG_IMAGE ) );
	}

	OnDone( 0, 0 );

	OnBnClickedRefresh();

	return TRUE;  // return TRUE  unless you set the focus to a control
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
	if ( m_Thread.joinable() )
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
			Start( true );
		}
	}
}

void CVisualDiskImagerDlg::OnBnClickedVerifyButton()
{
	if ( m_Thread.joinable() )
	{
		// "Stop" button pressed
		m_bCancel = true;

		m_wndVerifyButton.EnableWindow( FALSE );
	}
	else
	{
		// "Verify" button pressed
		Start( false );
	}
}

void CVisualDiskImagerDlg::Start(bool bWrite)
{
	if ( auto pdevice = GetSelectedDevice() )
	{
		CWaitCursor wc;

		CString strFilename;
		m_wndBrowse.GetWindowText( strFilename );

		ClearLog();

		if ( bWrite )
		{
			m_wndWriteButton.SetWindowText( LoadString( IDS_CANCEL ) );
			m_wndVerifyButton.EnableWindow( FALSE );
		}
		else
		{
			m_wndWriteButton.EnableWindow( FALSE );
			m_wndVerifyButton.SetWindowText( LoadString( IDS_CANCEL ) );
		}
		m_wndRefresh.EnableWindow( FALSE );
		m_wndBrowse.EnableWindow( FALSE );
		m_wndDevices.EnableWindow( FALSE );
		m_wndVerifyCheckbox.EnableWindow( FALSE );

		m_wndProgress.SetPos( 0 );
		m_wndProgress.ShowWindow( SW_SHOW );

		UpdateWindow();

		m_bCancel = false;
		m_Thread = std::thread( &CVisualDiskImagerDlg::WriteDiskThread, this, strFilename, pdevice->Name,
			bWrite, ! bWrite || ( m_wndVerifyCheckbox.GetCheck() == BST_CHECKED ) );
	}
}

void CVisualDiskImagerDlg::OnCancel()
{
	if ( m_Thread.joinable() )
	{
		if ( AfxMessageBox( IDS_CANCEL_ASK, MB_ICONEXCLAMATION | MB_YESNO ) != IDYES )
		{
			return;
		}

		CWaitCursor wc;

		// Wait till write end
		Stop();

		// Retry exit action
		PostMessage( WM_SYSCOMMAND, SC_CLOSE );

		return;
	}

	CDialogExSized::OnCancel();
}

LRESULT CVisualDiskImagerDlg::OnDone(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CWaitCursor wc;

	Stop();

	m_wndWriteButton.SetWindowText( LoadString( IDS_WRITE ) );
	m_wndWriteButton.EnableWindow( TRUE );

	m_wndVerifyButton.SetWindowText( LoadString( IDS_VERIFY ) );
	m_wndVerifyButton.EnableWindow( TRUE );

	m_wndRefresh.EnableWindow( TRUE );
	m_wndBrowse.EnableWindow( TRUE );
	m_wndDevices.EnableWindow( TRUE );
	m_wndVerifyCheckbox.EnableWindow( TRUE );

	m_wndProgress.ShowWindow( SW_HIDE );

	UpdateWindow();

	return 0;
}

void CVisualDiskImagerDlg::Stop()
{
	if ( m_Thread.joinable() )
	{
		m_bCancel = true;

		try
		{
			m_Thread.join();
		}
		catch (...)
		{
		}

		if ( m_Thread.joinable() )
		{
			m_Thread.detach();
		}
	}
}

void CVisualDiskImagerDlg::OnDestroy()
{
	Stop();

	ClearDevices();

	theApp.WriteProfileInt( REG_SETTINGS, REG_VERIFY, ( m_wndVerifyCheckbox.GetCheck() == BST_CHECKED ) ? TRUE : FALSE );


	CDialogExSized::OnDestroy();
}

void CVisualDiskImagerDlg::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szPath[ 1024 ] = {};
	if ( DragQueryFile( hDropInfo, 0, szPath, _countof( szPath ) ) )
	{
		SetFile( szPath );

		DragFinish( hDropInfo );
		return;
	}

	CDialogExSized::OnDropFiles( hDropInfo );
}

void CVisualDiskImagerDlg::SetFile(LPCTSTR szFilename)
{
	CString strFilename;
	m_wndBrowse.GetWindowText( strFilename );
	if ( szFilename == nullptr )
	{
		// Use current one
		szFilename = strFilename;
	}

	if ( strFilename != szFilename )
	{
		m_wndBrowse.SetWindowText( szFilename );

		theApp.WriteProfileString( REG_SETTINGS, REG_IMAGE, szFilename );
	}
}

void CVisualDiskImagerDlg::OnEnChangeBrowse()
{
	SetFile();
}

void CVisualDiskImagerDlg::OnBnClickedRefresh()
{
	CWaitCursor wc;

	ClearLog();

	EnumDevices();
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

void CVisualDiskImagerDlg::UpdateSize()
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
