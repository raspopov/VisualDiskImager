# Visual Disk Imager

A handy Windows tool for writing a disk images to the physical devices (HDD, SSD, USB sticks or SD/CF cards).

Inspired by minimalism of [Win32 Disk Imager](https://sourceforge.net/projects/win32diskimager/) and
overwhelming feature richness of [BOOTICE](https://www.google.com/search?q=bootice). :sunglasses:

## Features

 - Writing a [disk image file (*.IMG)](https://en.wikipedia.org/wiki/IMG_(file_format)) to the Windows physical device i.e. to "\\\\.\\PHYSICALDRIVEn" (where "n" is a zero-based disk number). 
 - Verification after writing.
 - A full log supporting copy to clipboard.
 - Device change autodetection.
 - Drag-n-drop.

![Visual Disk Imager](https://raw.githubusercontent.com/raspopov/VisualDiskImager/master/VisualDiskImager.png)

## System Requirements

 - Windows XP or later, 32 or 64-bit.
 - Microsoft Visual C++ 2015-2019 Redistributables ([32-bit](https://aka.ms/vs/15/release/VC_redist.x86.exe)/[64-bit](https://aka.ms/vs/15/release/VC_redist.x64.exe)).
 - Administrator rights.

## Development Requirements

 - [Microsoft Visual Studio 2017 Community](https://aka.ms/vs/15/release/vs_Community.exe) with components:
   - Microsoft.VisualStudio.Workload.NativeDesktop
   - Microsoft.VisualStudio.Component.VC.ATLMFC
   - Microsoft.VisualStudio.Component.WinXP
   - Microsoft.VisualStudio.ComponentGroup.NativeDesktop.WinXP
  
