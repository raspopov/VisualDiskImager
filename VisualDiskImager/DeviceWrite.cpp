// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "VisualDiskImager.h"
#include "VisualDiskImagerDlg.h"
#include "DeviceVolume.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CVisualDiskImagerDlg::WriteDiskThread(CVisualDiskImagerDlg* pThis, LPCTSTR szFilename, CDevice* pdevice)
{
	pThis->WriteDisk( szFilename, pdevice );

	pThis->PostMessage( WM_DONE );
}

void CVisualDiskImagerDlg::WriteDisk(LPCTSTR szFilename, CDevice* pdevice)
{
	const DWORD buf_size = 64 * 1024;
	CAutoVectorPtr< char > buf( new char[ buf_size ] );
	DWORD returned;

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
	Log( LOG_INFO, IDS_FILE_SIZE, (LPCTSTR)FormatByteSize( file_size ), file_size, (LPCTSTR)FormatByteSize( 0 ).Trim( _T(" 0") ) );
	if ( file_size % pdevice->BytesPerSector != 0 )
	{
		Log( LOG_WARNING, IDS_FILE_SECTOR_SIZE, (LPCTSTR)FormatByteSize( pdevice->BytesPerSector ) );
	}

	// Open device volumes
	std::deque< std::unique_ptr< CDeviceVolume > > volumes;
	for ( const auto& volume_name : pdevice->Volumes )
	{
		auto pvolume = std::make_unique< CDeviceVolume >();
		if ( pvolume->Open( volume_name ) )
		{
			volumes.push_back( std::move( pvolume ) );
		}
	}

	// Lock device volumes
	for ( const auto& volume : volumes )
	{
		volume->Lock();
	}

	// Unmount device volumes
	for ( const auto& volume : volumes )
	{
		volume->Dismount();
	}

	// Open disk
	Log( LOG_ACTION, IDS_DEVICE, (LPCTSTR)pdevice->DeviceID );
	CAtlFile disk;
	hr = disk.Create( pdevice->DeviceID, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0 );
	if ( FAILED( hr ) )
	{
		Log( LOG_ERROR, IDS_DEVICE_MISSING, (LPCTSTR)GetErrorString( hr ) );
		return;
	}

	// Check device size
	Log( LOG_INFO, IDS_DEVICE_SIZE, (LPCTSTR)FormatByteSize( pdevice->Size ), pdevice->Size, (LPCTSTR)FormatByteSize( 0 ).Trim( _T(" 0") ) );
	if ( pdevice->Size < file_size )
	{
		Log( LOG_WARNING, IDS_DEVICE_SIZE_MISMATCH );
	}

	// Write to disk
	Log( LOG_ACTION, IDS_WRITING );
	ULONGLONG leftover = file_size;
	while ( leftover && ! m_bCancel )
	{
		const DWORD to_read = ( leftover > buf_size ) ? buf_size : (DWORD)leftover;
		const DWORD to_write = ( to_read / pdevice->BytesPerSector + ( ( to_read % pdevice->BytesPerSector ) ? 1 : 0 ) ) * pdevice->BytesPerSector;

		memset( buf, 0, to_write );

		hr = file.Read( buf, to_read );
		if ( FAILED( hr ) )
		{
			Log( LOG_ERROR, IDS_FILE_ERROR, (LPCTSTR)GetErrorString( hr ) );
			break;
		}

		DWORD written = 0;
		hr = disk.Write( buf, to_write, &written );
		if ( FAILED( hr ) || written != to_write )
		{
			Log( LOG_ERROR, IDS_DEVICE_ERROR, (LPCTSTR)GetErrorString( hr ) );
			break;
		}

		leftover -= to_read;

		m_wndProgress.PostMessage( PBM_SETPOS, (int)( ( ( file_size - leftover ) * 100 ) / file_size ) );
	}

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

	// Unlock device volumes
	volumes.clear();

	// Update disk
	Log( LOG_ACTION, IDS_DEVICE_UPDATE );
	if ( ! DeviceIoControl( disk, IOCTL_DISK_UPDATE_PROPERTIES, nullptr, 0, nullptr, 0, &returned, nullptr ) )
	{
		Log( LOG_ERROR, IDS_DEVICE_UPDATE_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
	}
}
