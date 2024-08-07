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
#include "DeviceVolume.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CVisualDiskImagerDlg

void CVisualDiskImagerDlg::WriteDiskThread(CVisualDiskImagerDlg* pThis, CString sFilename, CString sDevice, ULONGLONG offset)
{
	pThis->WriteDisk( sFilename, sDevice, offset );

	pThis->PostMessage( WM_DONE );
}

void CVisualDiskImagerDlg::WriteDisk(CString sFilename, CString sDevice, ULONGLONG offset)
{
	Log( LOG_ACTION, IDS_FILE, static_cast< LPCTSTR >( sFilename ) );

	// Check file type
	if ( ( GetFileAttributes( LONG_PATH + sFilename ) & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
	{
		Log( LOG_ERROR, IDS_FILE_MISSING, static_cast< LPCTSTR >( GetErrorString( GetLastError() ) ) );
		return;
	}

	// Open file
	CAtlFile file;
	auto hr = file.Create( LONG_PATH + sFilename, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN );
	if ( FAILED( hr ) )
	{
		Log( LOG_ERROR, IDS_FILE_MISSING, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
		return;
	}

	// Check file size
	ULONGLONG file_size = 0;
	hr = file.GetSize( file_size );
	if ( FAILED( hr ) )
	{
		Log( LOG_ERROR, IDS_FILE_MISSING, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
		return;
	}
	if ( file_size == 0 )
	{
		Log( LOG_ERROR, IDS_FILE_ZERO_SIZE );
		return;
	}
	Log( LOG_INFO, IDS_FILE_SIZE, static_cast< LPCTSTR >( FormatByteSizeEx( file_size ) ) );

	CDevice device( sDevice );
	device.GetDeviceVolumes( false );

	// Lock device volumes
	for ( const auto & volume : device.Volumes )
	{
		volume->Lock();
	}

	// Unmount device volumes
	for ( const auto & volume : device.Volumes )
	{
		volume->Dismount();
	}

	// Open disk
	Log( LOG_ACTION, IDS_DEVICE, static_cast< LPCTSTR >( sDevice ) );
	if ( ! device.Open( m_Mode == MODE_WRITE || m_Mode == MODE_WRITE_VERIFY ) )
	{
		return;
	}

	// Check device size
	const ULONGLONG disk_size = device.Size();
	Log( LOG_INFO, IDS_DEVICE_SIZE, static_cast< LPCTSTR >( FormatByteSizeEx( disk_size ) ) );
	if ( disk_size < file_size )
	{
		if ( disk_size == 0 )
		{
			Log( LOG_ERROR, IDS_DEVICE_SIZE_MISMATCH );
			return;
		}
		else
		{
			Log( LOG_WARNING, IDS_DEVICE_SIZE_MISMATCH );
		}
	}

	const auto bytes_per_sector = device.BytesPerSector();
	const auto size = std::min( file_size, disk_size );
	const auto buf_size = 2 * 1024 * bytes_per_sector; // 1MB
	CVirtualBuffer< char > file_buf( buf_size );
	ULONGLONG completed = 0;

	hr = device.Seek( offset * device.BytesPerSector(), FILE_BEGIN );
	if ( FAILED( hr ) )
	{
		Log( LOG_ERROR, IDS_DEVICE_SEEK_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
		m_bCancel = true;
	}
	else if ( ! m_bCancel && ( m_Mode == MODE_WRITE || m_Mode == MODE_WRITE_VERIFY ) )
	{
		// Write to disk
		Log( LOG_ACTION, IDS_WRITING );

		const auto start = CTime::GetCurrentTime();

		while ( completed != size && ! m_bCancel )
		{
			Sleep( 0 );

			const auto to_read = static_cast< DWORD >( std::min( size - completed, static_cast< ULONGLONG >( buf_size ) ) );

			hr = file.Read( file_buf, to_read );
			if ( FAILED( hr ) )
			{
				Log( LOG_ERROR, IDS_FILE_READ_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
				break;
			}

			DWORD written = 0;
			hr = device.Write( file_buf, to_read, &written );
			completed += written;

			if ( FAILED( hr ) || written != to_read )
			{
				Log( LOG_ERROR, IDS_DEVICE_WRITE_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
				break;
			}

			m_nProgress = static_cast< int >( ( completed * 100 ) / file_size );
		}

		device.Flush();

		if ( completed == size )
		{
			// Success
			const auto elapsed = CTime::GetCurrentTime() - start;
			Log( LOG_INFO, IDS_WRITE_OK, static_cast< LPCTSTR >( elapsed.Format( _T("%H:%M:%S") ) ) );
		}
		else
		{
			// Errors
			Log( LOG_WARNING, IDS_WRITE_ERROR, static_cast< LPCTSTR >( FormatByteSize( completed ) ) );
		}
	}

	Sleep( 500 );

	if ( ! m_bCancel && ( m_Mode == MODE_VERIFY || ( m_Mode == MODE_WRITE_VERIFY && completed == size ) ) )
	{
		// Verify disk
		Log( LOG_ACTION, IDS_VERIFYING );

		if ( FAILED( hr = file.Seek( 0, FILE_BEGIN ) ) )
		{
			Log( LOG_ERROR, IDS_FILE_READ_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
			m_bCancel = true;
		}
		else if ( FAILED( hr = device.Seek( offset * device.BytesPerSector(), FILE_BEGIN ) ) )
		{
			Log( LOG_ERROR, IDS_DEVICE_SEEK_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
			m_bCancel = true;
		}
		else
		{
			CVirtualBuffer< char > device_buf( buf_size );
			completed = 0;
			while ( completed != size && ! m_bCancel )
			{
				Sleep( 0 );

				const auto to_read = static_cast< DWORD >( std::min( size - completed, static_cast< ULONGLONG >( buf_size ) ) );
				auto to_verify = to_read;
				if ( to_read % bytes_per_sector != 0 )
				{
					to_verify = ( to_read / bytes_per_sector + 1 ) * bytes_per_sector;
				}

				if ( FAILED( hr = file.Read( file_buf, to_read ) ) )
				{
					Log( LOG_ERROR, IDS_FILE_READ_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
					break;
				}

				if ( FAILED( hr = device.Read( device_buf, to_verify ) ) )
				{
					Log( LOG_ERROR, IDS_DEVICE_READ_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
					break;
				}

				// Compare
				auto count = to_read;
				for ( char * p1 = file_buf, *p2 = device_buf; count; --count )
				{
					if ( *p1++ != *p2++ )
						break;
				}
				if ( count )
				{
					Log( LOG_ERROR, IDS_VERIFY_ERROR, completed + to_read - count );
					break;
				}

				completed += to_read;

				m_nProgress = static_cast< int >( ( completed * 100 ) / file_size );
			}

			if ( completed == size )
			{
				// Success
				Log( LOG_INFO, IDS_VERIFY_OK );
			}
		}
	}

	// Unlock device volumes
	for ( const auto & volume : device.Volumes )
	{
		volume->Unlock();
	}

	// Update device
	device.Update();

	if ( m_bCancel )
	{
		Log( LOG_WARNING, IDS_CANCELED );
	}
	else
	{
		Log( LOG_INFO, IDS_DONE );
	}
}
