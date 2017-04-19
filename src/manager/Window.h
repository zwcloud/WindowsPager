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
#include <time.h>

#include "HangUpInfoWindow.h"

class Desktop;

class Window
{

public:
  bool Window::RunningInsideVisualStudio(void);
  void SystemMenuReset(void);
  Window(Desktop * _desktop, HWND _real_window, bool tasklist_window);
  ~Window();
  bool IsConsoleWindow(void);
  bool GetWmExTransparent(void);
  std::wstring *GetClassString(void);
  std::wstring *GetFileName(void);
  void Window::RemoveSubclassOrHook(void);
  bool SizeEvent(void);
  void ResetObservationTimer(void);
  void RestoreOnExitSticky(void);
  bool RemoveNoactivateStyle(void);
  void ShowOnExit();
  void RestoreOnExit(void);
  void Hide(void);
  void Show(bool observation, bool noAsync=false, bool activate=false);
  void SystemMenuUpdate(void);
  void UpdateAlwaysVisible(bool checked);
  Desktop *GetHoverDesktop(void);
  void SetHoverDesktop(Desktop *hover);
  void SetWindowPosFake(bool set_flag=true);
  bool SetExStyle(LONG_PTR new_style, int timeout);
  bool SetExStyleEx(LONG_PTR old_style_ex, LONG_PTR new_style, int timeout);
  bool RemoveExStyle(LONG_PTR remove_style, int timeout);
  bool RemoveDisabled(int timeout);
  bool SetDisabled(int timeout);
  void RestoreFromDuringHide(void);
  void RestoreFromDuringHideToHidden();
  void RestoreFromDuringHideToCurrent();
  void RestoreFromDuringShow(void);
  void SetDummyWindowNULL(void);
  HangUpInfoWindow* GetDummyWindow(void);
  bool WasDuringHide(void);
  void IncreaseHangupCount(void);
  int GetHangupCount(void);
  void ResetHangupCount(void);
  void StartObservation(bool during_hide);
  bool DeathCheckPositiv(void);
  bool IsDead(void);
  void SetDeath(void);
  void ResetFlashing(void);
  void ProcedeNcActivateEvent(void);
  void SetFlashTimerID(UINT id);
  int GetNcEvents(void);
  void SetCountingFalse(void);
  void SetNcEvents(int i);
  void SetBackupRect(RECT new_one);
  RECT GetBackupRect(void);
  void DrawMiniWindow(Gdiplus::Graphics *g, int x, int y, bool sticky);
  void UpdateMiniWindow(void);
  void DeleteFlag(char *key);
  bool HasFlag(char *key);
  void SetFlag(char *key);
  HICON GetSmallIcon(void);
  void SetSmallIcon(HICON icon);
  void DragOff();
  void DragOn();
  bool WasDragged();
  Desktop *GetDesktop();
  void SetPadding(POINT pt);
  POINT GetPadding();
  void SetMiniWindowRect(RECT & rect);
  RECT GetMiniWindowRect();
  void SetDesktop(Desktop * desktop);
  void UpdateGeo(void);
  HWND GetRealWindowHWND();
  bool MiniWindowStartDragging(RECT &main_window_rect, Desktop* current_desktop, int desktop_padding_horizontal, int desktop_padding_vertical);
  void MiniWindowMoving(Desktop *current_desktop, Desktop * _hover_desktop, POINT &pt);
  void MiniWindowDragStop(Desktop* current_desktop);
  void SetHidden(void);
  void DeleteHidden(void);
  void SetHiddenStyles(bool withfakepos=true);
  void RemoveHiddenStyles(bool withfakepos=true);
  
private:
  void HideConsoleWindow(void);
  void ShowConsoleWindow(void);
  void MiniWindowMoveOnForeignDesktop( int new_x, int new_y );
  void MiniWindowMoveOnCurrentDesktop( int new_x, int new_y );
  void PrimaryToSecondary(void);
  void SecondaryToPrimary(void);
  void CreateMiniWindowBitmap(int mini_width , int mini_height);
  void _UpdateMiniWindow(bool use_rect, RECT rect, bool accept_hidden);
  static int CALLBACK StopCounting(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime);    
  static int CALLBACK ObservationTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime);           
  RECT RealRectToMini(RECT rect_real);
  void DocsCallsNcLeftButtonDown( WPARAM wParam, HWND hWnd, LPARAM lParam );
 
  void InjectHookDll(void);
  bool Is64BitProcess(HANDLE hProcess);

  bool WpagerApiHookAlreadInjected ( HANDLE hProcess, bool is64Bit);

  static int CALLBACK MenuTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime);
  
  void SystemMenuInitTrigger(void);
  
    
  bool console_window;
  bool wm_ex_transparent;
  bool explorer;
  std::wstring *class_string;
  UINT_PTR observation_timer;
  Desktop *hover_desktop;
  std::wstring *filename;
  HangUpInfoWindow *dummy_window;
  bool duringhide;
  int hangup_count;
  bool dead;
  int nc_events;
  bool counting_events;
  UINT_PTR flash_timer_id;
  RECT backup_rect;
  HWND real_window;
  RECT mini_window_rect;
  RECT mini_rect_start;
  POINT padding;
  bool dragged;
   Desktop *desktop;
  HICON smallicon;
  std::map < std::string, int >flags;
  HBITMAP mini_window_bitmap;
  RECT mini_bitmap_rect;
  HWND window_icon;
  bool runningInsideVisualStudio;
  bool useToolWindowFlag;
};
