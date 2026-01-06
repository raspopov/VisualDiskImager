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

#pragma once

#include "DeviceVolume.h"

// CDevice (Name: \\?\Volume{guid} )

class CDevice : public CItem
{
public:
	CDevice() noexcept = default;
	CDevice(const CString & sDeviceID) : CItem( sDeviceID ) {}

	// Initialize device information
	bool Init(IWbemClassObject* disk);

	// Find all volumes of this device (fills Volumes member)
	void GetDeviceVolumes(bool bSilent);

	// Open device
	bool Open(bool bWrite);

	// Update device properties
	bool Update();

	// Eject device
	bool Eject();

	CString				Model;
	CString				Type;
	bool				Writable = false;
	bool				System = false;
	CDeviceVolumes		Volumes;

	inline auto Removable() const noexcept
	{
		return ( Info.Geometry.MediaType != FixedMedia );
	}

	ULONGLONG StartingOffset() const noexcept override
	{
		return 0;
	}

	ULONGLONG Size() const noexcept override
	{
		return Info.DiskSize.QuadPart;
	}

	DWORD BytesPerSector() const noexcept override
	{
		return Info.Geometry.BytesPerSector ? Info.Geometry.BytesPerSector : 512;
	}

	inline auto Cylinders() const noexcept
	{
		return Info.Geometry.Cylinders.QuadPart;
	}

	inline auto TracksPerCylinder() const noexcept
	{
		return Info.Geometry.TracksPerCylinder;
	}

	inline auto SectorsPerTrack() const noexcept
	{
		return Info.Geometry.SectorsPerTrack;
	}

private:
	DISK_GEOMETRY_EX	Info{};
};

using CDevices = std::deque< std::unique_ptr< CDevice > >;
