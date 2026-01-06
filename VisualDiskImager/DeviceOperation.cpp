// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*
This file is part of Visual Disk Imager

Copyright (C) 2020-2025 Nikolay Raspopov <raspopov@cherubicsoft.com>

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

void CVisualDiskImagerDlg::Start(Mode mode)
{
	if ( auto pdevice = GetSelected() )
	{
		m_Mode = mode;

		UpdateInterface();

		Cancel( false );
		m_Thread = std::thread( &CVisualDiskImagerDlg::DeviceOperationThread, this, mode, m_Filename, pdevice->Top()->Name, m_Offset, m_Size );
	}
}

void CVisualDiskImagerDlg::Stop()
{
	if ( IsStarted() )
	{
		Cancel();

		try
		{
			m_Thread.join();
		}
		catch (...)
		{
		}

		if ( IsStarted() )
		{
			m_Thread.detach();
		}
	}

	m_Mode = MODE_STOP;

	UpdateInterface();
}

void CVisualDiskImagerDlg::DeviceOperationThread(CVisualDiskImagerDlg* pThis, Mode mode, CString sFilename, CString sDevice, ULONGLONG offset, ULONGLONG size)
{
	pThis->DeviceOperation( mode, sFilename, sDevice, offset, size );

	pThis->PostMessage( WM_DONE );
}

