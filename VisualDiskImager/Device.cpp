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
#include "Device.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CDevice

CDevice::CDevice(LPCTSTR szDeviceID)
	: Name		( szDeviceID )
	, Info		()
	, Writable	( false )
	, System	( false )
{
}

bool CDevice::Init(IWbemClassObject* disk)
{
	CComVariant id, model, type;
	if ( SUCCEEDED( disk->Get( L"DeviceID", 0, &id, nullptr, nullptr ) ) &&
		 SUCCEEDED( disk->Get( L"Model", 0, &model, nullptr, nullptr ) ) &&
		 SUCCEEDED( disk->Get( L"InterfaceType", 0, &type, nullptr, nullptr ) ) )
	{
		VERIFY( SUCCEEDED( id.ChangeType( VT_BSTR ) ) );
		Name = id;

		VERIFY( SUCCEEDED( model.ChangeType( VT_BSTR ) ) );
		Model = model;

		VERIFY( SUCCEEDED( type.ChangeType( VT_BSTR ) ) );
		Type = type;

		if ( Open( false ) )
		{
			if ( DiskSize() == 0 )
			{
				Writable = false;
			}

			Close();
		}

		return true;
	}
	return false;
}

void CDevice::GetDeviceVolumes(bool bSilent)
{
	if ( ! bSilent ) Log( LOG_ACTION, IDS_ENUM_VOLUME, static_cast< LPCTSTR >( Name ) );

	Volumes.clear();

	// Windows system directory
	TCHAR szSystem[ MAX_PATH ] = {};
	GetSystemDirectory( szSystem, MAX_PATH );

	// Looking for volumes on disk
	TCHAR szVolumeName[ MAX_PATH ] = {};
	HANDLE hFindVolume = FindFirstVolume( szVolumeName, MAX_PATH );
	if ( hFindVolume != INVALID_HANDLE_VALUE )
	{
		do
		{
			// Volume name without ending slash
			auto volume = std::make_unique< CDeviceVolume >( CString( szVolumeName ).TrimRight( _T('\\') ) );

			bool bSystemVolume = false;

			// Get disk DOS names
			TCHAR buf_name[ 1024 ];
			DWORD returned_name = 0;
			if ( GetVolumePathNamesForVolumeName( szVolumeName, buf_name, _countof( buf_name ), &returned_name ) )
			{
				size_t name_len;
				for ( LPCTSTR name = buf_name; *name; name += name_len )
				{
					name_len = _tcslen( name );

					volume->PathNames.emplace_back( name );

					// Check for system volume
					if ( StrCmpNI( name, szSystem, static_cast< int >( name_len ) ) == 0 )
					{
						bSystemVolume = true;
					}
				}
			}

			// Get volume disk hosting information
			CAtlFile volume_file;
			HRESULT hr = volume_file.Create( volume->Name, FILE_READ_ATTRIBUTES, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0 );
			if ( SUCCEEDED( hr ) )
			{
				union { VOLUME_DISK_EXTENTS extents; char buf[ 1024 ]; } buf_extents;
				DWORD returned_extents = 0;
				if ( DeviceIoControl( volume_file, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0, &buf_extents, sizeof( buf_extents ), &returned_extents, nullptr ) )
				{
					for ( DWORD extent = 0; extent < buf_extents.extents.NumberOfDiskExtents; ++ extent )
					{
						CString device_id;
						device_id.Format( _T("\\\\.\\PHYSICALDRIVE%u"), buf_extents.extents.Extents[ extent ].DiskNumber );

						// Add volume to owner device
						if ( StrCmpI( device_id, Name ) == 0 )
						{
							if ( bSystemVolume )
							{
								System = true;
							}

							if ( ! bSilent )
							{
								CString names;
								for ( const auto & path : volume->PathNames )
								{
									names += path;
									names.TrimRight( _T('\\') );
									names += _T(' ');
								}
								Log( LOG_INFO, IDS_VOLUME_INFO,
									static_cast< LPCTSTR >( volume->Name ),
									static_cast< LPCTSTR >( names ),
									static_cast< LPCTSTR >( FormatByteSize( buf_extents.extents.Extents[ extent ].ExtentLength.QuadPart ) ),
									buf_extents.extents.Extents[ extent ].StartingOffset.QuadPart,
									buf_extents.extents.Extents[ extent ].StartingOffset.QuadPart / BytesPerSector() );
							}

							Volumes.emplace_back( std::move( volume ) );
							break;
						}
					}
				}
			}
		}
		while ( FindNextVolume( hFindVolume, szVolumeName, MAX_PATH ) );
		FindVolumeClose( hFindVolume );
	}

	if ( Volumes.empty() )
	{
		if ( ! bSilent ) Log( LOG_INFO, IDS_VOLUME_EMPTY );
	}
}

bool CDevice::Open(bool bWrite)
{
	ASSERT( ! Name.IsEmpty() );

	if ( ! m_h )
	{
		auto hr = CAtlFile::Create( Name, ( bWrite ? GENERIC_WRITE : 0 ) | GENERIC_READ,
			FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, FILE_FLAG_NO_BUFFERING );
		if ( FAILED( hr ) || ! m_h )
		{
			Log( LOG_ERROR, IDS_DEVICE_MISSING, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
			return false;
		}

		DWORD returned = 0;
		if ( ! DeviceIoControl( m_h, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0, &Info, sizeof( Info ), &returned, nullptr ) )
		{
			Log( LOG_WARNING, IDS_DEVICE_INFO_ERROR, static_cast< LPCTSTR >( GetErrorString( GetLastError() ) ) );
		}

		if ( DeviceIoControl( m_h, IOCTL_DISK_IS_WRITABLE, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Writable = true;
		}
		else if ( GetLastError() == ERROR_WRITE_PROTECT )
		{
			Writable = false;
		}
	}
	return true;
}

bool CDevice::Update()
{
	if ( m_h )
	{
		Log( LOG_ACTION, IDS_DEVICE_UPDATE );

		DWORD returned;
		if ( ! DeviceIoControl( m_h, IOCTL_DISK_UPDATE_PROPERTIES, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Log( LOG_ERROR, IDS_DEVICE_UPDATE_ERROR, static_cast< LPCTSTR >( GetErrorString( GetLastError() ) ) );
			return false;
		}
		return true;
	}
	return false;
}

bool CDevice::Eject()
{
	if ( m_h )
	{
		Log( LOG_ACTION, IDS_DEVICE_EJECT );

		DWORD returned;
		if ( ! DeviceIoControl( m_h, IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Log( LOG_ERROR, IDS_DEVICE_EJECT_ERROR, static_cast< LPCTSTR >( GetErrorString( GetLastError() ) ) );
			return false;
		}
		return true;
	}
	return false;
}
