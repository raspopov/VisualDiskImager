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

class CItem : public CAtlFile
{
public:
	CItem(CString name = CString()) : Name( name ) {}

	CString			Name;
	const CItem *	Parent = nullptr;

	virtual LONGLONG StartingOffset() const noexcept = 0;	// sectors
	virtual LONGLONG Size() const noexcept = 0;				// bytes
	virtual DWORD BytesPerSector() const noexcept = 0;		// bytes

	const CItem * Top() const noexcept
	{
		return Parent ? Parent->Top() : this;
	}
};