void CVisualDiskImagerDlg::DeviceOperation(Mode mode, CString sFilename, CString sDevice, ULONGLONG offset, ULONGLONG size)
{
	Log( LOG_ACTION, IDS_FILE, static_cast< LPCTSTR >( sFilename ) );

	// Check file type
	if ( ( mode != MODE_READ ) && ( GetFileAttributes( LONG_PATH + sFilename ) & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
	{
		Log( LOG_ERROR, IDS_FILE_MISSING, static_cast< LPCTSTR >( GetErrorString( GetLastError() ) ) );
		return;
	}

	// Open file
	CAtlFile file;
	auto hr = file.Create( LONG_PATH + sFilename,
		( mode != MODE_READ ) ? GENERIC_READ : GENERIC_WRITE,
		( mode != MODE_READ ) ? FILE_SHARE_READ : 0,
		( mode != MODE_READ ) ? OPEN_EXISTING : CREATE_ALWAYS,
		FILE_FLAG_SEQUENTIAL_SCAN );
	if ( FAILED( hr ) )
	{
		Log( LOG_ERROR, IDS_FILE_MISSING, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
		return;
	}

	// Check file size
	if ( mode != MODE_READ )
	{
		hr = file.GetSize( size );
		if ( FAILED( hr ) )
		{
			Log( LOG_ERROR, IDS_FILE_MISSING, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
			return;
		}
		if ( size == 0 )
		{
			Log( LOG_ERROR, IDS_FILE_ZERO_SIZE );
			return;
		}
		Log( LOG_INFO, IDS_FILE_SIZE, static_cast< LPCTSTR >( FormatByteSizeEx( size ) ) );
	}

	CDevice device( sDevice );
	device.GetDeviceVolumes( false );

	// Lock device volumes
	for ( const auto & volume : device.Volumes )
	{
		if ( volume->isOverlap( offset, size ) )
		{
			volume->Lock();
		}
	}

	// Dismount device volumes
	for ( const auto & volume : device.Volumes )
	{
		if ( volume->isOverlap( offset, size ) )
		{
			volume->Dismount();
		}
	}

	// Open disk
	Log( LOG_ACTION, IDS_DEVICE, static_cast< LPCTSTR >( sDevice ) );
	if ( ! device.Open( mode == MODE_WRITE || mode == MODE_WRITE_VERIFY ) )
	{
		return;
	}

	// Get device size
	const auto offset_bytes = offset * device.BytesPerSector();
	Log( LOG_INFO, IDS_DEVICE_SIZE, static_cast< LPCTSTR >( FormatByteSizeEx( device.Size() ) ) );
	if ( mode == MODE_READ && size == 0 && device.Size() > size + offset_bytes )
	{
		size = device.Size() - offset_bytes;
	}

	// Check device size
	if ( mode != MODE_READ && device.Size() < size + offset_bytes )
	{
		if ( device.Size() == 0 )
		{
			Log( LOG_ERROR, IDS_DEVICE_SIZE_MISMATCH );
			return;
		}
		else
		{
			Log( LOG_WARNING, IDS_DEVICE_SIZE_MISMATCH );
		}
	}

	// Writing disk
	bool success = ! isCancelled() && ( mode == MODE_WRITE || mode == MODE_WRITE_VERIFY ) && DeviceOperationWrite( device, file, offset, size );

	// Verify/Reading disk
	if ( ! isCancelled() && ( mode == MODE_READ || mode == MODE_VERIFY || ( mode == MODE_WRITE_VERIFY && success ) ) )
	{
		DeviceOperationRead( device, file, offset, size, mode != MODE_READ );
	}

	// Unlock device volumes
	for ( const auto & volume : device.Volumes )
	{
		volume->Unlock();
	}

	// Update device
	device.Update();

	if ( isCancelled() )
	{
		Log( LOG_WARNING, IDS_CANCELED );
	}
	else
	{
		Log( LOG_INFO, IDS_DONE );
	}
}

bool CVisualDiskImagerDlg::DeviceOperationWrite(CDevice & device, CAtlFile & file, ULONGLONG offset, ULONGLONG size)
{
	HRESULT hr;
	const auto bytes_per_sector = device.BytesPerSector();
	const auto buf_size = 2 * 1024 * bytes_per_sector; // 1MB
	CVirtualBuffer< char > file_buf( buf_size );
	ULONGLONG completed = 0;

	Log( LOG_ACTION, IDS_WRITING );

	const auto start = CTime::GetCurrentTime();

	// Set operation offset
	if ( FAILED( hr = device.Seek( offset * bytes_per_sector, FILE_BEGIN ) ) )
	{
		Log( LOG_ERROR, IDS_DEVICE_SEEK_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
		Cancel();
	}

	while ( completed < size && ! isCancelled() )
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

		Progress( static_cast< int >( ( completed * 100 ) / size ) );
	}

	device.Flush();

	if ( completed == size )
	{
		// Success
		const auto elapsed = CTime::GetCurrentTime() - start;
		Log( LOG_INFO, IDS_WRITE_OK, static_cast< LPCTSTR >( elapsed.Format( _T("%H:%M:%S") ) ) );
		return true;
	}
	else if ( completed )
	{
		// Errors
		Log( LOG_WARNING, IDS_WRITE_ERROR, static_cast< LPCTSTR >( FormatByteSize( completed ) ) );
	}

	return false;
}

bool CVisualDiskImagerDlg::DeviceOperationRead(CDevice & device, CAtlFile & file, ULONGLONG offset, ULONGLONG size, bool isVerify)
{
	HRESULT hr;
	const auto bytes_per_sector = device.BytesPerSector();
	const auto buf_size = 2 * 1024 * bytes_per_sector; // 1MB
	CVirtualBuffer< char > file_buf( buf_size );
	CVirtualBuffer< char > device_buf( buf_size );
	ULONGLONG completed = 0;

	Log( LOG_ACTION, isVerify ? IDS_VERIFYING : IDS_READING );

	if ( isVerify && FAILED( hr = file.Seek( 0, FILE_BEGIN ) ) )
	{
		Log( LOG_ERROR, IDS_FILE_READ_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
		Cancel();
		return false;
	}

	ULONGLONG new_offset;
	if ( FAILED( hr = device.Seek( offset * bytes_per_sector, FILE_BEGIN ) ) ||
	     FAILED( hr = device.GetPosition( new_offset ) ) ||
		 new_offset != offset * bytes_per_sector )
	{
		Log( LOG_ERROR, IDS_DEVICE_SEEK_ERROR, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
		Cancel();
		return false;
	}

	while ( completed < size && ! isCancelled() )
	{
		Sleep( 0 );

		const auto part_size = static_cast< DWORD >( std::min( size - completed, static_cast< ULONGLONG >( buf_size ) ) );
		auto to_verify = part_size;
		if ( part_size % bytes_per_sector != 0 )
		{
			to_verify = ( part_size / bytes_per_sector + 1 ) * bytes_per_sector;
		}

		if ( FAILED( hr = device.Read( device_buf, to_verify ) ) )
		{
			Log( LOG_ERROR, IDS_DEVICE_READ_ERROR, completed, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
			break;
		}

		if ( isVerify )
		{
			if ( FAILED( hr = file.Read( file_buf, part_size ) ) )
			{
				Log( LOG_ERROR, IDS_FILE_READ_ERROR, completed, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
				break;
			}
		}
		else
		{
			if ( FAILED( hr = file.Write( device_buf, part_size ) ) )
			{
				Log( LOG_ERROR, IDS_FILE_WRITE_ERROR, completed, static_cast< LPCTSTR >( GetErrorString( hr ) ) );
				break;
			}
		}

		// Compare
		if ( isVerify )
		{
			auto count = part_size;
			for ( char * p1 = file_buf, *p2 = device_buf; count; --count )
			{
				if ( *p1++ != *p2++ )
					break;
			}
			if ( count )
			{
				Log( LOG_ERROR, IDS_VERIFY_ERROR, completed + part_size - count );
				break;
			}
		}

		completed += part_size;

		Progress( static_cast< int >( ( completed * 100 ) / size ) );
	}

	if ( completed == size )
	{
		// Success
		Log( LOG_INFO, isVerify ? IDS_VERIFY_OK : IDS_READ_OK );
		return true;
	}

	return false;
}
