// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "DeviceVolume.h"
#include "VisualDiskImager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDeviceVolume

CDeviceVolume::CDeviceVolume() noexcept
	: m_bLocked	( false )
{
}

CDeviceVolume::~CDeviceVolume()
{
	Unlock();
}

bool CDeviceVolume::Open(LPCTSTR szVolumeName)
{
	m_sName = szVolumeName;

	HRESULT hr = m_Volume.Create( szVolumeName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0 );
	if ( FAILED( hr ) )
	{
		Log( LOG_WARNING, IDS_VOLUME_MISSING, (LPCTSTR)GetErrorString( hr ) );
		return false;
	}

	return true;
}

bool CDeviceVolume::Lock()
{
	if ( ! m_bLocked )
	{
		Log( LOG_ACTION, IDS_VOLUME_LOCK, (LPCTSTR)m_sName );

		DWORD returned;
		if ( ! DeviceIoControl( m_Volume, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Log( LOG_WARNING, IDS_VOLUME_LOCK_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
			return false;
		}

		m_bLocked = true;
	}

	return true;
}

bool CDeviceVolume::Unlock()
{
	if ( m_bLocked )
	{
		Log( LOG_ACTION, IDS_VOLUME_UNLOCK, (LPCTSTR)m_sName );

		DWORD returned;
		if ( ! DeviceIoControl( m_Volume, FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Log( LOG_WARNING, IDS_VOLUME_UNLOCK_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
			return false;
		}

		m_bLocked = false;
	}

	return true;
}

bool CDeviceVolume::Dismount()
{
	Log( LOG_ACTION, IDS_VOLUME_DISMOUNT, (LPCTSTR)m_sName );

	DWORD returned;
	if ( ! DeviceIoControl( m_Volume, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &returned, nullptr ) )
	{
		Log( LOG_WARNING, IDS_VOLUME_DISMOUNT_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
		return false;
	}

	return true;
}
