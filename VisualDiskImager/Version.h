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

#include "Build.h"

#define MAJOR_VERSION	1
#define MINOR_VERSION	3
#define PATCH_VERSION	0

#define STR_VALUE(arg)    		#arg
#define STR_NAME(name)    		STR_VALUE(name)

#define FILE_VERSION			MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION, BUILD_VERSION
#define STRING_FILE_VERSION		STR_NAME(MAJOR_VERSION) "." STR_NAME(MINOR_VERSION) "." STR_NAME(PATCH_VERSION) "." STR_NAME(BUILD_VERSION)

#define PRODUCT_VERSION			FILE_VERSION
#define STRING_PRODUCT_VERSION	STRING_FILE_VERSION
