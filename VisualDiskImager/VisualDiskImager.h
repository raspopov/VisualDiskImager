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

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols

// CVisualDiskImagerApp

class CVisualDiskImagerApp : public CWinApp
{
protected:
	BOOL InitInstance() override;

	DECLARE_MESSAGE_MAP()
};

extern CVisualDiskImagerApp theApp;

#define REG_SETTINGS	_T("Settings")
#define REG_IMAGE		_T("Image")
#define REG_DEVICE		_T("Device")
#define REG_VERIFY		_T("Verify")
#define REG_OFFSET		_T("Offset")

// Log priority
enum LogPriority { LOG_ERROR = 0, LOG_INFO, LOG_WARNING, LOG_ACTION, LOG_DEVICE, LOG_VOLUME };

void Log(LogPriority nPriority, LPCTSTR szFormat, ...);
void Log(LogPriority nPriority, UINT nFormat, ...);
void LogV(LogPriority nPriority, LPCTSTR szFormat, va_list ap);

CString LoadString(UINT nID);
CString FormatByteSize(LONGLONG nSize);
CString FormatByteSizeEx(LONGLONG nSize);
CString GetErrorString(DWORD error);


// CVirtualBuffer

template< typename T >
class CVirtualBuffer
{
public:
	inline CVirtualBuffer(SIZE_T count) noexcept
		: buf( VirtualAlloc( nullptr, count * sizeof( T ), MEM_COMMIT, PAGE_READWRITE ) )
	{
	}

	inline ~CVirtualBuffer() noexcept
	{
		VirtualFree( buf, 0, MEM_RELEASE );
	}

	operator T*() const noexcept
	{
		return static_cast< T* >( buf );
	}

private:
	LPVOID buf;
};
