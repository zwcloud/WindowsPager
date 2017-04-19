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
#include "Window.h"

class Desktop
{
public:
  Desktop(int _desk_x);
  ~Desktop();
  void UpdateIgnoreStuff(void);
  void DrawFlash(Gdiplus::Graphics *g);
  HICON GetTrayIcon(void);
  void ChangeWallpaper(void);
  std::wstring GetWallpaperPath(void);
  void ManageNotification(void);
  void StopFlash(void);
  void RestoreWindowsBeforeExitOnCurrent(void); 
  void RestoreWindowsBeforeExitOnHidden(void);
  void UpdateGeo(void);
  void UpdateMiniWindows(void);
  void CheckForStillVisibleWindows(void);
  bool IsButton(HWND window);
  HWND GetButtonByHWND(HWND window);
  bool MaybeAddWindow(HWND window, bool hide=false);
  void WindowActivated(Window * window);
  void ButtonActivated(Window * window);
  void AddButton(HWND window);
  void RemoveButton(HWND window);
  void DrawDesktop(Gdiplus::Graphics *g);
  int GetDesktopNumber();
  void AddToplevelWindows();
  Window *GetMiniWindowByPoint(POINT pt);
  void DrawMiniWindows(Gdiplus::Graphics *g);
  Window *GetWindowByHWND(HWND id);
  bool PointIsInside(POINT pt);
  void Show();
  void Hide(void);
  void AddWindow(Window * window);
  void RemoveWindow(Window * window, bool deleteit);
  void SetActiveWindow(Window * window);
  RECT GetCellRect();
  RECT GetDesktopRect();
  static int CALLBACK FlashTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime);
  bool GetFlashStatus(void);
  void SetFlashStatus(bool new_status);

private:

  void MarkInjectedWindows(HWND win);
  bool AddHiddenGedoens(HWND window, HWND owner, RECT real_rect);
  void AddToplevelWindowsWin7(void);
  void AddToplevelWindowsNotTasklistVistaXP(void);
  void AddToplevelWindowsFromTasklistVistaXP();
  void AddNewTaskBarWindow(HWND window, bool hide=false);
  void HideTaskbarButtons(void);
  void ShowTaskbarButtons(void);
  void DrawVista(Gdiplus::Graphics *g);
  void DrawWin2000(Gdiplus::Graphics *g);

  HICON tray_icon;
  bool flash_status;
  std::wstring wallpaper_path;
  int preview_row;
  int preview_column;
  UINT_PTR flash_timer_id;
  bool flashing;
  ATOM hotkeyatom;
  ATOM hotkeyatom_move_to;
  int number;
  RECT desk_rect;
  RECT cell_rect;
  RECT mouse_rect;
  std::vector < Window * >list_zorder;
  std::vector < HWND > list_torder;

  HWND activeWindow;


};
