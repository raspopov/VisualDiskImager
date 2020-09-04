#pragma once

#include "DeviceVolume.h"

// CDevice

class CDevice : public CAtlFile
{
public:
	CDevice(LPCTSTR szDeviceID = _T(""));
	~CDevice();

	// Initialize device information from WMI
	bool Init(IWbemClassObject* disk);

	// Find all volumes of this device (fills Volumes member)
	void GetDeviceVolumes(bool bSilent);

	// Open device
	bool Open(bool bWrite);

	// Update device properties
	bool Update();

	// Eject device
	bool Eject();

	CString				Name;
	CString				Model;
	CString				Type;
	DISK_GEOMETRY_EX	Info;
	bool				Writable;
	bool				Removable;
	bool				System;
	CDeviceVolumes		Volumes;

private:
	CDevice(const CDevice&) = delete;
	CDevice& operator=(const CDevice&) = delete;
};

typedef std::deque< std::unique_ptr< CDevice > > CDevices;
