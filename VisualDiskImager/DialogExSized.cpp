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
#include "DialogExSized.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDialogExSized dialog

IMPLEMENT_DYNAMIC(CDialogExSized, CDialogEx)

BEGIN_MESSAGE_MAP(CDialogExSized, CDialogEx)
	ON_WM_GETMINMAXINFO()
	ON_WM_DESTROY()
END_MESSAGE_MAP()

BOOL CDialogExSized::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GetWindowRect( m_rcInitial );
	GetClientRect( m_rcInitialClient );

	if ( m_pDynamicLayout )
		m_pDynamicLayout->SetMinSize( m_rcInitialClient.Size() );

	return TRUE;
}

void CDialogExSized::ReloadLayout()
{
	ASSERT( ! IsIconic() );

	VERIFY( LoadDynamicLayoutResource( m_lpszTemplateName ) );

	if ( m_pDynamicLayout )
		m_pDynamicLayout->SetMinSize( m_rcInitialClient.Size() );
}

void CDialogExSized::SaveWindowPlacement()
{
	CString sClassName( GetRuntimeClass()->m_lpszClassName );
	ASSERT( sClassName != ("CDialogExSized") ); // Use DECLARE_DYNAMIC() in the child class.

	WINDOWPLACEMENT wp = { sizeof( WINDOWPLACEMENT ) };
	if ( GetWindowPlacement( &wp ) )
	{
		AfxGetApp()->WriteProfileBinary( _T("Window"), sClassName + _T("_position"), (LPBYTE)&wp, sizeof( WINDOWPLACEMENT ) );
	}
}

void CDialogExSized::RestoreWindowPlacement()
{
	CString sClassName( GetRuntimeClass()->m_lpszClassName );
	ASSERT( sClassName != ("CDialogExSized") ); // Use DECLARE_DYNAMIC() in the child class.

	CAutoVectorPtr< WINDOWPLACEMENT >wp;
	UINT wp_size = 0;
	if ( AfxGetApp()->GetProfileBinary( _T("Window"), sClassName + _T("_position"), (LPBYTE*)&wp, &wp_size ) &&
			wp_size == sizeof( WINDOWPLACEMENT ) )
	{
		SetWindowPlacement( wp );
	}
}

// CDialogExSized message handlers

void CDialogExSized::OnGetMinMaxInfo( MINMAXINFO* lpMMI )
{
	lpMMI->ptMaxTrackSize.x = GetSystemMetrics( SM_CXMAXIMIZED );
	lpMMI->ptMaxTrackSize.y = GetSystemMetrics( SM_CYMAXIMIZED );
	lpMMI->ptMinTrackSize.x = min( m_rcInitial.Width(),  lpMMI->ptMaxTrackSize.x );
	lpMMI->ptMinTrackSize.y = min( m_rcInitial.Height(), lpMMI->ptMaxTrackSize.y );

	CDialogEx::OnGetMinMaxInfo( lpMMI );
}

void CDialogExSized::OnDestroy()
{
	SaveWindowPlacement();

	CDialogEx::OnDestroy();
}
