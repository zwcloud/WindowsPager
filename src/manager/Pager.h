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

#include <ole2.h>   
#include "Desktop.h"
#include "DropTarget.h"
#include "Ini.h"
#include "SimpleTray.h"

#define GetPager (Pager::Instance())

class Pager
{
public:
  ~Pager();

  void Pager::SetExplorerPID(DWORD pid);
  
  DWORD Pager::GetExplorerPID(void);
  void Pager::DeletePID(DWORD pid);
  bool Pager::PIDKnown(DWORD pid);
  void Pager::AddPID(DWORD pid);

  bool Pager::RememberActiveWindow(void);

  void RestoreWindowsBeforeExit(bool closeWindows=false);
  bool CheckIfOnIgnoreList(const std::wstring& exe_string, const std::wstring& class_string);
  bool StickyWindowsHaveButtons(void);
  ATOM GetHookAtom(void);
  bool UseHungUpInfoWindow(void);
  bool UseIncognitoMode(void);
  void ResetListwindow(void);
  bool InitReady(void);
  void Key2ValueInsert(UINT_PTR key, UINT_PTR value);
  UINT_PTR Key2ValueGetValue(UINT_PTR id);
  void Key2ValueDelete(UINT_PTR key);
  UINT_PTR CreateUniqueTimerID(void);
  void SetDragTimerDesktop(int desk);
  int GetDragTimerDesktop(void);
  void GetRowandColumnbyNum(int num, int& row, int& col);
  int GetSpace(void);;
  void PanelRectToScreenRect(RECT & rect);
  Window *GetStickyWindowByHWND(HWND hwnd);
  UINT GetShellMessageID();
  void DeleteButtonAllDesks(HWND win);
  void WindowHideEvent(HWND win);
  void WindowSizeEvent(HWND win);
  void ToggleStickyEvent(HWND win);

  void StickyToNormal( Window * window );

  void NormalToSticky( Window * window );

  void MoveWorkspaceEvent(HWND win, int new_desk, bool switchDesktop);
  bool IsBadWindow(HWND window);
  void WindowCreateEvent(HWND win);
  void WindowDestroyEvent(HWND win);
  int GetNumberOfDesktops();
  HWND GetButtonHWNDAllDesks(HWND hwnd);
  Window *GetWindowByHWNDAllDesks(HWND hwnd);
  void SetCurrentDesktop(Desktop * desktop);
  std::vector < Desktop * >GetDesktopList(void);
  Desktop *CurrentDesktop(void);
  Desktop *GetDesktopFromPoint(POINT pt);
  void DesktopToPager(Desktop * desktop, RECT & rect);
  void PagerToDesktop(Desktop * desktop, RECT & rect);
  HWND GetPagerWindowHWND();
  UINT GetHookMessage();
  int GetCellHeight(void);
  int GetCellWidth(void);
  int GetDesktopHeight(void);
  int GetDesktopWidth(void);
  int GetPagerWidth(void);
  int GetPagerHeight(void);
  int GetCols(void);
  int GetRows(void);
  int GetPagerPaddingHorizontal(void);
  int GetPagerPaddingVertical(void);
  void UnregisterHotKeys();
  void Init();
  void RedrawWindow();
  static Pager *Instance();
  void SwitchToDesktop(Desktop * new_desk);
  
  int CrashReportingOption(void);
  

protected:
  Pager(){};
  Pager(const Pager & other){}

private:

  void CreatePagerMessageWindow(void);
  

  static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  static LRESULT CALLBACK WndProcMessageWindow(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);

