// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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

#include "pch.h"
#include "VisualDiskImager.h"
#include "VisualDiskImagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CVisualDiskImagerDlg::EnumDevices(bool bSilent)
{
	// Looking for disk devices

	if ( ! bSilent ) { Log( LOG_ACTION, IDS_WMI_CREATE ); }

	CComPtr< IWbemLocator > locator;
	HRESULT hr = locator.CoCreateInstance( CLSID_WbemLocator );
	if ( SUCCEEDED( hr ) )
	{
		if ( ! bSilent ) { Log( LOG_INFO, IDS_SUCCESS ); Log( LOG_ACTION, IDS_WMI_CONNECT ); }

		CComPtr< IWbemServices > services;
		hr = locator->ConnectServer( CComBSTR( L"ROOT\\CIMV2" ), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services );
		if ( SUCCEEDED( hr ) )
		{
			if ( ! bSilent ) { Log( LOG_INFO, IDS_SUCCESS ); Log( LOG_ACTION, IDS_WMI_DISK ); }

			CComPtr< IEnumWbemClassObject > disks;
			hr = services->ExecQuery( CComBSTR( L"WQL" ), CComBSTR( L"SELECT * FROM Win32_DiskDrive" ),
				WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &disks );
			if ( SUCCEEDED( hr ) )
			{
				if ( ! bSilent ) { Log( LOG_INFO, IDS_SUCCESS ); Log( LOG_ACTION, IDS_DEVICE_ENUM ); }

				unsigned count = 0;

				for (;;)
				{
					CComPtr< IWbemClassObject > disk;
					ULONG uReturn = 0;
					hr = disks->Next( WBEM_INFINITE, 1, &disk, &uReturn );
					if ( FAILED( hr ) || uReturn == 0 )
					{
						break;
					}

					auto device = std::make_unique< CDevice >();
					if ( device->Init( disk ) )
					{
						if ( ! bSilent )
						{
							Log( LOG_INFO, IDS_DEVICE_INFO,
								static_cast< LPCTSTR >( device->Name ),
								static_cast< LPCTSTR >( device->Model ),
								static_cast< LPCTSTR >( FormatByteSize( device->DiskSize() ) ),
								device->DiskSize() / device->BytesPerSector(),
								device->Cylinders(),
								device->TracksPerCylinder(),
								device->SectorsPerTrack(),
								static_cast< LPCTSTR >( FormatByteSize( device->BytesPerSector() ) ) );
						}

						device->GetDeviceVolumes( bSilent );

						m_Devices.push_back( std::move( device ) );
					}
					else
					{
						if ( ! bSilent ) { Log( LOG_INFO, IDS_DEVICE_SKIP, static_cast< LPCTSTR >( device->Name ) ); }
					}

					++ count;
				}

				if ( ! bSilent && SUCCEEDED( hr ) && ! count ) { Log( LOG_INFO, IDS_DEVICE_NONE ); }
			}
		}
	}

	if ( ! bSilent && FAILED( hr ) ) { Log( LOG_WARNING, IDS_WMI_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) ); }

	const CString sRemovable = LoadString( IDS_REMOVABLE );
	const CString sFixed = LoadString( IDS_FIXED );
	const CString sNotRecommended = LoadString( IDS_NOT_RECOMMENDED );

	// Fill interface list
	const CString sDefDeviceID = theApp.GetProfileString( REG_SETTINGS, REG_DEVICE );
	for ( const auto& device : m_Devices )
	{
		CString sDevice;
		sDevice.Format( IDS_DEVICE_INFO_LIST,
			static_cast< LPCTSTR >( device->Name ),
			static_cast< LPCTSTR >( device->Model ),
			static_cast< LPCTSTR >( FormatByteSize( device->DiskSize() ) ),
			static_cast< LPCTSTR >( device->Type ),
			( device->Removable() ? static_cast< LPCTSTR >( sRemovable ) : static_cast< LPCTSTR >( sFixed ) ),
			( ( device->Writable && ! device->System ) ? _T("") : static_cast< LPCTSTR >( sNotRecommended ) ) );

		const int nIndex = m_wndDevices.AddString( sDevice );
		ASSERT( nIndex != CB_ERR );
		m_wndDevices.SetItemDataPtr( nIndex, device.get() );

		// Select default one
		if ( StrCmpI( sDefDeviceID, device->Name ) == 0 )
		{
			m_wndDevices.SetCurSel( nIndex );
		}
	}

	if ( ! bSilent ) { Log( LOG_INFO, IDS_DONE ); }
}
