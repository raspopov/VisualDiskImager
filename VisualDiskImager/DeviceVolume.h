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
