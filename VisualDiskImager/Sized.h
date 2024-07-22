/*
This file is part of Visual Disk Imager

Copyright (C) 2020-2024 Nikolay Raspopov <raspopov@cherubicsoft.com>

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

template< class T >
class CSized
{
public:
	void InitialLayout()
	{
		auto pThis = static_cast< T* >( this );

		if ( m_sizeMin == CSize( 0, 0 ) )
		{
			CRect rectWindow;
			pThis->GetWindowRect( rectWindow );
			m_sizeMin = rectWindow.Size();

			CRect rectClient;
			pThis->GetClientRect( rectClient );
			m_sizeMinClient = rectClient.Size();

			if ( auto pLayout = pThis->GetDynamicLayout() )
			{
				pLayout->SetMinSize( m_sizeMinClient );
			}
		}
	}

	virtual void SaveWindowPlacement()
	{
		auto pThis = static_cast< T* >( this );

		CWnd* pWnd = pThis;
		if ( auto pForm = DYNAMIC_DOWNCAST( CFormView, pThis ) ) //-V2018
		{
			pWnd = pForm->GetParentFrame();
		}

		WINDOWPLACEMENT wp = { sizeof( WINDOWPLACEMENT ) };
		if ( pWnd->GetWindowPlacement( &wp ) )
		{
			AfxGetApp()->WriteProfileBinary( _T("Window"),
				CString( pThis->GetRuntimeClass()->m_lpszClassName ) + _T("_position"), reinterpret_cast< LPBYTE >( &wp ), sizeof( WINDOWPLACEMENT ) );
		}
	}

	virtual void RestoreWindowPlacement(BOOL bShiftBlock = TRUE)
	{
		auto pThis = static_cast< T* >( this );

		if ( bShiftBlock && GetKeyState( VK_SHIFT ) < 0 )
		{
			return;
		}

		ATL::CAutoVectorPtr< WINDOWPLACEMENT > wp;
		UINT wp_size = 0;
		if ( AfxGetApp()->GetProfileBinary( _T("Window"),
			CString( pThis->GetRuntimeClass()->m_lpszClassName ) + _T("_position"), reinterpret_cast< LPBYTE* >( &wp ), &wp_size ) && wp_size == sizeof( WINDOWPLACEMENT ) )
		{
			CWnd* pWnd = pThis;
			if ( auto pForm = DYNAMIC_DOWNCAST( CFormView, pThis ) ) //-V2018
			{
				pWnd = pForm->GetParentFrame();
			}
			pWnd->SetWindowPlacement( wp );
		}
	}

	virtual void OnMinMaxInfo(MINMAXINFO* lpMMI)
	{
		if ( m_sizeMin != CSize( 0, 0 ) )
		{
			lpMMI->ptMaxTrackSize.x = GetSystemMetrics( SM_CXMAXIMIZED );
			lpMMI->ptMaxTrackSize.y = GetSystemMetrics( SM_CYMAXIMIZED );
			lpMMI->ptMinTrackSize.x = std::min( m_sizeMin.cx,  lpMMI->ptMaxTrackSize.x );
			lpMMI->ptMinTrackSize.y = std::min( m_sizeMin.cy, lpMMI->ptMaxTrackSize.y );
		}
	}

protected:
	CSize m_sizeMin, m_sizeMinClient;
};
