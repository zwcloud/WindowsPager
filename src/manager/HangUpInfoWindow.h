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
*    Jochen Baier, 2007, 2008, 2009, 2010, 2011  (email@Jochen-Baier.de)
*
*/

#pragma once


#include "..\common\stdafx.h"
#include "System.h"

#define dummy_width 310
#define dummy_height 65

class HangUpInfoWindow
{
public:
  HangUpInfoWindow(HWND bad_win, HICON &smallicon, bool duringhide);
  ~HangUpInfoWindow(void);
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  void Update(void);
  void DieSlowly(void);
  void Show(void);

private:
  void OnPaint(void);
  LRESULT WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

  bool visible;
  std::string *title;
  std::wstring *title_w;
  bool unicode;
  RECT _close_rect;
  HICON _close_icon;
  HFONT m_hFont;
  UINT _dummyWindowCloseMessage;
  bool _duringhide;
  bool _isdeath;
  HICON _icon;
  HWND _bad_win;
  int _seconds;
  HWND m_hWnd;
};
