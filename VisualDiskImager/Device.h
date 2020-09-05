/*
This file is part of Visual Disk Imager

Copyright (C) 2020 Nikolay Raspopov <raspopov@cherubicsoft.com>

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
