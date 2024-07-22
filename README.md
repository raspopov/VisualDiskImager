[![Build status](https://ci.appveyor.com/api/projects/status/wdo1lp9i82c2d552?svg=true)](https://ci.appveyor.com/project/raspopov/visualdiskimager)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/0d8a5451f6774b568698a26a4691d15b)](https://www.codacy.com/manual/raspopov/VisualDiskImager?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=raspopov/VisualDiskImager&amp;utm_campaign=Badge_Grade)
[![GitHub All Releases](https://img.shields.io/github/downloads/raspopov/VisualDiskImager/total)](https://github.com/raspopov/VisualDiskImager/releases)
[![GitHub release (latest by date)](https://img.shields.io/github/v/release/raspopov/VisualDiskImager)](https://github.com/raspopov/VisualDiskImager/releases)

# Visual Disk Imager

A handy Windows tool for writing a disk images to the physical devices (HDD, SSD, USB sticks or SD/CF cards).

Inspired by minimalism of [Win32 Disk Imager](https://sourceforge.net/projects/win32diskimager/) and
overwhelming feature richness of [BOOTICE](https://www.google.com/search?q=bootice). :sunglasses:

## Features

 - Writing a [disk image file (*.IMG)](https://en.wikipedia.org/wiki/IMG_(file_format)) to the Windows physical device i.e. to "\\\\.\\PHYSICALDRIVEn" (where "n" is a zero-based disk number). 
 - Verification after writing.
 - A comprehensive log (supporting copy to clipboard).
 - Device change autodetection.
 - Drag-n-drop.

![Visual Disk Imager](https://raw.githubusercontent.com/raspopov/VisualDiskImager/master/VisualDiskImager.png)

## System Requirements

 - Windows XP or later, 32 or 64-bit.
 - Administrator rights.

## Development Requirements

 - [Microsoft Visual Studio 2022 Community](https://aka.ms/vs/17/release/vs_Community.exe) with components:
   - Microsoft.VisualStudio.Workload.NativeDesktop
   - Microsoft.VisualStudio.Component.VC.ATLMFC
   - Microsoft.VisualStudio.Component.WinXP
   - Microsoft.VisualStudio.ComponentGroup.NativeDesktop.WinXP
   - Microsoft.VisualStudio.Component.NuGet

## License

Copyright (C) 2020-2024 Nikolay Raspopov <<raspopov@cherubicsoft.com>>

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
( at your option ) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
