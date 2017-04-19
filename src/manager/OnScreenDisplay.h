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
*    Jochen Baier, 2007, 2008, 2009 (email@Jochen-Baier.de)
*
*/

#pragma once

#include "..\common\stdafx.h"

class OnScreenDisplay
{
public:
  void OnScreenDisplay::ShowDesktopNumber(int number);
  OnScreenDisplay();
  ~OnScreenDisplay();
  void OnScreenDisplay::HideOSD(void);

private:
  int switch_number;
  int timer_count;

  void OnScreenDisplay::MouseUp(void);

  void OnPaint(void);
  static LRESULT CALLBACK OnScreenDisplay::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  LRESULT OnScreenDisplay::WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  HWND m_hWnd;

};
