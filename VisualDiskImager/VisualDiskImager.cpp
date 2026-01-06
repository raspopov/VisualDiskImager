// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "pch.h"
#include "VisualDiskImager.h"
#include "VisualDiskImagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void Log(LogPriority nPriority, LPCTSTR szFormat, ...)
{
	va_list ap;
	va_start( ap, szFormat ); //-V2018 //-V2019
	LogV( nPriority, szFormat, ap );
	va_end( ap );
}

void Log(LogPriority nPriority, UINT nFormat, ...)
{
	va_list ap;
	va_start( ap, nFormat ); //-V2018 //-V2019
	LogV( nPriority, LoadString( nFormat ), ap );
	va_end( ap );
}

void LogV(LogPriority nPriority, LPCTSTR szFormat, va_list ap)
{
	const auto now = CTime::GetCurrentTime();

	auto plog = std::make_unique< CString >( now.Format( _T("%T : ") ) );
	plog->AppendFormatV( szFormat, ap );

	TRACE( _T("%s\n"), plog->operator const WCHAR *() );

	if ( CWnd* pWnd = AfxGetMainWnd() )
	{
		pWnd->SendMessage( WM_LOG, static_cast< UINT >( nPriority ), reinterpret_cast< LPARAM >( plog.release() ) );
	}
	else if ( theApp.m_pMainWnd )
	{
		theApp.m_pMainWnd->PostMessage( WM_LOG, static_cast< UINT >( nPriority ), reinterpret_cast< LPARAM >( plog.release() ) );
	}
}

CString LoadString(UINT nID)
{
	CString str;
	VERIFY( str.LoadString( nID ) );
	return str;
}

CString FormatByteSize(LONGLONG nSize)
{
	CString str;
	StrFormatByteSize( nSize, str.GetBuffer( 32 ), 32 );
	str.ReleaseBuffer();
	return str;
}

CString FormatByteSizeEx(LONGLONG nSize)
{
	CString str = FormatByteSize( nSize );
	if ( nSize >= 1024 )
	{
		str.AppendFormat( _T(" (%I64u %s)"), nSize, static_cast< LPCTSTR >( FormatByteSize( 0 ).Trim( _T(" 0") ) ) );
	}
	return str;
}

CString GetErrorString(DWORD error)
{
	CString str;
	str.Format( _T("0x%08x "), error );

	LPTSTR errormessage = nullptr;
	if ( FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER, nullptr, error, 0, reinterpret_cast< LPTSTR >( &errormessage ), 0, nullptr ) )
	{
		str += errormessage;
		str.TrimRight( _T("\r\n") );
		LocalFree( errormessage );
	}

	return str;
}

// CVisualDiskImagerApp

BEGIN_MESSAGE_MAP(CVisualDiskImagerApp, CWinApp)
END_MESSAGE_MAP()

// The one and only CVisualDiskImagerApp object

CVisualDiskImagerApp theApp;

// CVisualDiskImagerApp initialization

BOOL CVisualDiskImagerApp::InitInstance()
{
	const INITCOMMONCONTROLSEX InitCtrls = { sizeof( InitCtrls ), ICC_WIN95_CLASSES | ICC_STANDARD_CLASSES | ICC_USEREX_CLASSES };
	InitCommonControlsEx( &InitCtrls );

	VERIFY( SUCCEEDED( CoInitializeEx( nullptr, COINIT_APARTMENTTHREADED ) ) );

	VERIFY( SUCCEEDED( CoInitializeSecurity( nullptr, -1, nullptr, nullptr, RPC_C_AUTHN_LEVEL_DEFAULT, RPC_C_IMP_LEVEL_IMPERSONATE,
		nullptr, EOAC_NONE, nullptr ) ) );

	EnableHtmlHelp();

	SetAppID( AfxGetAppName() );

	SetRegistryKey( AFX_IDS_COMPANY_NAME );

	CWinApp::InitInstance();

	EnableTaskbarInteraction();

	// Activate "Windows Native" visual manager for enabling themes in MFC controls
	CMFCVisualManager::SetDefaultManager( RUNTIME_CLASS( CMFCVisualManagerWindows ) ); //-V2018

	// Disable system error pop-ups
	SetErrorMode( SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX );

	CCommandLineInfo cmdInfo;
	ParseCommandLine( cmdInfo );
	if ( ! cmdInfo.m_strFileName.IsEmpty() )
	{
		theApp.WriteProfileString( REG_SETTINGS, REG_IMAGE, cmdInfo.m_strFileName );
	}

	CVisualDiskImagerDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
