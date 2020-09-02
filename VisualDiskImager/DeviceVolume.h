#pragma once

// CDeviceVolume

class CDeviceVolume
{
public:
	CDeviceVolume() noexcept;
	~CDeviceVolume();

	// Open a volume
	bool Open(LPCTSTR szVolumeName);

	// Lock the volume
	bool Lock();

	// Unlock the volume
	bool Unlock();

	// Dismount the volume
	bool Dismount();

private:
	CString		m_sName;
	CAtlFile	m_Volume;
	bool		m_bLocked;
};
