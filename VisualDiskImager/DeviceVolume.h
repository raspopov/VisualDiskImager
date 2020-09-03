#pragma once

// CDeviceVolume

class CDeviceVolume : public CAtlFile
{
public:
	CDeviceVolume(LPCTSTR szVolumeName = _T(""));
	~CDeviceVolume();

	// Open a volume
	bool Open();

	// Open and lock the volume
	bool Lock();

	// Unlock and close the volume
	bool Unlock();

	// Dismount the volume
	bool Dismount();

	CString					Name;
	std::deque< CString >	PathNames;

private:
	bool					m_bLocked;

	CDeviceVolume(const CDeviceVolume&) = delete;
	CDeviceVolume& operator=(const CDeviceVolume&) = delete;
};

typedef std::deque< std::unique_ptr< CDeviceVolume > > CDeviceVolumes;
