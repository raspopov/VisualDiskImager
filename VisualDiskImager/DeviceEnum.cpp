// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "VisualDiskImager.h"
#include "VisualDiskImagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CVisualDiskImagerDlg::EnumDevices(bool bUseDefault)
{
	Log( LOG_ACTION, IDS_DEVICE_ENUM );

	ClearDevices();

	UpdateWindow();

	// Looking for disk devices
	CComPtr< IWbemLocator > locator;
	HRESULT hr = locator.CoCreateInstance( CLSID_WbemLocator );
	if ( SUCCEEDED( hr ) )
	{
		CComPtr< IWbemServices > services;
		hr = locator->ConnectServer( CComBSTR( L"ROOT\\CIMV2" ), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &services );
		if ( SUCCEEDED( hr ) )
		{
			hr = CoSetProxyBlanket( services, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
				nullptr, EOAC_NONE );
			if ( SUCCEEDED( hr ) )
			{
				CComPtr< IEnumWbemClassObject > disks;
				hr = services->ExecQuery( CComBSTR( L"WQL" ), CComBSTR( L"SELECT * FROM Win32_DiskDrive" ),
					WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &disks );
				if ( SUCCEEDED( hr ) )
				{
					for (;;)
					{
						CComPtr< IWbemClassObject > disk;
						ULONG uReturn = 0;
						hr = disks->Next( WBEM_INFINITE, 1, &disk, &uReturn );
						if ( FAILED( hr ) || uReturn == 0 )
						{
							break;
						}

						auto pdevice = std::make_unique< CDevice >();
						if ( pdevice->Init( disk ) )
						{
							Log( LOG_INFO, IDS_DEVICE_FOUND, (LPCTSTR)pdevice->DeviceID );
							m_Devices.push_back( std::move( pdevice ) );
						}
						else
						{
							Log( LOG_INFO, IDS_DEVICE_SKIP, (LPCTSTR)pdevice->DeviceID );
						}
					}
				}
			}
		}
	}

	if ( FAILED ( hr ) )
	{
		Log( LOG_WARNING, IDS_WMI_ERROR, (LPCTSTR)GetErrorString( hr) );
	}

	Log( LOG_ACTION, IDS_ENUM_VOLUME );

	// Windows system directory
	TCHAR szSystem[ MAX_PATH ];
	GetSystemDirectory( szSystem, MAX_PATH );

	// Looking for volumes on disk
	TCHAR szVolumeName[ MAX_PATH ];
	HANDLE hFindVolume = FindFirstVolume( szVolumeName, MAX_PATH );
	if ( hFindVolume != INVALID_HANDLE_VALUE )
	{
		TCHAR buf[ 1024 ];
		DWORD returned;

		do
		{
			// Volume name without ending slash
			CString sVolumeNameNoSlash( szVolumeName );
			sVolumeNameNoSlash.TrimRight( _T('\\') );

			bool bSystem = false;

			// Get disk DOS names
			CString sVolumePathName;
			if ( GetVolumePathNamesForVolumeName( szVolumeName, buf, _countof( buf ), &returned ) )
			{
				size_t name_len;
				for ( LPCTSTR name = buf; *name; name += name_len )
				{
					name_len = _tcslen( name );

					sVolumePathName += name;
					sVolumePathName += _T(' ');

					// Check for system volume
					if ( StrCmpNI( name, szSystem, static_cast< int >( name_len ) ) == 0 )
					{
						bSystem = true;
					}
				}
			}

			// Get volume disk hosting information
			CAtlFile volume;
			HRESULT hr = volume.Create( sVolumeNameNoSlash, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0 );
			if ( SUCCEEDED( hr ) )
			{
				if ( DeviceIoControl( volume, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0, buf, sizeof( buf ), &returned, nullptr ) )
				{
					PVOLUME_DISK_EXTENTS extents = reinterpret_cast< PVOLUME_DISK_EXTENTS >( buf );
					for ( DWORD extent = 0; extent < extents->NumberOfDiskExtents; ++ extent )
					{
						CString device_id;
						device_id.Format( _T("\\\\.\\PHYSICALDRIVE%u"), extents->Extents[ extent ].DiskNumber );

						if ( sVolumePathName.IsEmpty() )
						{
							Log( LOG_INFO, IDS_VOLUME_INFO, (LPCTSTR)sVolumeNameNoSlash, (LPCTSTR)device_id );
						}
						else
						{
							Log( LOG_INFO, IDS_VOLUME_INFO_MAP, (LPCTSTR)sVolumeNameNoSlash, (LPCTSTR)device_id, (LPCTSTR)sVolumePathName );
						}

						// Add volume to owner device
						for ( auto& device : m_Devices )
						{
							if ( StrCmpI( device_id, device->DeviceID ) == 0 )
							{
								// Exclude system volume
								if ( bSystem )
								{
									device->Writable = false;
								}

								device->Volumes.push_back( sVolumeNameNoSlash );
								break;
							}
						}
					}
				}
			}
		}
		while ( FindNextVolume( hFindVolume, szVolumeName, MAX_PATH ) );
		FindVolumeClose( hFindVolume );
	}

	// Fill interface list
	const CString sDefDeviceID = bUseDefault ? theApp.GetProfileString( REG_SETTINGS, REG_DEVICE ) : _T("");
	for ( const auto& pdevice : m_Devices )
	{
		CString sDevice;
		sDevice.Format( _T("%s \"%s\" (%s) %s %s%s"),
			(LPCTSTR)pdevice->DeviceID,
			(LPCTSTR)pdevice->Model,
			(LPCTSTR)FormatByteSize( pdevice->Size ),
			(LPCTSTR)pdevice->Type,
			pdevice->Removable ? _T("Removable") : _T("Fixed"),
			pdevice->Writable ? _T("") : _T(" (NOT RECOMMENDED)") );

		const int nIndex = m_wndDevices.AddString( sDevice );
		ASSERT( nIndex != CB_ERR );
		m_wndDevices.SetItemDataPtr( nIndex, pdevice.get() );

		// Select default one
		if ( StrCmpI( sDefDeviceID, pdevice->DeviceID ) == 0 )
		{
			m_wndDevices.SetCurSel( nIndex );
		}
	}

	Log( LOG_INFO, IDS_DONE );
}
