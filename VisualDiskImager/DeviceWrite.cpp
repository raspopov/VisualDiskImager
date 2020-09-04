// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "VisualDiskImager.h"
#include "VisualDiskImagerDlg.h"
#include "DeviceVolume.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CVisualDiskImagerDlg

void CVisualDiskImagerDlg::WriteDiskThread(CVisualDiskImagerDlg* pThis, LPCTSTR szFilename, LPCTSTR szDevice, bool bWrite, bool bVerify)
{
	pThis->WriteDisk( szFilename, szDevice, bWrite, bVerify );

	pThis->PostMessage( WM_DONE );
}

void CVisualDiskImagerDlg::WriteDisk(LPCTSTR szFilename, LPCTSTR szDevice, bool bWrite, bool bVerify)
{
	Log( LOG_ACTION, IDS_FILE, szFilename );

	// Check file type
	WIN32_FILE_ATTRIBUTE_DATA wfad = {};
	if ( ! GetFileAttributesEx( szFilename, GetFileExInfoStandard, &wfad ) )
	{
		Log( LOG_ERROR, IDS_FILE_MISSING, (LPCTSTR)GetErrorString( GetLastError() ) );
		return;
	}
	if ( ( wfad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0 )
	{
		Log( LOG_ERROR, IDS_FILE_DIRECTORY );
		return;
	}

	// Open file
	CAtlFile file;
	HRESULT hr = file.Create( szFilename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0 );
	if ( FAILED( hr ) )
	{
		Log( LOG_ERROR, IDS_FILE_MISSING, (LPCTSTR)GetErrorString( hr ) );
		return;
	}

	// Check file size
	ULONGLONG file_size = 0;
	hr = file.GetSize( file_size );
	if ( FAILED( hr ) )
	{
		Log( LOG_ERROR, IDS_FILE_MISSING, (LPCTSTR)GetErrorString( hr ) );
		return;
	}
	if ( file_size == 0 )
	{
		Log( LOG_ERROR, IDS_FILE_ZERO_SIZE );
		return;
	}
	Log( LOG_INFO, IDS_FILE_SIZE, (LPCTSTR)FormatByteSizeEx( file_size ) );

	CDevice device( szDevice );
	device.GetDeviceVolumes( false );

	if ( bWrite )
	{
		// Lock device volumes
		for ( const auto& volume : device.Volumes )
		{
			volume->Lock();
		}

		// Unmount device volumes
		for ( const auto& volume : device.Volumes )
		{
			volume->Dismount();
		}
	}

	// Open disk
	Log( LOG_ACTION, IDS_DEVICE, szDevice );
	if ( ! device.Open( bWrite ) )
	{
		return;
	}

	// Check device size
	const ULONGLONG disk_size = device.Info.DiskSize.QuadPart;
	Log( LOG_INFO, IDS_DEVICE_SIZE, (LPCTSTR)FormatByteSizeEx( disk_size ) );
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
	const DWORD bytes_per_sector = device.Info.Geometry.BytesPerSector ? device.Info.Geometry.BytesPerSector : 512;
	if ( file_size % bytes_per_sector != 0 )
	{
		Log( LOG_WARNING, IDS_FILE_SECTOR_SIZE, (LPCTSTR)FormatByteSize( bytes_per_sector ) );
	}

	const DWORD buf_size = 1024 * 1024; // 1MB
	CVirtualBuffer file_buf( buf_size );
	ULONGLONG leftover = 0;

	// Write to disk
	if ( bWrite && ! m_bCancel )
	{
		Log( LOG_ACTION, IDS_WRITING );

		leftover = min( file_size, disk_size );
		while ( leftover && ! m_bCancel )
		{
			const DWORD to_read = ( leftover > buf_size ) ? buf_size : (DWORD)leftover;
			const DWORD to_write = ( to_read / bytes_per_sector + ( ( to_read % bytes_per_sector ) ? 1 : 0 ) ) * bytes_per_sector;

			memset( file_buf, 0, to_write );

			hr = file.Read( file_buf, to_read );
			if ( FAILED( hr ) )
			{
				Log( LOG_ERROR, IDS_FILE_READ_ERROR, (LPCTSTR)GetErrorString( hr ) );
				break;
			}

			DWORD written = 0;
			hr = device.Write( file_buf, to_write, &written );
			if ( FAILED( hr ) || written != to_write )
			{
				Log( LOG_ERROR, IDS_DEVICE_WRITE_ERROR, (LPCTSTR)GetErrorString( hr ) );
				break;
			}

			leftover -= to_read;

			m_wndProgress.PostMessage( PBM_SETPOS, (int)( ( ( file_size - leftover ) * 100 ) / file_size ) );
		}

		device.Flush();

		if ( leftover == 0 )
		{
			// Success
			Log( LOG_INFO, IDS_WRITE_OK );
		}
		else
		{
			// Errors
			Log( LOG_WARNING, IDS_WRITE_LEFT, (LPCTSTR)FormatByteSize( file_size - leftover ), (LPCTSTR)FormatByteSize( file_size ) );
		}
	}

	if ( leftover == 0 && bVerify && ! m_bCancel )
	{
		Log( LOG_ACTION, IDS_VERIFYING );

		hr = file.Seek( 0, FILE_BEGIN );
		if ( SUCCEEDED( hr ) )
		{
			hr = device.Seek( 0, FILE_BEGIN );
			if ( SUCCEEDED( hr ) )
			{
				CVirtualBuffer device_buf( buf_size );
				leftover = min( file_size, disk_size );
				while ( leftover && ! m_bCancel )
				{
					const DWORD to_read = ( leftover > buf_size ) ? buf_size : (DWORD)leftover;
					const DWORD to_write = ( to_read / bytes_per_sector + ( ( to_read % bytes_per_sector ) ? 1 : 0 ) ) * bytes_per_sector;

					memset( file_buf, 0, to_write );

					hr = file.Read( file_buf, to_read );
					if ( FAILED( hr ) )
					{
						Log( LOG_ERROR, IDS_FILE_READ_ERROR, (LPCTSTR)GetErrorString( hr ) );
						break;
					}

					memset( device_buf, 0, to_write );

					hr = device.Read( device_buf, to_write );
					if ( FAILED( hr ) )
					{
						Log( LOG_ERROR, IDS_DEVICE_READ_ERROR, (LPCTSTR)GetErrorString( hr ) );
						break;
					}

					if ( memcmp( file_buf, device_buf, to_read ) != 0 )
					{
						Log( LOG_ERROR, IDS_VERIFY_ERROR );
						break;
					}

					leftover -= to_read;

					m_wndProgress.PostMessage( PBM_SETPOS, (int)( ( ( file_size - leftover ) * 100 ) / file_size ) );
				}

				if ( leftover == 0 )
				{
					// Success
					Log( LOG_INFO, IDS_VERIFY_OK );
				}
			}
			else
			{
				Log( LOG_ERROR, IDS_DEVICE_READ_ERROR, (LPCTSTR)GetErrorString( hr ) );
			}
		}
		else
		{
			Log( LOG_ERROR, IDS_FILE_READ_ERROR, (LPCTSTR)GetErrorString( hr ) );
		}
	}

	if ( bWrite )
	{
		// Unlock device volumes
		for ( const auto& volume : device.Volumes )
		{
			volume->Unlock();
		}

		// Update device
		device.Update();
	}

	Log( LOG_INFO, IDS_DONE );
}
