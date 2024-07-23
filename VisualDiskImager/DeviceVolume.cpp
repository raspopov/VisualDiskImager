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
#include "DeviceVolume.h"
#include "Device.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CDeviceVolume

CDeviceVolume::~CDeviceVolume()
{
	Unlock();
}

bool CDeviceVolume::Init(const CItem * parent, const DISK_EXTENT & extent)
{
	CString str;
	str.Format( _T("\\\\.\\PHYSICALDRIVE%u"), extent.DiskNumber );
	if ( StrCmpI( parent->Name, str ) == 0 )
	{
		Parent = parent;
		Start = extent.StartingOffset.QuadPart / BytesPerSector();
		Length = extent.ExtentLength.QuadPart;
		return true;
	}

	return false;
}

CString CDeviceVolume::Paths() const
{
	CString names;
	for ( const auto & path : PathNames )
	{
		names += path;
		names.TrimRight( _T('\\') );
		names += _T(' ');
	}
	return names;
}

bool CDeviceVolume::Open()
{
	ASSERT( ! Name.IsEmpty() );

	if ( ! m_h )
	{
		auto hr = CAtlFile::Create( Name, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0 );
		if ( FAILED( hr ) )
		{
			Log( LOG_WARNING, IDS_VOLUME_MISSING, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
			return false;
		}
	}
	return true;
}

bool CDeviceVolume::Lock()
{
	if ( Open() )
	{
		if ( ! m_bLocked )
		{
			Log( LOG_ACTION, IDS_VOLUME_LOCK, static_cast< LPCTSTR >( Name ) );

			DWORD returned;
			if ( ! DeviceIoControl( m_h, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
			{
				Log( LOG_WARNING, IDS_VOLUME_LOCK_ERROR, static_cast< LPCTSTR >( GetErrorString( GetLastError() ) ) );
				return false;
			}

			m_bLocked = true;
		}
		return true;
	}
	return false;
}

bool CDeviceVolume::Unlock()
{
	if ( m_h )
	{
		if ( m_bLocked )
		{
			Log( LOG_ACTION, IDS_VOLUME_UNLOCK, static_cast< LPCTSTR >( Name ) );

			DWORD returned;
			if ( ! DeviceIoControl( m_h, FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
			{
				Log( LOG_WARNING, IDS_VOLUME_UNLOCK_ERROR, static_cast< LPCTSTR >( GetErrorString( GetLastError() ) ) );
			}

			m_bLocked = false;
		}

		CAtlFile::Close();

		return true;
	}
	return false;
}

bool CDeviceVolume::Dismount()
{
	if ( m_h )
	{
		Log( LOG_ACTION, IDS_VOLUME_DISMOUNT, static_cast< LPCTSTR >( Name ) );

		DWORD returned;
		if ( ! DeviceIoControl( m_h, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Log( LOG_WARNING, IDS_VOLUME_DISMOUNT_ERROR, static_cast< LPCTSTR >( GetErrorString( GetLastError() ) ) );
			return false;
		}
		return true;
	}
	return false;
}
