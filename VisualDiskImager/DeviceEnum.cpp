// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "VisualDiskImager.h"
#include "VisualDiskImagerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CVisualDiskImagerDlg::EnumDevices(bool bSilent)
{
	if ( ! bSilent ) Log( LOG_ACTION, IDS_DEVICE_ENUM );

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

						auto device = std::make_unique< CDevice >();
						if ( device->Init( disk ) )
						{
							if ( ! bSilent )
							{
								Log( LOG_INFO, IDS_DEVICE_FOUND, (LPCTSTR)device->Name );

								Log( LOG_INFO, IDS_DEVICE_INFO,
									(LPCTSTR)FormatByteSize( device->Info.DiskSize.QuadPart ),
									device->Info.DiskSize.QuadPart / ( device->Info.Geometry.BytesPerSector ? device->Info.Geometry.BytesPerSector : 512 ),
									(LPCTSTR)FormatByteSize( device->Info.Geometry.BytesPerSector ),
									device->Info.Geometry.Cylinders.QuadPart,
									device->Info.Geometry.TracksPerCylinder,
									device->Info.Geometry.SectorsPerTrack );
							}

							device->GetDeviceVolumes( bSilent );

							m_Devices.push_back( std::move( device ) );
						}
						else
						{
							if ( ! bSilent ) Log( LOG_INFO, IDS_DEVICE_SKIP, (LPCTSTR)device->Name );
						}
					}
				}
			}
		}
	}

	if ( FAILED ( hr ) )
	{
		if ( ! bSilent ) Log( LOG_WARNING, IDS_WMI_ERROR, (LPCTSTR)GetErrorString( hr) );
	}

	const CString sRemovable = LoadString( IDS_REMOVABLE );
	const CString sFixed = LoadString( IDS_FIXED );
	const CString sNotRecommended = LoadString( IDS_NOT_RECOMMENDED );

	// Fill interface list
	const CString sDefDeviceID = theApp.GetProfileString( REG_SETTINGS, REG_DEVICE );
	for ( const auto& device : m_Devices )
	{
		CString sDevice;
		sDevice.Format( _T("%s \"%s\" (%s) %s %s %s"),
			(LPCTSTR)device->Name,
			(LPCTSTR)device->Model,
			(LPCTSTR)FormatByteSize( device->Info.DiskSize.QuadPart ),
			(LPCTSTR)device->Type,
			( device->Removable ? (LPCTSTR)sRemovable : (LPCTSTR)sFixed ),
			( ( device->Writable && ! device->System ) ? _T("") : (LPCTSTR)sNotRecommended ) );

		const int nIndex = m_wndDevices.AddString( sDevice );
		ASSERT( nIndex != CB_ERR );
		m_wndDevices.SetItemDataPtr( nIndex, device.get() );

		// Select default one
		if ( StrCmpI( sDefDeviceID, device->Name ) == 0 )
		{
			m_wndDevices.SetCurSel( nIndex );
		}
	}

	if ( ! bSilent ) Log( LOG_INFO, IDS_DONE );

	UpdateWindow();
}
