// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "VisualDiskImager.h"
#include "Device.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDevice

CDevice::CDevice(LPCTSTR szDeviceID)
	: Name			( szDeviceID )
	, DiskSize		( 0 )
	, BytesPerSector( 0 )
	, Writable		( false )
	, System		( false )
	, Removable		( false )
{
}

CDevice::~CDevice()
{
}

bool CDevice::Init(IWbemClassObject* disk)
{
	CComVariant id, model, type, size, sector, capabilities;
	if ( SUCCEEDED( disk->Get( L"DeviceID", 0, &id, 0, 0 ) ) &&
		 SUCCEEDED( disk->Get( L"Model", 0, &model, 0, 0 ) ) &&
		 SUCCEEDED( disk->Get( L"InterfaceType", 0, &type, 0, 0 ) ) &&
		 SUCCEEDED( disk->Get( L"BytesPerSector", 0, &sector, 0, 0 ) ) &&
		 SUCCEEDED( disk->Get( L"Size", 0, &size, 0, 0 ) ) &&
		 SUCCEEDED( disk->Get( L"Capabilities", 0, &capabilities, 0, 0 ) ) )
	{
		VERIFY( SUCCEEDED( id.ChangeType( VT_BSTR ) ) );
		Name = id;

		VERIFY( SUCCEEDED( model.ChangeType( VT_BSTR ) ) );
		Model = model;

		VERIFY( SUCCEEDED( type.ChangeType( VT_BSTR ) ) );
		Type = type;

		VERIFY( SUCCEEDED( size.ChangeType( VT_UI8 ) ) );
		DiskSize = size.ullVal;

		VERIFY( SUCCEEDED( sector.ChangeType( VT_UI4 ) ) );
		BytesPerSector = sector.ulVal;

		bool removable = false;
		if ( capabilities.vt == ( VT_ARRAY | VT_I4 ) )
		{
			CComSafeArray< LONG > caps;
			if ( SUCCEEDED( caps.Attach( capabilities.parray ) ) )
			{
				const ULONG count = caps.GetCount();
				for ( ULONG i = 0; i < count; ++i )
				{
					switch ( caps.GetAt( i ) )
					{
					case 4:
						Writable = true;
						break;

					case 7:
						Removable = true;
						break;
					}
				}
			}
			caps.Detach();
		}

		if ( DiskSize == 0 || BytesPerSector == 0 )
		{
			Writable = false;
		}

		return true;
	}
	return false;
}

void CDevice::GetDeviceVolumes(bool bSilent)
{
	if ( ! bSilent ) Log( LOG_ACTION, IDS_ENUM_VOLUME, (LPCTSTR) Name );

	Volumes.clear();

	// Windows system directory
	TCHAR szSystem[ MAX_PATH ] = {};
	GetSystemDirectory( szSystem, MAX_PATH );

	// Looking for volumes on disk
	TCHAR szVolumeName[ MAX_PATH ] = {};
	HANDLE hFindVolume = FindFirstVolume( szVolumeName, MAX_PATH );
	if ( hFindVolume != INVALID_HANDLE_VALUE )
	{
		TCHAR buf[ 1024 ];
		DWORD returned;

		do
		{
			// Volume name without ending slash
			auto volume = std::make_unique< CDeviceVolume >( CString( szVolumeName ).TrimRight( _T('\\') ) );

			bool bSystemVolume = false;

			// Get disk DOS names
			if ( GetVolumePathNamesForVolumeName( szVolumeName, buf, _countof( buf ), &returned ) )
			{
				size_t name_len;
				for ( LPCTSTR name = buf; *name; name += name_len )
				{
					name_len = _tcslen( name );

					volume->PathNames.push_back( name );

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
				if ( DeviceIoControl( volume_file, IOCTL_VOLUME_GET_VOLUME_DISK_EXTENTS, nullptr, 0, buf, sizeof( buf ), &returned, nullptr ) )
				{
					PVOLUME_DISK_EXTENTS extents = reinterpret_cast< PVOLUME_DISK_EXTENTS >( buf );
					for ( DWORD extent = 0; extent < extents->NumberOfDiskExtents; ++ extent )
					{
						CString device_id;
						device_id.Format( _T("\\\\.\\PHYSICALDRIVE%u"), extents->Extents[ extent ].DiskNumber );

						// Add volume to owner device
						if ( StrCmpI( device_id, Name ) == 0 )
						{
							if ( bSystemVolume )
							{
								System = true;
							}

							if ( volume->PathNames.empty() )
							{
								if ( ! bSilent ) Log( LOG_INFO, IDS_VOLUME_INFO, (LPCTSTR)volume->Name );
							}
							else
							{
								CString sVolumePathName = volume->Name;
								sVolumePathName += _T(" -> ");
								for ( const auto& path : volume->PathNames )
								{
									sVolumePathName += path + _T(' ');
								}
								if ( ! bSilent ) Log( LOG_INFO, IDS_VOLUME_INFO, (LPCTSTR)sVolumePathName );
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

	if ( m_h == NULL )
	{
		HRESULT hr = CAtlFile::Create( Name, ( bWrite ? GENERIC_WRITE : 0 ) | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, OPEN_EXISTING, 0 );
		if ( FAILED( hr ) )
		{
			Log( LOG_ERROR, IDS_DEVICE_MISSING, (LPCTSTR)GetErrorString( hr ) );
			return false;;
		}

		char buf [ 256 ] = {};
		DWORD returned;
		if ( DeviceIoControl( m_h, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, nullptr, 0, buf, sizeof( buf ), &returned, nullptr ) )
		{
			const PDISK_GEOMETRY_EX geo = reinterpret_cast< PDISK_GEOMETRY_EX  >( buf );
			BytesPerSector = geo->Geometry.BytesPerSector;
			DiskSize = geo->DiskSize.QuadPart;
		}
		else
		{
			Log( LOG_WARNING, IDS_DEVICE_INFO_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
		}

		if ( DeviceIoControl( m_h, IOCTL_DISK_IS_WRITABLE, nullptr, 0, buf, sizeof( buf ), &returned, nullptr ) )
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
	if ( m_h != NULL )
	{
		Log( LOG_ACTION, IDS_DEVICE_UPDATE );

		VERIFY( SUCCEEDED ( CAtlFile::Flush() ) );

		DWORD returned;
		if ( ! DeviceIoControl( m_h, IOCTL_DISK_UPDATE_PROPERTIES, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Log( LOG_ERROR, IDS_DEVICE_UPDATE_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
			return false;
		}
		return true;
	}
	return false;
}

bool CDevice::Eject()
{
	if ( m_h != NULL )
	{
		Log( LOG_ACTION, IDS_DEVICE_EJECT );

		DWORD returned;
		if ( ! DeviceIoControl( m_h, IOCTL_STORAGE_EJECT_MEDIA, nullptr, 0, nullptr, 0, &returned, nullptr ) )
		{
			Log( LOG_ERROR, IDS_DEVICE_EJECT_ERROR, (LPCTSTR)GetErrorString( GetLastError() ) );
			return false;
		}
		return true;
	}
	return false;
}
