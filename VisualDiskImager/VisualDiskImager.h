
// VisualDiskImager.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'pch.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CVisualDiskImagerApp

class CVisualDiskImagerApp : public CWinApp
{
public:
	CVisualDiskImagerApp();

protected:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};

extern CVisualDiskImagerApp theApp;

#define REG_SETTINGS	_T("Settings")
#define REG_IMAGE		_T("Image")
#define REG_DEVICE		_T("Device")
#define REG_VERIFY		_T("Verify")

// Log priority
enum LogPriority { LOG_ERROR = 0, LOG_INFO, LOG_WARNING, LOG_ACTION };

void Log(LogPriority nPriority, LPCTSTR szFormat, ...);
void Log(LogPriority nPriority, UINT nFormat, ...);
void LogV(LogPriority nPriority, LPCTSTR szFormat, va_list ap);

CString LoadString(UINT nID);
CString FormatByteSize(LONGLONG nSize);
CString FormatByteSizeEx(LONGLONG nSize);
CString GetErrorString(DWORD error);


// CVirtualBuffer

class CVirtualBuffer
{
public:
	inline CVirtualBuffer(SIZE_T size) noexcept
		: buf( VirtualAlloc( nullptr, size, MEM_COMMIT, PAGE_READWRITE ) )
	{
	}

	inline ~CVirtualBuffer() noexcept
	{
		VirtualFree( buf, 0, MEM_RELEASE );
	}

	operator LPVOID() const noexcept
	{
		return buf;
	}

private:
	LPVOID buf;
};
