/*
This file is part of Visual Disk Imager

Copyright (C) 2020-2024 Nikolay Raspopov <raspopov@cherubicsoft.com>

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

#include "Item.h"

// CDeviceVolume (Name: \\?\Volume{guid} )

class CDeviceVolume : public CItem
{
public:
	CDeviceVolume(CString sVolumeName = CString()) : CItem( sVolumeName ) {}
	virtual ~CDeviceVolume();

	// Initialize device volume information
	bool Init(const CItem * device, const DISK_EXTENT & extent);

	// Open a volume
	bool Open();

	// Open and lock the volume
	bool Lock();

	// Unlock and close the volume
	bool Unlock();

	// Dismount the volume
	bool Dismount();

	LONGLONG StartingOffset() const noexcept override
	{
		return Start;
	}

	LONGLONG Size() const noexcept override
	{
		return Length;
	}

	DWORD BytesPerSector() const noexcept override
	{
		return Parent->BytesPerSector();
	}

	CString Paths() const;

	std::deque< CString >	PathNames;	// C:\, ...

private:
	bool					m_bLocked = false;
	LONGLONG				Start = 0;	// sectors
	LONGLONG				Length = 0;	// bytes
};

using CDeviceVolumes = std::deque< std::unique_ptr< CDeviceVolume > >;
