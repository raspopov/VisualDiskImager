// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "VisualDiskImager.h"
#include "DeviceVolume.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDeviceVolume

CDeviceVolume::CDeviceVolume(LPCTSTR szVolumeName)
	: Name		( szVolumeName )
	, m_bLocked	( false )
{
}

CDeviceVolume::~CDeviceVolume()
{
	Unlock();
}

bool CDeviceVolume::Open()
{
	ASSERT( ! Name.IsEmpty() );

	if ( m_h == NULL )
	{
		HRESULT hr = CAtlFile::Create( Name, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0 );
		if ( FAILED( hr ) )
		{
			Log( LOG_WARNING, IDS_VOLUME_MISSING, (LPCTSTR)GetErrorString( hr ) );
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
			Log( LOG_ACTION, IDS_VOLUME_LOCK, (LPCTSTR)Name );

			DWORD returned;
			if ( ! DeviceIoControl( m_h, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
			{
				Log( LOG_WARNING, IDS_VOLUME_LOCK_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
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
	if ( m_h != NULL )
	{
		if ( m_bLocked )
		{
			Log( LOG_ACTION, IDS_VOLUME_UNLOCK, (LPCTSTR)Name );

			DWORD returned;
			if ( ! DeviceIoControl( m_h, FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
			{
				Log( LOG_WARNING, IDS_VOLUME_UNLOCK_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
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
	if ( m_h != NULL )
	{
		Log( LOG_ACTION, IDS_VOLUME_DISMOUNT, (LPCTSTR)Name );

		DWORD returned;
		if ( ! DeviceIoControl( m_h, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Log( LOG_WARNING, IDS_VOLUME_DISMOUNT_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
			return false;
		}
		return true;
	}
	return false;
}
