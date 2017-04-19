/*
* GPL Notice:
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; version 2 of the License.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU Library General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*
*
* Name:
*
*    WindowsPager
*
*
* Copyright:
*
*    Jochen Baier, 2007, 2008, 2009, 2010, 2011 (email@Jochen-Baier.de)
*
*/

#pragma once

#ifdef __cplusplus
#define EXPORT extern "C" __declspec (dllexport)
#else
#define EXPORT __declspec (dllexport)
#endif

#include <windows.h>

#ifdef _M_X64
EXPORT bool InitHooksDll64(HWND window, HWND tray_window,
                         HWND task_window,
                         HWND list_window, UINT message, int _numDesktops, int _moveAndSwitchModifier);
EXPORT bool UnInstallFilters64();
#else
EXPORT bool InitHooksDll32(HWND window, HWND tray_window,
                         HWND task_window,
                         HWND list_window, UINT message, int _numDesktops, int _moveAndSwitchModifier);
EXPORT bool UnInstallFilters32();
#endif


