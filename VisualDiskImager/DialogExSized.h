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

#include "Sized.h"

// CDialogExSized dialog

class CDialogExSized : public CDialogEx, public CSized< CDialogExSized >
{
	DECLARE_DYNAMIC(CDialogExSized)

public:
	CDialogExSized(UINT nIDTemplate, CWnd *pParent = nullptr) : CDialogEx( nIDTemplate, pParent ) {}
	CDialogExSized(LPCTSTR lpszTemplateName, CWnd *pParentWnd = nullptr) : CDialogEx( lpszTemplateName, pParentWnd ) {}

	void ReloadLayout()
	{
		ASSERT( ! IsIconic() );

		VERIFY( LoadDynamicLayoutResource( m_lpszTemplateName ) );

		if ( auto pLayout = GetDynamicLayout() )
		{
			pLayout->SetMinSize( m_sizeMinClient );
		}
	}

protected:
	BOOL OnInitDialog() override;

	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnDestroy();

	DECLARE_MESSAGE_MAP()
};