  static DWORD WINAPI Watchdog(LPVOID lpParam);
  static int CALLBACK DragTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime); 
  static int CALLBACK DelayTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime);

  void WindowSelectorReady(void);
  void AddStickyWindow(Window * window);
  void RemoveStickyWindow(Window * window);
  LRESULT HookCalls(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  LRESULT MessageCalls(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  void MessageCommand( WPARAM wParam );
  void MessageContextMenu( LPARAM lParam, HWND hWnd );
  void MessageMouseMove( LPARAM lParam, WPARAM wParam );
  void MessageLeftMouseUp( LPARAM lParam );
  void MessageLeftMouseDown( LPARAM lParam, WPARAM wParam );
  void MessagePaint();
  void MessageTrayNotify( LPARAM lParam );
  void MessageAdobeShit( HWND win );
  LRESULT ShellCalls(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam);
  void WindowActivateEvent( LPARAM lParam );
  void WindowActivateEvent(Window *window);
  void OnPaint(Gdiplus::Graphics *g);
  void MouseDown(POINT pt);
  void MouseUpNoMiniWindow(POINT pt);
  void StartMiniWindowDragging(Window *clicked_window);
  void MouseUpWithMiniWindow(void);
  void MouseMoveWithMiniWindow(POINT pt);
  void MouseMoveNoMiniWindow(POINT pt);
  void UpdateIgnoreWindows(void);
  void InitINIStuff(void);
  void InitBadClassList();
  void StartHookStarter(void);
  void StartHookStarter32Bit();
  void StartHookStarter64Bit();
  void Init2(HWND _main_window);
  void InitRightClickMenu();
  void Init3(void);
  void MoveWindowFromToDesktop(Desktop *current_desktop, Desktop *new_desktop, Window *window);
  void HotkeyPressed(WPARAM wParam);
  void UninitializeStickyList(void);
  int GetAboveDesktopNumber(int current_number); 
  int GetRightDesktopNumber(int current_number);
  int GetLeftDesktopNumber(int current_number);
  int GetBelowDesktopNumber(int current_number);
  void InitHotkeysDesktopScrolling(void);
  void InitHotkeysMoveToDesktop(void);
  void ShowHelpDialog(void);
  bool WindowShowEvent(HWND win);
  void NcActivateEvent(HWND win);
  void UpdateStickyWindows();

  void UpdatePagerWindowPosHorizontalTaskbar(bool called_from_init, bool pagertoo);
  void UpdatePagerWindowPosVerticalTaskbar(bool called_from_init, bool pagertoo);


  void DrawStickyWindows(Gdiplus::Graphics *g);
  void SetActiveStickyWindow(Window * window);
  void Init1Point5(void);
     
  void UpdateToolbars(int panelHeight);
  static BOOL CALLBACK Pager::DoSomethingHelper(HWND hwnd, LPARAM lParam);
    

  int start_delay;
  static Pager *_instance;
  std::map<std::wstring, std::set<std::wstring>> ignore_map;
  bool maybe_window_destroyed;
  bool clicked_mini_window_destroyed;
  ATOM hookatom;
  HCURSOR hand_cursor;
  UINT_PTR drag_timer_id;
  std::vector<std::wstring> bad_classes;
  ATOM hotkeyatom_left;
  ATOM hotkeyatom_right;
  ATOM hotkeyatom_up;
  ATOM hotkeyatom_down;
  ATOM hotkeyatom_move_left;
  ATOM hotkeyatom_move_right;
  ATOM hotkeyatom_move_up;
  ATOM hotkeyatom_move_down;
  CSimpleTray *tray;
  bool incognito_mode;
  bool draw_sticky_windows;
  bool sticky_window_button;
  bool hung_up_information_window;
  bool rememberActiveWindow;
  bool init_ready;
  int _rows;
  int _cols;
  std::map <UINT_PTR, UINT_PTR> key2value;
  UINT_PTR unique_id;
  LPDROPTARGET pDropTarget;  
  int _timer_set_on_desk;
  int space;
  int _desktop_padding_horizontal;
  int _desktop_padding_vertical;
  UINT m_hookMessage;
  HWND hwnd_main;
  int cell_width;
  int cell_height;
  int pager_width;
  int pager_height;
  int desktop_height;
  int desktop_width;
  int pager_rel_left;
  int pager_rel_top;
  int num_desktops;
  std::vector < Desktop * >list;
  Desktop *current_desktop;
  Window *clicked_mini_window;
  Window *clicked_mini_window_maybe;
  Desktop *clicked_desktop;
  POINT tray_window_left;
  RECT tray_window_rect;
  HMENU menu;
  UINT shellMessage;
  std::vector < Window * >sticky_list;

  int crashReportingOption;

  std::map <DWORD, bool> m_knownPIDs;

  DWORD m_explorerPID;
  
};
