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
*    Jochen Baier, 2009, 2010, 2011  (email@Jochen-Baier.de)
*
*/

#pragma once

#include "..\common\stdafx.h"

class WindowSelector  
{
public:
 WindowSelector();
 ~WindowSelector();

private:
  void InvalidateTheShit(HWND hwnd, RECT rect);
  int LButtonDown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  int MouseMove(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  int LButtonUp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  void GetAllWindows(void);
  bool RectInRect(const RECT& big_rect, const RECT& small_rect);
  void AddToInIWin(void);
  void AddToInIProcess(void);
  void onPaint(HWND hwnd, Gdiplus::Graphics& graphics);
  LRESULT WindowCalls (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

  Gdiplus::Image *add_button_win;
  Gdiplus::Image *add_button_proc;
  Gdiplus::Image *exit_button;
  Gdiplus::Image *help_button;
  Gdiplus::Bitmap *bmp;
  std::vector<std::pair<HWND, RECT>> windows;
  int move_startx;
  int move_starty;
  bool moving;
  bool resizing1;
  bool resizing2;
  bool in_move_box;
  bool in_resizing1_box;
  bool in_resizing2_box;
  bool add_pressed_proc;
  bool add_pressed_win;
  bool help_pressed;
  bool exit_pressed;
  RECT selection;  
  RECT move_start_rect;
  HCURSOR hand_cursor;
  HWND main_win;
  bool did_something;
  WNDCLASS wc;
};