#pragma once

// CDevice

class CDevice
{
public:
	CDevice() noexcept;

	// Initialize device information from WMI
	bool Init(IWbemClassObject* disk);

	inline operator bool() const
	{
		return ( DeviceID.IsEmpty() == FALSE );
	}

	CString		DeviceID;
	CString		Model;
	CString		Type;
	ULONGLONG	Size;
	DWORD		BytesPerSector;
	bool		Writable;
	bool		Removable;
	std::deque< CString >	Volumes;
};
