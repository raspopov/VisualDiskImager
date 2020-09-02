// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "pch.h"
#include "Device.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDevice

CDevice::CDevice() noexcept
	: Size			( 0 )
	, BytesPerSector( 0 )
	, Writable		( false )
	, Removable		( false )
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
		DeviceID = id;

		VERIFY( SUCCEEDED( model.ChangeType( VT_BSTR ) ) );
		Model = model;

		VERIFY( SUCCEEDED( type.ChangeType( VT_BSTR ) ) );
		Type = type;

		VERIFY( SUCCEEDED( size.ChangeType( VT_UI8 ) ) );
		Size = size.ullVal;

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

		if ( Size == 0 || BytesPerSector == 0 )
		{
			Writable = false;
		}

		return true;
	}
	return false;
}
