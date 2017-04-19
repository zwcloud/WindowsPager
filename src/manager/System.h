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

#include <objbase.h>
#include <oleauto.h>
#include <olectl.h>
#include <ocidl.h>
#include <shlobj.h>
#include <Psapi.h>
#include <shobjidl.h>
#include <sys/stat.h> 
#include <Shellapi.h>

#include "Ini.h"

#define GetSys (System::Instance())

class System
{

public:
  bool WpagerApiHookAlreadInjected ( HANDLE hProcess, bool is64Bit);
  std::wstring GetExeFolder(void);
  static System *Instance();
  ~System();
  HWND GetRootWindow(void);
  void RectToGDIPlusRect(Gdiplus::Rect *rect, RECT _rect);
  LONG_PTR GetWindowLongPTR(HWND hWnd, int nIndex);
  UINT GetVirtualKeyCodebyString(std::wstring key_value);
  std::wstring *GetWindowFileName(HWND hWnd);
  bool Is64BitWindows(void);
  RECT GetPrimaryRect(void);
  int GetPrimaryWidth(void);
  int GetPrimaryHeight(void);
  bool GotoURL(std::string url);
  bool SetWindowLongPtrTimeout(HWND hWnd, int nIndex, LONG_PTR dwNewLong, int timeout);
  int GetMultiMonitorMode(void);
  CIniReader* GetIniFile (void);
  RECT RealToVirtualScreenRect (RECT real);
  void VirtualToRealScreenXY (int &x, int &y);
  int GetMonitor(HWND window);
  RECT GetHiddenRect(void);
  bool WindowIsVisibleOnAMonitor(HWND window);
  void ShowMessageBoxModless(std::string text, UINT icon);
  std::string GetDefaultWallpaper(void);
  bool WindowIsHungTimeoutCheck(HWND inhHWnd, UINT waitTime);
  bool CheckIfWindowDoRespond(HWND inhHWnd, UINT waitTime);
  bool CheckIfFileExists(std::wstring strFilename);
  bool WindowIsOwnedbyMe(HWND window);
  void Init(void);
  void Update(void);
  int GetXPTheme(void);
  int GetCaptionHeight(void);
  int GetBorderWidth(void);
  void ShowTaskButton(HWND window);
  void HideTaskButton(HWND window);
  HICON GetIconFromWindowHWND(HWND window);
  HPEN GetPenFrameColor(void) {return pen_framecolor;}
  HPEN GetPenNull(void) {return pen_null;}
  HBRUSH GetBrushFrameColor(void) {return brush_framecolor;}
  HBRUSH GetBrushBlack(void) {return brush_black;}
  HBRUSH GetBrushRed(void) {return brush_red;}
  HBRUSH GetBrushWhite(void) {return brush_white;}
  bool PointIsInsideRect(RECT rect, POINT pt);
  HWND GetPanelWindow(void);
  HWND GetListWindow(void);
  HWND GetTrayWindow(void);
  RECT GetTrayWindowRect(void);
  RECT GetListWindowRect(void);
  RECT GetPanelWindowRect(void);
  int GetScreenHeight(void);
  void SetScreenHeight(int height);
  int GetScreenWidth(void);
  RECT GetFullScreenRect(void);
  int Debuging(void);
  int Round(double a);
  void GetMousePos(int &x, int &y);
  HWND GetTaskButtonWindow(void);
  bool Is64BitProcess(HANDLE hProcess);


  bool TaskBarIsHorizontal(void)
  {
    LogMethod;
    return taskbarHorizontal;
  }

  int TaskBarEdge(void)
  {
    return taskbarEdge;
  }
    
protected:
  System()
  {
  };
  System(const System & other)
  {
  }

private:

  int taskbarEdge;
  bool taskbarHorizontal;
    
  bool System::GetDirectoryFromExe(std::wstring& currentDirectory);
  static DWORD WINAPI SetWindowLongPtrTimeoutThread(LPVOID lpParam);
  static DWORD WINAPI DialogThread(LPVOID lpParam);
  int _GetXPTheme(void);
  LPSTR GetSIDbyProcess(HANDLE process);
  LPSTR GetOwnSID(void);
 
  typedef int (WINAPI *pShellExecute) (HWND hwnd, LPCTSTR lpOperation, LPCTSTR lpFile,
    LPCTSTR lpParameters, LPCTSTR lpDirectory, INT nShowCmd);
  pShellExecute ShellExecutePointer;

  typedef BOOL (WINAPI *pIsWow64Process) (HANDLE, PBOOL);
  pIsWow64Process IsWow64ProcessPointer;

  BOOL InternalIs64BitWindows(void);
    
  std::wstring currentFolder;
  int virtual_left;
  int virtual_top;
  HWND desktop_window;
  bool is64bit;
  int primary_width;
  int primary_height;
  int multiMonitorMode;
  CIniReader *IniFile;
  std::string default_wallpaper;
  RECT primary_rect;
  RECT fullscreen_rect;
  RECT hidden_rect;
  int xp_theme;
  static System *_instance;
  int debugmode;
  ITaskbarList *pTaskbar;
  int caption_height;
  int border_width;
  HWND panel_window;
  HWND list_window;
  HWND tray_window;
  HWND taskbutton_window;
  HWND progman_window;
  RECT tray_window_rect;
  RECT list_window_rect;
  RECT panel_window_rect;
  HBRUSH brush_framecolor;
  HBRUSH brush_white;
  HPEN pen_framecolor;
  HPEN pen_null;
  HBRUSH brush_black;
  HBRUSH brush_red;
  int screen_height;
  int screen_width;
  int fullscreen_height;
  int fullscreen_width;
  LPSTR own_sid;

  struct dialog_data
  {
    std::string *text;
    UINT icon;
  };

  struct set_window_long_data
  {
    HWND hWnd;
    int nIndex;
    LONG_PTR dwNewLong;
  };


};
