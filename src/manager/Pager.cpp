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

#include "..\common\stdafx.h"
#include "System.h"
#include <time.h>
#include "PowrProf.h"


#include "Pager.h"
#include "XWinVer.h"
#include "DropTarget.h"
#include "Ini.h"
#include "SimpleTray.h"
#include "WindowSelector.h"
#include "mdump.h"


using namespace std;
using namespace Gdiplus;
using namespace boost;

Pager *Pager::_instance = NULL;
Pager *Pager::Instance()
{
  if (_instance == NULL)
  {
    _instance = new Pager;
  }
  return _instance;
}

void Pager::SetExplorerPID(DWORD pid)
{
  m_explorerPID=pid;
}

DWORD Pager::GetExplorerPID(void)
{
  return m_explorerPID;
}

void Pager::AddPID(DWORD pid)
{
  LogMethod;

  trace("AddPid: %d\n\n", pid);
  m_knownPIDs.insert(make_pair(pid, TRUE));
}

bool Pager::PIDKnown(DWORD pid)
{
  LogMethod;

  trace("serach pid: %d\n", pid);

  map <DWORD, bool>::iterator iter = m_knownPIDs.find(pid);
  if (iter != m_knownPIDs.end())
  {
    trace("pid %d known\n", pid);
    return TRUE;
  } else
  {
    trace("pid  %d UNkonwdn\n", pid);
    return FALSE;
  }
}

void Pager::DeletePID(DWORD pid)
{
  LogMethod;
  trace("delete pid: %d\n", pid);
  m_knownPIDs.erase(pid);
}


void Pager::UpdateIgnoreWindows(void)
{
  LogMethod;

  vector<wstring> list;
  if (!GetSys->GetIniFile()->GetIgnoreWindowsEntries(list))
  {
    return;
  }

  ignore_map.clear();

  BOOST_FOREACH(wstring s, list)
  {
    int gleich=s.find(L"=");
    if (gleich==wstring::npos)
    {
      continue;
    }

    wstring value=s.substr(gleich+1);
    int komma=value.find(L":");
    if (komma==wstring::npos)
    {
      continue;
    }

    wstring exe_string=value.substr(0, komma);
    to_lower(exe_string);

    wstring class_string=value.substr(komma+1);

    map<wstring, set<wstring>>::iterator iter = ignore_map.find(exe_string);
    if (iter != ignore_map.end())
    {
      set<wstring>::iterator wildcard_iter = iter->second.find(L"*");
      if (wildcard_iter !=iter->second.end())
      {
        trace("found *, do nothing\n");
      } else
      {
        iter->second.insert(class_string);
      }
    } else
    {
      set<wstring> set; set.insert(class_string);
      ignore_map.insert(make_pair(exe_string, set));
    }
  }
}  

bool Pager::CheckIfOnIgnoreList(const wstring& exe_string, const wstring& class_string)
{
  LogMethod;

  map<wstring, set<wstring>>::iterator iter = ignore_map.find(exe_string);
  if (iter == ignore_map.end())
  {
    return false;
  }

  if (iter->second.count(L"*") > 0)
  {
    return true;
  }

  if (iter->second.count(class_string) > 0)
  {
    return true;
  } 
  return false;
}

void Pager::InitHotkeysDesktopScrolling(void)
{
  LogMethod;

  UINT modifier_id=GetSys->GetIniFile()->GetHotkeyModifier(L"HotkeysDesktopScrolling");

  if (_cols > 1)
  {
    {
      UINT vcode=GetSys->GetIniFile()->GetHotkeyScrolling(L"page_left");
      if (vcode!=-1)
      {
        hotkeyatom_left = GlobalAddAtom("PAGER_HOTKEY_LEFT");
        RegisterHotKey(hwnd_main, hotkeyatom_left, modifier_id, vcode);
      }
    }

    {
      UINT vcode=GetSys->GetIniFile()->GetHotkeyScrolling(L"page_right");
      if (vcode!=-1)
      {
        hotkeyatom_right = GlobalAddAtom("PAGER_HOTKEY_RIGHT");
        RegisterHotKey(hwnd_main, hotkeyatom_right, modifier_id, vcode);
      }  
    }
  }

  if (_rows>1)
  {
    {
      UINT vcode=GetSys->GetIniFile()->GetHotkeyScrolling(L"page_up");
      if (vcode!=-1)
      {
        hotkeyatom_up = GlobalAddAtom("PAGER_HOTKEY_UP");
        RegisterHotKey(hwnd_main, hotkeyatom_up, modifier_id, vcode);
      } 
    }
    {
      UINT vcode=GetSys->GetIniFile()->GetHotkeyScrolling(L"page_down");
      if (vcode!=-1)
      {
        hotkeyatom_down = GlobalAddAtom("PAGER_HOTKEY_DOWN");
        RegisterHotKey(hwnd_main, hotkeyatom_down, modifier_id, vcode);
      } 
    }
  }
}

void Pager::InitHotkeysMoveToDesktop(void)
{
  LogMethod;

  UINT modifier_id=GetSys->GetIniFile()->GetHotkeyModifier(L"HotkeysMoveToDesktop");

  trace("modifier_id: %x\n", modifier_id);

  if (_cols > 1)
  {
    {
      UINT vcode=GetSys->GetIniFile()->GetHotkeyMoveToDesktop(L"page_left");
      if (vcode!=-1)
      {
        hotkeyatom_move_left = GlobalAddAtom("PAGER_HOTKEY_MOVE_LEFT");
        RegisterHotKey(hwnd_main, hotkeyatom_move_left, 
          modifier_id, vcode);
      }
    }

    {
      UINT vcode=GetSys->GetIniFile()->GetHotkeyMoveToDesktop(L"page_right");
      if (vcode!=-1)
      {
        hotkeyatom_move_right = GlobalAddAtom("PAGER_HOTKEY_MOVE_RIGHT");
        RegisterHotKey(hwnd_main, hotkeyatom_move_right,
          modifier_id, vcode);
      }  
    }
  }

  if (_rows>1)
  {
    {
      UINT vcode=GetSys->GetIniFile()->GetHotkeyMoveToDesktop(L"page_up");
      if (vcode!=-1)
      {
        hotkeyatom_move_up = GlobalAddAtom("PAGER_HOTKEY_MOVE_UP");
        RegisterHotKey(hwnd_main, hotkeyatom_move_up, 
          modifier_id, vcode);
      } 
    }
    {
      UINT vcode=GetSys->GetIniFile()->GetHotkeyMoveToDesktop(L"page_down");
      if (vcode!=-1)
      {
        hotkeyatom_move_down = GlobalAddAtom("PAGER_HOTKEY_MOVE_DOWN");
        RegisterHotKey(hwnd_main, hotkeyatom_move_down, 
          modifier_id, vcode);
      } 
    }
  }
}

void Pager::InitBadClassList()
{
  LogMethod;

  bad_classes.push_back(L"OfficeTooltip");
  bad_classes.push_back(L"tooltips_class32"); 
  bad_classes.push_back(L"PagerMagicWindow");
  bad_classes.push_back(L"pagermain");
  bad_classes.push_back(L"Progman");
  bad_classes.push_back(L"Shell_TrayWnd");
  bad_classes.push_back(L"DV2ControlHost");
  bad_classes.push_back(L"gdkWindowTempShadow");
  bad_classes.push_back(L"SysShadow");
  bad_classes.push_back(L"Ghost");
  bad_classes.push_back(L"PagerHangUpWindow");
  bad_classes.push_back(L"ComboLBox");
  bad_classes.push_back(L"CtrlNotifySink");
  bad_classes.push_back(L"MsoCommandBarPopup");
  bad_classes.push_back(L"Desktop User Picture");
  bad_classes.push_back(L"SCA_VolumeToolTip");
  bad_classes.push_back(L"PagerHookStarter32");
  bad_classes.push_back(L"PagerHookStarter64");
  bad_classes.push_back(L"WindowsPagerWindowSelector");
  bad_classes.push_back(L"ClockFlyoutWindow");
  bad_classes.push_back(L"ClockTooltipWindow");
  bad_classes.push_back(L"DesktopProgramsMFU");
  bad_classes.push_back(L"Desktop More Programs Pane");
  bad_classes.push_back(L"Desktop OpenBox Host");
  bad_classes.push_back(L"DesktopLogoffPane");
  bad_classes.push_back(L"DesktopDestinationList");
  bad_classes.push_back(L"Desktop User Pane");
  bad_classes.push_back(L"Desktop NSCHost");
  bad_classes.push_back(L"DesktopSpecialFolders");
  bad_classes.push_back(L"TaskListOverlayWnd");
  bad_classes.push_back(L"TaskListThumbnailWnd");
  bad_classes.push_back(L"PAGER_MESSAGE_WINDOW");
  bad_classes.push_back(L"BaseBar");
  bad_classes.push_back(L"SysFader");
  bad_classes.push_back(L"WorkerW");

  if (GetSys->Debuging())
  {
    bad_classes.push_back(L"wndclass_desked_gsk");
    bad_classes.push_back(L"Afx:00A40000:8:00010005:00000000:005C04CD");
  }

  sort(bad_classes.begin(), bad_classes.end());
}

void Pager::InitINIStuff(void)
{
  LogMethod;

  _cols=GetSys->GetIniFile()->GetKeyValueInt(L"Layout", L"columns");
  if (_cols<=0)
  {
    char buf[256];
    sprintf_s(buf, 256, "Error reading key: '%s' in section '%s' in 'windowspager.ini'."\
      "\nPlease repair the 'windowspager.ini' file. Will quit now.", "columns", "layout");
    MessageBox(0, buf, "WindowsPager", MB_ICONHAND);
    exit(1);
  }

  _rows=GetSys->GetIniFile()->GetKeyValueInt(L"Layout", L"rows");
  if (_rows<=0)
  {
    char buf[256];
    sprintf_s(buf, 256, "Error reading key: '%s' in section '%s' in 'windowspager.ini'."\
      "\nPlease repair the 'windowspager.ini' file. Will quit now.", "rows", "layout");
    MessageBox(0, buf, "WindowsPager", MB_ICONHAND);
    exit(1);
  }
  num_desktops=_rows*_cols;

  if (_rows>1)
  {
    space=0;
  } else
  {
    if (GetSys->TaskBarIsHorizontal())
    {
      space=(GetWinVersion->IsVista() || GetWinVersion->IsXP())? 1 : 0;
    } else
    {
      space=0;
    }
  }
  trace("space: %d\n", space);

  start_delay=GetSys->GetIniFile()->GetKeyValueInt(L"Misc", L"start_delay");
  if (start_delay<0)
  {
    char buf[256];
    sprintf_s(buf, 256, "Error reading key: '%s' in section '%s' in 'windowspager.ini'."\
      "\nPlease repair the 'windowspager.ini' file. Will quit now.", "Misc", "start_delay");
    MessageBox(0, buf, "WindowsPager", MB_ICONHAND);
    exit(1);
  }

  start_delay*=1000;
  trace("start delay: %d\n", start_delay);

  draw_sticky_windows=false;
  wstring sticky_string=GetSys->GetIniFile()->GetKeyValue(L"Misc", L"draw_sticky_windows");
  if (sticky_string.compare(L"yes")==0)
  {
    trace("use sticky hack\n");
    draw_sticky_windows=true;
  }  

  sticky_window_button=false;
  wstring sticky_button_string=GetSys->GetIniFile()->GetKeyValue(L"Misc", L"sticky_window_button");
  if (sticky_button_string.compare(L"yes")==0)
  {
    trace("use buttons for sticky windows\n");
    sticky_window_button=true;
  }  

  incognito_mode=false;
  wstring incognito_string=GetSys->GetIniFile()->GetKeyValue(L"Misc", L"trayicon");
  if (incognito_string.compare(L"yes")==0)
  {
    trace("use incognito mode\n");
    incognito_mode=true;
  } 

  hung_up_information_window=false;
  wstring hung_up_string=GetSys->GetIniFile()->GetKeyValue(L"Misc", L"inform_about_non_responding_windows");
  if (hung_up_string.compare(L"yes")==0)
  {
    trace("inform_about_non_responding_windows=yes\n");
    hung_up_information_window=true;
  } 

  rememberActiveWindow=false;
  wstring rememberActiveWindoString=GetSys->GetIniFile()->GetKeyValue(L"Misc", L"remember_active_window");
  if (rememberActiveWindoString.compare(L"yes")==0)
  {
    trace("rember aktive window=true\n");
    rememberActiveWindow=true;
  }  

  wstring tmp_string=GetSys->GetIniFile()->GetKeyValue(L"MoveAndSwitch", L"modifier");
  if( tmp_string.compare(L"shift")==0)
  {
    SetProp(GetSys->GetPanelWindow(), "PagerMoveAndSwitchModifier",  (void *) 16);
  } else if ( tmp_string.compare(L"control")==0)
  {
    SetProp(GetSys->GetPanelWindow(), "PagerMoveAndSwitchModifier",  (void *) 17);
  }   

  crashReportingOption=0;
  wstring crashOption=GetSys->GetIniFile()->GetKeyValue(L"Misc", L"crash_reporting");
  if( crashOption.compare(L"silent")==0)
  {
    crashReportingOption=1;
  } else if ( crashOption.compare(L"none")==0)
  {
    crashReportingOption=2;
  }   

  InitBadClassList();
}

void Pager::StartHookStarter64Bit(void)
{
  LogMethod;

  PROCESS_INFORMATION pif;
  STARTUPINFOW si; 
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);

  wstring hookStarterPath=GetSys->GetExeFolder()+L"\\hookstarter64.exe";
  tracew(L"hoostarter path: %s\n", hookStarterPath.c_str());

  if (!GetSys->Debuging() && !GetSys->CheckIfFileExists(hookStarterPath.c_str()))
  {
    MessageBox(0, "Missing 'hookstarter64.exe' file in the lib folder! Will quit now.",
      "WindowsPager", MB_ICONHAND);
    exit(1);
  }

  WCHAR cmd_buf[MAX_PATH+100];
  swprintf_s (cmd_buf, MAX_PATH+100, L"%s %x", hookStarterPath.c_str(), (long) hwnd_main); 
  if (CreateProcessW(NULL,
    cmd_buf, NULL, NULL, FALSE, IDLE_PRIORITY_CLASS, NULL, NULL, &si, &pif))
  {
    trace("starting ok\n");
    CloseHandle(pif.hProcess);
    CloseHandle(pif.hThread);
  } 

}

void Pager::StartHookStarter32Bit()
{
  LogMethod;

  PROCESS_INFORMATION pif32;
  STARTUPINFOW si32; 
  ZeroMemory(&si32,sizeof(si32));
  si32.cb = sizeof(si32);

  wstring hookStarterPath=GetSys->GetExeFolder() +L"\\hookstarter32.exe";
  tracew(L"hoostarter path: %s\n", hookStarterPath.c_str());

  if (!GetSys->Debuging() && !GetSys->CheckIfFileExists(hookStarterPath.c_str()))
  {
    MessageBox(0, "Missing 'hookstarter32.exe' file in the lib folder! Will quit now.",
      "WindowsPager", MB_ICONHAND);
    exit(1);
  }

  trace("hwnd_main hookstarter: %x\n", hwnd_main);

  WCHAR cmd_buf[MAX_PATH+100];
  swprintf_s (cmd_buf, MAX_PATH+100, L"%s %x", hookStarterPath.c_str(), (long) hwnd_main); 
  if (CreateProcessW(NULL,
    cmd_buf, NULL, NULL, FALSE, IDLE_PRIORITY_CLASS, NULL, NULL, &si32, &pif32))
  {
    trace("starting ok\n");
    CloseHandle(pif32.hProcess);
    CloseHandle(pif32.hThread);
  }

  tracew(cmd_buf);

}

void Pager::StartHookStarter(void)
{
  LogMethod;

  if (GetSys->Is64BitWindows())
  {
    StartHookStarter64Bit();
  } 

  StartHookStarter32Bit();
}

void Pager::CreatePagerMessageWindow(void)
{
  LogMethod;

  WNDCLASS wc2;

  trace("create messag  window\n");

  ZeroMemory(&wc2, sizeof(wc2));
  wc2.style = CS_HREDRAW | CS_VREDRAW;
  wc2.lpfnWndProc = (WNDPROC) Pager::WndProcMessageWindow;
  wc2.cbClsExtra = 0;
  wc2.cbWndExtra = 0;
  wc2.hInstance = hI;
  wc2.hIcon = NULL;
  wc2.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc2.hbrBackground = (HBRUSH) NULL;
  wc2.lpszMenuName = NULL;
  wc2.lpszClassName = "PAGER_MESSAGE_WINDOW";
  RegisterClass(&wc2);


  HWND message_window =
    CreateWindowExW(WS_EX_TOOLWINDOW| WS_EX_NOACTIVATE, L"PAGER_MESSAGE_WINDOW",
    L"WindowsPager", WS_POPUP, -1, -1,
    0, 0, NULL, NULL, hI,
    (LPVOID) NULL);

  ShowWindow(message_window, SW_SHOWNOACTIVATE);

  trace("create down\n");
}

void Pager::Init3(void)
{
  LogMethod;

  if (!GetSys->Debuging())
  {
    current_desktop->AddToplevelWindows();
  }

  if (GetSys->TaskBarIsHorizontal())
  {
    !incognito_mode?Pager::UpdatePagerWindowPosHorizontalTaskbar(FALSE, TRUE):0;
  } else
  {
    !incognito_mode?Pager::UpdatePagerWindowPosVerticalTaskbar(FALSE, TRUE):0;
  }

  !incognito_mode?ShowWindow(hwnd_main, SW_SHOW):0;
  !incognito_mode?InvalidateRect(hwnd_main, NULL, true):0;
  !incognito_mode?UpdateWindow(hwnd_main):0;

  HANDLE threadHandle_panel = CreateThread(0, 0, Pager::Watchdog, NULL, 0, NULL);
  CloseHandle(threadHandle_panel);

  init_ready=true;
  PostMessage(hwnd_main, MSG_CREATE_MESSAGE_WINDOW, 0, 0);
}

void Pager::InitRightClickMenu()
{
  LogMethod;

  menu = CreatePopupMenu();
  InsertMenu(menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUQUIT), "Quit");
  InsertMenu(menu, 0, MF_BYPOSITION | MF_SEPARATOR, 2, NULL);
  InsertMenu(menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUSELECTOR), 
    "Add window(s) to the ignore list");
  InsertMenu(menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUHELP), 
    "Open help web site");
  InsertMenu(menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUDOWNLOAD), 
    "Open download web site");
  InsertMenu(menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUABOUT), "About");
}

void Pager::Init2(HWND _main_window)
{
  LogMethod;

  hwnd_main=_main_window;

  trace("init 2: hwnd_main: %x\n", hwnd_main);

  Pager::InitHotkeysDesktopScrolling();
  Pager::InitHotkeysMoveToDesktop();

  for (int i = 0; i < num_desktops; i++)
  {
    Desktop *NewDesktop = new Desktop(i);
    list.push_back(NewDesktop);
  }

  current_desktop = list.at(0);

  m_hookMessage = RegisterWindowMessage("PagerHookMessages");

  shellMessage = RegisterWindowMessage("SHELLHOOK");
  RegisterShellHookWindow(hwnd_main);

  if (!incognito_mode)
  {
    pDropTarget = (LPDROPTARGET) new DropTarget(); 
    CoLockObjectExternal(pDropTarget, TRUE, TRUE);
    RegisterDragDrop(hwnd_main, pDropTarget);
  }

  InitRightClickMenu();

  if (current_desktop->GetWallpaperPath()!=L"default")
  {
    current_desktop->ChangeWallpaper();
  }

  SetProp (hwnd_main, "PAGER_NUM_DESKTOPS", (void *) num_desktops);

  if (incognito_mode)
  {
    tray= new CSimpleTray(current_desktop->GetTrayIcon(), hwnd_main);
  }

  Pager::StartHookStarter();
}


void Pager::Init1Point5(void)
{
  LogMethod;

  char szName[] = "pagermain";
  WNDCLASS wc;

  Pager::UpdateIgnoreWindows();


  if (GetSys->TaskBarIsHorizontal())
  {
    !incognito_mode?Pager::UpdatePagerWindowPosHorizontalTaskbar(TRUE, FALSE):0;
  } else
  {
    !incognito_mode?Pager::UpdatePagerWindowPosVerticalTaskbar(TRUE, FALSE):0;
  }

  ZeroMemory(&wc, sizeof(wc));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC) Pager::WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hI;
  wc.hIcon = NULL;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) NULL;
  wc.lpszMenuName = NULL;
  wc.lpszClassName = szName;
  RegisterClass(&wc);

  if (hung_up_information_window==true)
  {
    WNDCLASS wcDummy;
    ZeroMemory(&wcDummy, sizeof(wcDummy));
    wcDummy.style = CS_HREDRAW | CS_VREDRAW;
    wcDummy.lpfnWndProc = (WNDPROC) HangUpInfoWindow::WndProc;
    wcDummy.cbClsExtra = 0;
    wcDummy.cbWndExtra = 0;
    wcDummy.hInstance = hI;
    wcDummy.hIcon = LoadIcon (NULL, IDI_EXCLAMATION);
    wcDummy.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcDummy.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
    wcDummy.lpszMenuName = NULL;
    wcDummy.lpszClassName = "PagerHangUpWindow";
    RegisterClass(&wcDummy);
  }

  if (incognito_mode)
  {
    HWND hwnd_main =
      CreateWindowEx(WS_EX_NOPARENTNOTIFY | WS_EX_NOACTIVATE, szName, NULL,
      0, 0, 0,
      0, 0, NULL, NULL, hI,
      (LPVOID) GetPager);
  } else
  {
    HWND hwnd_main =
      CreateWindowEx(WS_EX_NOPARENTNOTIFY|WS_EX_NOACTIVATE, szName,
      NULL, WS_CHILD, pager_rel_left, pager_rel_top,
      pager_width, pager_height, GetSys->GetPanelWindow(), NULL, hI,
      (LPVOID) GetPager);
  }
}

int CALLBACK Pager::DelayTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
  LogMethod;

  KillTimer(hwnd, idTimer);
  trace("delay time over\n");
  GetPager->Init1Point5();
  return 0;
}

void Pager::Init(void)
{
  LogMethod;

  init_ready=false;
  key2value.clear();
  list.clear();
  sticky_list.clear();
  bad_classes.clear();
  hotkeyatom_left=NULL;
  hotkeyatom_right=NULL;
  hotkeyatom_up=NULL;
  hotkeyatom_down=NULL;
  tray=NULL;
  drag_timer_id=0;
  hand_cursor= LoadCursor(NULL, IDC_HAND); 
  clicked_desktop = NULL;
  clicked_mini_window = NULL;
  clicked_mini_window_maybe=NULL;
  pager_rel_left=0;
  pager_rel_top=0;
  pager_width=0;
  pager_height=0;
  hwnd_main=NULL;
  _timer_set_on_desk=-1;
  start_delay=0;
  crashReportingOption=0;
  m_explorerPID=0;

  unique_id= 0;
  hookatom= GlobalAddAtom("WindowsPagerMessage");
  maybe_window_destroyed=false;
  clicked_mini_window_destroyed=false;

  ignore_map.clear();

  m_knownPIDs.clear();

  Pager::InitINIStuff();


  SetProp(GetSys->GetPanelWindow(), "PAGER_NUM_DESKTOPS", (void*) num_desktops);



  wstring apiHook32Path=GetSys->GetExeFolder() +L"\\WindowsPagerApiHook32bit.dll";
  tracew(L"hoostarter path: %s\n",  apiHook32Path.c_str());

  if (!GetSys->CheckIfFileExists(apiHook32Path))
  {
    MessageBox(0, "Missing 'WindowsPagerApiHook32bit.dll' file in the lib folder! Will quit now.",
      "WindowsPager", MB_ICONHAND);
    exit(1);
  }

  if (GetSys->Is64BitWindows())
  {
    wstring apiHook64Path=GetSys->GetExeFolder() +L"\\WindowsPagerApiHook64bit.dll";
    tracew(L"hoostarter path: %s\n",  apiHook64Path.c_str());

    if (!GetSys->CheckIfFileExists(apiHook64Path))
    {
      MessageBox(0, "Missing 'WindowsPagerApiHook64bit.dll' file in the lib folder! Will quit now.",
        "WindowsPager", MB_ICONHAND);
      exit(1);
    }
  }

  if (start_delay >0)
  {
    UINT_PTR delay_timer_id=CreateUniqueTimerID();
    trace("start delay\n");
    SetTimer(NULL, delay_timer_id, start_delay, (TIMERPROC) DelayTimer);
  } else
  {
    GetPager->Init1Point5();
  }
}

DWORD WINAPI Pager::Watchdog(LPVOID lpParam)
{
  LogMethod;

  HANDLE hProcess;
  DWORD process_id;

  if (!GetWindowThreadProcessId(GetSys->GetPanelWindow(), &process_id))
  {
    return 0;
  }

  GetPager->SetExplorerPID(process_id);

  hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
  if (hProcess == NULL)
  {
    return 0;
  }

  WaitForSingleObject(hProcess, INFINITE);
  CloseHandle(hProcess);


  GetPager->RestoreWindowsBeforeExit();

  delete GetPager;
  PostQuitMessage(0);

  exit(0);

  return 0;
}

void Pager::RestoreWindowsBeforeExit(bool closeWindows/*=false*/)
{
  LogMethod;

  for (int i = 0; i < num_desktops; i++)
  {
    Desktop *desktop = list.at(i);
    if (desktop->GetDesktopNumber()!=current_desktop->GetDesktopNumber())
    {
      desktop->RestoreWindowsBeforeExitOnHidden();
    } else
    {
      desktop->RestoreWindowsBeforeExitOnCurrent();
    }
  }
}

void Pager::UninitializeStickyList(void)
{
  LogMethod;

  vector <Window*>::iterator iter = sticky_list.begin();
  while (iter != sticky_list.end())
  {
    (*iter)->RestoreOnExitSticky();
    delete (*iter);
    iter++;
  }
}

void Pager::UnregisterHotKeys()
{
  LogMethod;

  if (hotkeyatom_left)
  {
    UnregisterHotKey(hwnd_main, hotkeyatom_left);
    GlobalDeleteAtom(hotkeyatom_left);
  }
  if (hotkeyatom_right)
  {
    UnregisterHotKey(hwnd_main, hotkeyatom_right);
    GlobalDeleteAtom(hotkeyatom_right);
  }
  if (hotkeyatom_up)
  {
    UnregisterHotKey(hwnd_main, hotkeyatom_up);
    GlobalDeleteAtom(hotkeyatom_up);
  }
  if (hotkeyatom_down)
  {
    UnregisterHotKey(hwnd_main, hotkeyatom_down);
    GlobalDeleteAtom(hotkeyatom_down);
  }

  if (hotkeyatom_move_left)
  {
    UnregisterHotKey(hwnd_main, hotkeyatom_move_left);
    GlobalDeleteAtom(hotkeyatom_move_left);
  }
  if (hotkeyatom_move_right)
  {
    UnregisterHotKey(hwnd_main, hotkeyatom_move_right);
    GlobalDeleteAtom(hotkeyatom_move_right);
  }
  if (hotkeyatom_move_up)
  {
    UnregisterHotKey(hwnd_main, hotkeyatom_move_up);
    GlobalDeleteAtom(hotkeyatom_move_up);
  }
  if (hotkeyatom_move_down)
  {
    UnregisterHotKey(hwnd_main, hotkeyatom_move_down);
    GlobalDeleteAtom(hotkeyatom_move_down);
  }
}

Pager::~Pager()
{
  LogMethod;
  trace("pager deconstruct\n");
  DestroyCursor(hand_cursor);

  ignore_map.clear();

  if (tray)
  {
    delete tray;
  }

  DeregisterShellHookWindow(hwnd_main);

  if (!incognito_mode)
  {
    RevokeDragDrop(hwnd_main);
    pDropTarget->Release();  
    CoLockObjectExternal(pDropTarget, FALSE, TRUE);
  }


  for (int i2 = 0; i2 < num_desktops; i2++)
  {
    Desktop *desktop = list.at(i2);
    delete desktop;
  }
  list.clear();

  Pager::UninitializeStickyList();
  sticky_list.clear();

  if (GetSys->Is64BitWindows())
  {
    HWND hook_starter64 = FindWindow("PagerHookStarter64", NULL);
    if (hook_starter64)
    {
      trace("hook_starter64: %x\n", hook_starter64);
      PostMessage(hook_starter64, WM_CLOSE, 0, 0);
    }
  }

  HWND hook_starter32 = FindWindow("PagerHookStarter32", NULL);
  if (hook_starter32)
  {
    trace("hook_starter32: %x\n", hook_starter32);
    PostMessage(hook_starter32, WM_CLOSE, 0, 0);
  }

  bad_classes.clear();


  key2value.clear();

  DestroyMenu(menu);

  UnregisterClass("pagermain", hI);
  UnregisterClass("PagerHangUpWindow", hI);
  DestroyWindow(hwnd_main);

  Pager::ResetListwindow();

  UnregisterHotKeys();

  GlobalDeleteAtom(hookatom);
  delete GetSys;
  delete GetWinVersion;

  //PostQuitMessage(0);
}

void Pager::UpdateStickyWindows(void)
{
  LogMethod;

  vector < Window * >::iterator iter = sticky_list.begin();
  while (iter != sticky_list.end())
  {
    (*iter)->UpdateMiniWindow();
    iter++;
  }
}

void Pager::DrawStickyWindows(Gdiplus::Graphics *g)
{
  LogMethod;

  vector < Window * >::iterator iter = sticky_list.begin();
  while (iter != sticky_list.end())
  {
    RECT rect_first = (*iter)->GetMiniWindowRect();
    Pager::PagerToDesktop((*iter)->GetDesktop(), rect_first);

    if (!(*iter)->HasFlag("iconic") &&
      !(*iter)->HasFlag("ignore") &&
      !(*iter)->HasFlag("OnSecondaryMonitor") &&
      !(*iter)->GetWmExTransparent())
    {
      vector < Desktop * >::iterator desktop_iter = list.begin();
      while (desktop_iter != list.end())
      {
        RECT tmp;
        CopyRect(&tmp, &rect_first);
        Pager::DesktopToPager(*desktop_iter, tmp);
        (*iter)->DrawMiniWindow(g, tmp.left, tmp.top, TRUE);
        desktop_iter++;
      }

    }
    iter++;
  }
}

void Pager::OnPaint(Gdiplus::Graphics *g)
{
  try
  {
    LogMethod;

    Rect panelrect(0, 0, pager_width-1, pager_height-1);
    if (GetWinVersion->IsVista() || GetWinVersion->IsXP())
    {
      REAL positions[3] = {0.0f, 0.3f, 1.0f};
      REAL factors[3] =   {0.0f, 0.6f, 1.0f};
      LinearGradientBrush linGrBrush(
        Point(0, panelrect.Y-1), Point(0, panelrect.GetBottom()+1),
        Color(255, 100, 100, 100), Color(255, 200, 200, 200));
      linGrBrush.SetBlend(factors, positions, 3);
      g->FillRectangle(&linGrBrush, panelrect.X, panelrect.Y, panelrect.Width+1,
        panelrect.Height+1);
    } else if (GetWinVersion->IsXPClassic() || GetWinVersion->IsWin2000())
    {
      SolidBrush background(Color(255, 212, 208, 200));
      g->FillRectangle(&background, panelrect.X, panelrect.Y, panelrect.Width+1,
        panelrect.Height+1);
    }

    vector < Desktop * >::iterator desktop_iter = list.begin();
    while (desktop_iter != list.end())
    {
      (*desktop_iter)->DrawDesktop(g);
      desktop_iter++;
    }

    desktop_iter = list.begin();
    while (desktop_iter != list.end())
    {
      (*desktop_iter)->DrawMiniWindows(g);
      (*desktop_iter)->DrawFlash(g);
      desktop_iter++;
    }
  } catch (...)
  {
  }
}

int CALLBACK Pager::DragTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
  LogMethod;

  KillTimer(hwnd, idTimer);

  trace("time over\n");

  if (GetPager->clicked_mini_window_maybe==NULL)
  {
    return 0;
  }

  if (GetPager->clicked_desktop==NULL)
  {
    GetPager->clicked_mini_window_maybe=NULL;
    trace("clicked desktop is null\n");
    return 0;
  }

  POINT loc;
  if (!GetCursorPos(&loc))
  {
    GetPager->clicked_mini_window_maybe=NULL;
    return 0;
  }

  if (!ScreenToClient(GetPager->GetPagerWindowHWND(), &loc))
  {
    GetPager->clicked_mini_window_maybe=NULL;
    return 0;
  }

  Desktop *desktop_2000 = GetPager->GetDesktopFromPoint(loc);
  if (desktop_2000==NULL)
  {
    GetPager->clicked_mini_window_maybe=NULL;
    trace("desktop after 2000 ist null\n");
    return 0;
  }

  if (GetPager->clicked_desktop->GetDesktopNumber()!=desktop_2000->GetDesktopNumber())
  {
    trace("different desktop after 2000\n");
    GetPager->clicked_mini_window_maybe=NULL;
    return 0;
  }

  Window *clicked_mini_window_2000 = desktop_2000->GetMiniWindowByPoint(loc);
  if (clicked_mini_window_2000==NULL)
  {
    trace("no window after 2000\n");
    GetPager->clicked_mini_window_maybe=NULL;
    return 0;
  }

  if (clicked_mini_window_2000->GetRealWindowHWND() == 
    GetPager->clicked_mini_window_maybe->GetRealWindowHWND())
  {
    trace("clicked windows sind gleich\n");
    clicked_mini_window_2000->SetPadding(loc);
    GetPager->StartMiniWindowDragging(clicked_mini_window_2000);
    GetPager->clicked_mini_window_maybe=NULL;
  } else
  {
    GetPager->clicked_mini_window_maybe=NULL;
  }

  return 0;
}    

void Pager::MouseDown(POINT pt)
{
  LogMethod;

  clicked_mini_window_maybe=NULL;
  drag_timer_id?KillTimer(hwnd_main, drag_timer_id):0;
  drag_timer_id=0;
  maybe_window_destroyed=false;
  clicked_mini_window_destroyed=false;

  clicked_desktop = Pager::GetDesktopFromPoint(pt);
  if (clicked_desktop!=NULL)
  {
    clicked_mini_window_maybe = clicked_desktop->GetMiniWindowByPoint(pt);
    if (clicked_mini_window_maybe!=NULL)
    {
      trace("have clicked window start timer\n");
      drag_timer_id=CreateUniqueTimerID();
      SetTimer(hwnd_main, drag_timer_id, 250, (TIMERPROC) DragTimer);
    }
  }

}

void Pager::StartMiniWindowDragging(Window *clicked_window)
{
  LogMethod;

  if (clicked_window==NULL)
  {
    return;
  }

  RECT rect;
  if (!GetWindowRect(hwnd_main, &rect))
  {
    clicked_mini_window=NULL;
    return;
  }

  clicked_mini_window = clicked_window;

  if (!clicked_mini_window->MiniWindowStartDragging(rect, current_desktop,
    _desktop_padding_horizontal, _desktop_padding_vertical))
  {
    clicked_mini_window=NULL;
    return;
  }

  SetCapture(hwnd_main);
  ClipCursor(&rect);
  SetCursor(hand_cursor);
}

void Pager::PanelRectToScreenRect(RECT & rect)
{
  LogMethod;

  if (GetWinVersion->IsWin2000() || GetWinVersion->IsXPClassic())
  {
    rect.top += (GetSys->GetPanelWindowRect().top + 4);
    rect.right += (GetSys->GetPanelWindowRect().left + 4);
    rect.bottom += (GetSys->GetPanelWindowRect().top + 4);
    rect.left += (GetSys->GetPanelWindowRect().left + 4);
  } else
  {
    rect.top += GetSys->GetPanelWindowRect().top;
    rect.right += GetSys->GetPanelWindowRect().left;
    rect.bottom += GetSys->GetPanelWindowRect().top;
    rect.left += GetSys->GetPanelWindowRect().left;
  }
}

void Pager::MouseUpNoMiniWindow(POINT pt)
{
  LogMethod;

  if (clicked_desktop == NULL)
  {
    return;
  }

  Desktop *released_desk = Pager::GetDesktopFromPoint(pt);
  if (released_desk == NULL)
  {
    return;
  }

  if (released_desk->GetDesktopNumber() != clicked_desktop->GetDesktopNumber())
  {
    return;
  }

  if (released_desk->GetDesktopNumber() == current_desktop->GetDesktopNumber())
  {
    return;
  }

  Pager::SwitchToDesktop(released_desk);
  return;
}

void Pager::MouseUpWithMiniWindow(void)
{
  LogMethod;

  ReleaseCapture();
  ClipCursor(NULL);
  clicked_mini_window->MiniWindowDragStop(current_desktop);
  clicked_mini_window = NULL;
  Pager::RedrawWindow();
}

void Pager::MouseMoveNoMiniWindow(POINT pt)
{
  LogMethod;

  if (clicked_mini_window_maybe!=NULL)
  {
    Desktop *desk = Pager::GetDesktopFromPoint(pt);
    if (desk==NULL)
    {
      clicked_mini_window_maybe=NULL;
      drag_timer_id?KillTimer(hwnd_main, drag_timer_id):0;
      drag_timer_id=0;
      return;
    }

    Window *curr_window = desk->GetMiniWindowByPoint(pt);
    if (curr_window==NULL)
    {
      clicked_mini_window_maybe=NULL;
      drag_timer_id?KillTimer(hwnd_main, drag_timer_id):0;
      drag_timer_id=0;
      trace("no window under cursor\n");
      return;
    }

    trace("have window under cursor\n");

    if (curr_window->GetRealWindowHWND() != clicked_mini_window_maybe->GetRealWindowHWND())
    {
      trace("moved out of maybe window\n");
      clicked_mini_window_maybe=NULL;
      drag_timer_id?KillTimer(hwnd_main, drag_timer_id):0;
      drag_timer_id=0;
      return;
    }
  }
}

void Pager::MouseMoveWithMiniWindow(POINT pt)
{
  LogMethod;

  Desktop *hover_desktop = Pager::GetDesktopFromPoint(pt);
  if (!hover_desktop)
  {
    trace("no hover desktop\n");
    ClipCursor(NULL);
    ReleaseCapture();
    clicked_mini_window->DragOff();
    RemoveProp(clicked_mini_window->GetRealWindowHWND(), "PAGER_MINIWINDOW");

    clicked_mini_window->RemoveHiddenStyles();

    clicked_mini_window->SetHoverDesktop(NULL);
    clicked_mini_window=NULL;
    return;
  }

  clicked_mini_window->MiniWindowMoving(current_desktop, hover_desktop, pt);

  Pager::RedrawWindow();
}

LRESULT Pager::ShellCalls(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  LogMethod;

  switch (wParam)
  {
  case HSHELL_WINDOWCREATED:
    {
      if (IsBadWindow((HWND) lParam))
      {
        break;
      }
      Pager::WindowCreateEvent((HWND) lParam);

    }
    break;

  case HSHELL_WINDOWDESTROYED:
    {
      Pager::WindowDestroyEvent((HWND) lParam);
    }
    break;

  case HSHELL_WINDOWACTIVATED:
    {
      if (IsBadWindow((HWND) lParam))
      {
        break;
      }
      WindowActivateEvent(lParam);
    }
    break;
  }

  return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

LRESULT CALLBACK Pager::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  __try
  {
    if (!GetPager->InitReady() && uMessage==WM_CREATE)
    {
      GetPager->Init2(hWnd);
      return 0;
    }

#if defined(_WIN64)
    if (!GetPager->InitReady()&& uMessage == GetPager->GetHookMessage() &&  wParam==MSG_HOOK_INSTALLED_64)
    {
      GetPager->Init3();
      return 0;
    }
#else
    if (!GetPager->InitReady() && uMessage == GetPager->GetHookMessage() &&  wParam==MSG_HOOK_INSTALLED_32)
    {
      GetPager->Init3();
      return 0;
    }
#endif

    if (!GetPager->InitReady())
    {
      return DefWindowProc(hWnd, uMessage, wParam, lParam);
    }

    if (uMessage == GetPager->GetHookMessage())
    {
      return GetPager->HookCalls(hWnd, uMessage, wParam, lParam);
    }

    if (uMessage == GetPager->GetShellMessageID())
    {
      return GetPager->ShellCalls(hWnd, uMessage, wParam, lParam);
    }

    return GetPager->MessageCalls(hWnd, uMessage, wParam, lParam);

  } __except(MiniDumper::NonSeriousCrash(GetExceptionInformation()))
  {
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
  }
}

LRESULT CALLBACK Pager::WndProcMessageWindow(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  if (uMessage==WM_QUERYENDSESSION)
  {
    trace("got query end session");

    GetPager->RestoreWindowsBeforeExit();
    delete GetPager;

    return true;
  }

  return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

int Pager::GetAboveDesktopNumber(int current_number)
{
  LogMethod;

  if (_rows==1)
  {
    return -1;
  }

  if (current_number<=_cols)
  {
    return num_desktops-_cols+ current_number;
  } else
  {
    return current_number-_cols;
  }
}

int Pager::GetBelowDesktopNumber(int current_number)
{
  LogMethod;

  if (_rows==1)
  {
    return -1;
  }

  if (current_number>(num_desktops-_cols))
  {
    return (_cols-(num_desktops-current_number));
  } else
  {
    return current_number+_cols;
  }
}

int Pager::GetRightDesktopNumber(int current_number)
{
  LogMethod;

  if (_cols==1)
  {
    return -1;
  }


  int rest=current_number%_cols; 
  if (rest==0)
  {
    return current_number-(_cols-1);
  } else
  {
    return current_number+1;
  }
}

int Pager::GetLeftDesktopNumber(int current_number)
{
  LogMethod;

  if (_cols==1)
  {
    return -1;
  }

  int rest=current_number%_cols; 
  if (rest==1)
  {
    return current_number+(_cols-1);
  } else
  {
    return current_number-1;
  }

}

void Pager::HotkeyPressed(WPARAM wParam)
{
  LogMethod;

  char buf[50];
  if (!GlobalGetAtomNameA((ATOM) wParam, buf, sizeof(buf)))
  {
    return;
  }

  if (strncmp(buf, "PAGER_HOTKEY_DESKTOP_", 21)==0)
  {
    int number = atoi(&buf[21]);
    trace("hotkey %d pressed\n", number);

    if ((number-1) == current_desktop->GetDesktopNumber())
    {
      return;
    }

    Desktop *new_desk = Pager::list.at(number-1);
    Pager::SwitchToDesktop(new_desk);

  } else if (strncmp(buf, "PAGER_HOTKEY_MOVE_TO_DESKTOP_", 29)==0)
  {
    int number = atoi(&buf[29]);
    trace("hotkey %d pressed\n", number);

    if ((number-1) == current_desktop->GetDesktopNumber())
    {
      return;
    }

    HWND toplevel_window = GetForegroundWindow();
    if (toplevel_window == NULL)
    {
      return;
    }

    Window *window = current_desktop->GetWindowByHWND(toplevel_window);
    if (!window)
    {
      return;
    }

    Desktop *new_desk = Pager::list.at(number-1);
    Pager::MoveWindowFromToDesktop(current_desktop, new_desk, window);

  } else if (strcmp(buf, "PAGER_HOTKEY_LEFT")==0)
  {
    trace("got hotkey left\n");
    int new_desk_number=Pager::GetLeftDesktopNumber(current_desktop->GetDesktopNumber()+1);
    if (new_desk_number==-1)
    {
      return;
    }

    trace("new desk: %d\n", new_desk_number);
    Desktop *new_desk = Pager::list.at(new_desk_number-1);
    Pager::SwitchToDesktop(new_desk);
    return;
  } else if (strcmp(buf, "PAGER_HOTKEY_RIGHT")==0)
  {
    trace("got hotkey right\n");
    int new_desk_number=Pager::GetRightDesktopNumber(current_desktop->GetDesktopNumber()+1);
    if (new_desk_number==-1)
    {
      return;
    }

    trace("new desk: %d\n", new_desk_number);
    Desktop *new_desk = Pager::list.at(new_desk_number-1);
    Pager::SwitchToDesktop(new_desk);
    return;
  }else if (strcmp(buf, "PAGER_HOTKEY_UP")==0)
  {
    trace("got hotkey UP\n");
    int new_desk_number=Pager::GetAboveDesktopNumber(current_desktop->GetDesktopNumber()+1);
    if (new_desk_number==-1)
    {
      return;
    }

    trace("new desk: %d\n", new_desk_number);
    Desktop *new_desk = Pager::list.at(new_desk_number-1);
    Pager::SwitchToDesktop(new_desk);
    return;
  }else if (strcmp(buf, "PAGER_HOTKEY_DOWN")==0)
  {
    trace("got hotkey DOWN\n");
    int new_desk_number=Pager::GetBelowDesktopNumber(current_desktop->GetDesktopNumber()+1);
    if (new_desk_number==-1)
    {
      return;
    }


    trace("new desk: %d\n", new_desk_number);
    Desktop *new_desk = Pager::list.at(new_desk_number-1);
    Pager::SwitchToDesktop(new_desk);
    return;
  } else if (strcmp(buf, "PAGER_HOTKEY_MOVE_LEFT")==0)
  {
    trace("got hotkey move left\n");
    HWND toplevel_window = GetForegroundWindow();
    if (toplevel_window == NULL)
    {
      return;
    }
    trace("have toplevel\n");

    Window *window = current_desktop->GetWindowByHWND(toplevel_window);
    if (!window)
    {
      return;
    }
    trace("toplevel known\n");

    int new_desk_number=Pager::GetLeftDesktopNumber(current_desktop->GetDesktopNumber()+1);
    trace("new desk: %d\n", new_desk_number);
    Desktop *new_desk = Pager::list.at(new_desk_number-1);
    Pager::MoveWindowFromToDesktop(current_desktop, new_desk, window);
    return;
  }
  else if (strcmp(buf, "PAGER_HOTKEY_MOVE_RIGHT")==0)
  {
    trace("got hotkey move right\n");
    HWND toplevel_window = GetForegroundWindow();
    if (toplevel_window == NULL)
    {
      return;
    }
    trace("have toplevel\n");

    Window *window = current_desktop->GetWindowByHWND(toplevel_window);
    if (!window)
    {
      return;
    }
    trace("toplevel known\n");

    int new_desk_number=Pager::GetRightDesktopNumber(current_desktop->GetDesktopNumber()+1);
    trace("new desk: %d\n", new_desk_number);
    Desktop *new_desk = Pager::list.at(new_desk_number-1);
    Pager::MoveWindowFromToDesktop(current_desktop, new_desk, window);
    return;
  }
  else if (strcmp(buf, "PAGER_HOTKEY_MOVE_UP")==0)
  {
    trace("got hotkey move up\n");
    HWND toplevel_window = GetForegroundWindow();
    if (toplevel_window == NULL)
    {
      return;
    }
    trace("have toplevel\n");

    Window *window = current_desktop->GetWindowByHWND(toplevel_window);
    if (!window)
    {
      return;
    }
    trace("toplevel known\n");

    int new_desk_number=Pager::GetAboveDesktopNumber(current_desktop->GetDesktopNumber()+1);
    trace("new desk: %d\n", new_desk_number);
    Desktop *new_desk = Pager::list.at(new_desk_number-1);
    Pager::MoveWindowFromToDesktop(current_desktop, new_desk, window);
    return;
  }
  else if (strcmp(buf, "PAGER_HOTKEY_MOVE_DOWN")==0)
  {
    trace("got hotkey move down\n");
    HWND toplevel_window = GetForegroundWindow();
    if (toplevel_window == NULL)
    {
      return;
    }
    trace("have toplevel\n");

    Window *window = current_desktop->GetWindowByHWND(toplevel_window);
    if (!window)
    {
      return;
    }
    trace("toplevel known\n");

    int new_desk_number=Pager::GetBelowDesktopNumber(current_desktop->GetDesktopNumber()+1);
    trace("new desk: %d\n", new_desk_number);
    Desktop *new_desk = Pager::list.at(new_desk_number-1);
    Pager::MoveWindowFromToDesktop(current_desktop, new_desk, window);
    return;
  }
}

LRESULT Pager::MessageCalls(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  switch (uMessage)
  {
  case WM_TRAYNOTIFY:
    {
      MessageTrayNotify(lParam);
      return 0;
    }
    break;

  case WM_HOTKEY:
    {
      HotkeyPressed(wParam);
      return 0;
    }
    break;

  case WM_PAINT:
    {
      MessagePaint();
    }
    break;

  case WM_SETCURSOR: 
    {
      if (clicked_mini_window)
      {
        SetCursor(hand_cursor);
        return TRUE;
      }
    }
    break;

  case WM_LBUTTONDOWN:
    {
      MessageLeftMouseDown(lParam, wParam);
    }
    return 0;

  case WM_LBUTTONUP:
    {
      MessageLeftMouseUp(lParam);
    }
    return 0;

  case WM_CANCELMODE:
    {
      trace("wm_cancelmode\n");
      if (clicked_mini_window!=NULL)
      {
        Pager::MouseUpWithMiniWindow();
        clicked_desktop=NULL;
        return 0;
      }
    }
    break;

  case WM_MOUSEMOVE:
    {
      MessageMouseMove(lParam, wParam);
    }
    break;

  case WM_COMMAND:
    {
      MessageCommand(wParam);
    }
    return 0;

  case WM_CONTEXTMENU:
    { 
      MessageContextMenu(lParam, hWnd);
    }
    return 0;

  case MSG_ADOBE_SHIT:
    {
      MessageAdobeShit((HWND) wParam);
    }
    break;

  case MSG_MOVE_TO_WORKSPACE:
    {
      Pager::MoveWorkspaceEvent((HWND) wParam, (int) lParam, false);
    }
    break;
  case MSG_MOVE_TO_WORKSPACE_SHIFTED:
    {
      Pager::MoveWorkspaceEvent((HWND) wParam, (int) lParam, true);
    }
    break;

  case MSG_CREATE_MESSAGE_WINDOW:
    {
      CreatePagerMessageWindow();
    }
    break;

  case MSG_PROCESS_ATTACH:
    {
      DWORD pid= (DWORD) wParam;
      trace("msg_process_ATTACH: pid: %d\n", pid);
      Pager::AddPID(pid);
    } break;

  case MSG_PROCESS_DETACH:
    {
      DWORD pid= (DWORD) wParam;
      trace("msg_process_detach: pid: %d\n", pid);
      Pager::DeletePID(pid);
    } break;

  default:
    break;
  }

  return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

void Pager::NcActivateEvent(HWND win)
{
  LogMethod;


  // trace("nc activent direkt\n");

  Window *window = (Window *) Pager::GetWindowByHWNDAllDesks(win);
  if (!window)
  {
    return;
  }

  if (window->HasFlag("OnSecondaryMonitor"))
  {
    trace("ignore activating on secondary monitor\n");
    return;
  }

  if (window->GetDesktop()->GetDesktopNumber() == current_desktop->GetDesktopNumber())
  {
    return;
  }

  if (!window->HasFlag("button"))
  {
    return;
  }

  if (window->HasFlag("ignore"))
  {
    return;
  }

  if (window->HasFlag("flashing"))
  {
    trace("flashing already dected\n");
    return;
  }

  trace("nc activent\n");

  window->ProcedeNcActivateEvent();

}

bool Pager::WindowShowEvent(HWND win)
{
  LogMethod;

  //char wndClassName2[256];
  //GetClassName(win, wndClassName2, 256);
  //trace("show event window: %s,%x\n", wndClassName2, win);

  Window *window = (Window *) Pager::GetWindowByHWNDAllDesks(win);
  if (window)
  {
    trace("window show known\n");
    return false;
  }

  Window *window2 = (Window *) Pager::GetStickyWindowByHWND(win);
  if (!window2)
  {
    return current_desktop->MaybeAddWindow(win);
  }
  return false;

}

void Pager::WindowSelectorReady(void)
{
  LogMethod;

  Pager::UpdateIgnoreWindows();

  current_desktop->UpdateIgnoreStuff();

  Pager::RedrawWindow();
}


LRESULT Pager::HookCalls(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  LogMethod;

  if ((HWND) lParam == 0)
  {
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
  }

  switch (wParam)
  {

  case MSG_SHOW:
    {
      if (clicked_mini_window && 
        clicked_mini_window->GetRealWindowHWND() == (HWND) lParam)
      {
        break;
      }

      if (IsBadWindow((HWND) lParam))
      {
        break;
      }

      if (Pager::WindowShowEvent((HWND) lParam))
      {
        Pager::RedrawWindow();
      }
    }
    break;

  case MSG_HIDE:
    {

      // trace ("msg hide event:  %x\n", (HWND) lParam);

      if (IsBadWindow((HWND) lParam))
      {
        break;
      }

      Pager::WindowHideEvent((HWND) lParam);
    }
    break;

  case MSG_SIZED:
    {
      if (IsBadWindow((HWND) lParam))
      {
        break;
      }

      Pager::WindowSizeEvent((HWND) lParam);
    }
    break;

  case MSG_DESTROYED:
    {
      Pager::WindowDestroyEvent((HWND) lParam);
    }
    break;

  case MSG_TRAYSIZED:
    {
      if (incognito_mode)
        break;

      GetSys->Update();

      if (GetSys->TaskBarIsHorizontal())
      {
        Pager::UpdatePagerWindowPosHorizontalTaskbar(FALSE, TRUE);
      } else
      {
        Pager::UpdatePagerWindowPosVerticalTaskbar(FALSE, TRUE);
      }
      Pager::RedrawWindow();
    }
    break;

  case MSG_TOGGLESTICKY:
    {
      Pager::ToggleStickyEvent((HWND) lParam);
    }
    break;

  case MSG_NCACTIVATE:
    {
      if (IsBadWindow((HWND) lParam) || incognito_mode)
      {
        break;
      }
      Pager::NcActivateEvent((HWND) lParam);
    }
    break;

  case MSG_ACTIVATE:
    {
      if (IsBadWindow((HWND) lParam) || incognito_mode)
      {
        break;
      }

      Window *window = (Window *) current_desktop->GetWindowByHWND((HWND) lParam);
      if (window)
      {
        if (!window->HasFlag("button"))
        {
          //trace("activate non button %x\n", (HWND) lParam);
          Pager::WindowActivateEvent(window);
        }
      } else
      {
        Window *window2 = Pager::GetStickyWindowByHWND((HWND) lParam);
        if (window2)
        {
          if (!window2->HasFlag("button"))
          {
            Pager::SetActiveStickyWindow(window2);
          }
        }
      }
    }
    break;

  case MSG_WINDOW_SELECTOR:
    {
      Pager::WindowSelectorReady();
      trace("got message window selecotr\n");
    }
    break;

  }

  return DefWindowProc(hWnd, uMessage, wParam, lParam);
}

void Pager::WindowActivateEvent(Window *window)
{
  LogMethod;
  LogExtra("WindowActivateEvent: %x\n", window->GetRealWindowHWND());

  //trace("activate event %x\n", id);

  if (clicked_mini_window)
  {
    return;
  }

  if (window)
  {
    if (window->HasFlag("hangup"))
    {
      return;
    }

    if (window->HasFlag("ignore"))
    {
      return;
    }

    current_desktop->WindowActivated(window);
    return;
  }

}

void Pager::WindowActivateEvent( LPARAM lParam )
{
  LogMethod;

  Window *window = (Window *) current_desktop->GetWindowByHWND((HWND) lParam);
  if (window)
  {
    if (window->HasFlag("button") && !window->HasFlag("ignore"))
    {
      //trace("button activate: %x\n", (HWND) lParam);
      Pager::WindowActivateEvent(window);
    }
  } else
  {
    Window *window2 = Pager::GetStickyWindowByHWND((HWND) lParam);
    if (window2)
    {
      if (window2->HasFlag("button") && !window2->HasFlag("ignore"))
      {
        Pager::SetActiveStickyWindow(window2);
      }
    }
  }
}

void Pager::WindowCreateEvent(HWND win)
{
  LogMethod;


  Window *window2 = Pager::GetStickyWindowByHWND(win);
  if (window2)
  {
    return;
  }

  Window *window = (Window *) Pager::GetWindowByHWNDAllDesks(win);
  if (window)
  {
    return;
  }

  trace("create new toplevel: %x\n", win);
  if (!GetSys->WindowIsOwnedbyMe(win))
  {
    return;
  }

  Window *NewToplevel = new Window(current_desktop, win, true);
  if (NewToplevel->IsDead())
  {
    trace("new create window is death\n");
    delete NewToplevel;
    return;
  }

  current_desktop->AddWindow(NewToplevel);
  NewToplevel->UpdateMiniWindow();
  current_desktop->AddButton(win);
  current_desktop->SetActiveWindow(NewToplevel);

  //char wndClassName2[256];
  //if (GetClassName(win, wndClassName2, 256))
  //{
  //LogExtra("WindowCreateEvent new Toplevel Widnow: ID: %x class: %s\n", win, wndClassName2);
  //}


  if (!NewToplevel->HasFlag("iconic") && !incognito_mode)
  {
    Pager::RedrawWindow();
  }

}

void Pager::WindowDestroyEvent(HWND win)
{
  LogMethod;


  Window *window = Pager::GetWindowByHWNDAllDesks(win);
  if (window)
  {
    trace("window destroy event: %x\n", win);

    window->SystemMenuReset();

    if (clicked_mini_window_maybe!=NULL)
    {
      if (window->GetRealWindowHWND()==clicked_mini_window_maybe->GetRealWindowHWND())
      {
        clicked_mini_window_maybe=NULL;
        drag_timer_id?KillTimer(hwnd_main, drag_timer_id):0;
        drag_timer_id=0;
        maybe_window_destroyed=true;
      }
    }

    if (clicked_mini_window!=NULL)
    {
      if (window->GetRealWindowHWND()==clicked_mini_window->GetRealWindowHWND())
      {
        ReleaseCapture();
        ClipCursor(NULL);
        clicked_mini_window=NULL;
        clicked_mini_window_destroyed=true;
      }
    }

    if (window->HasFlag("hangup"))
    {
      trace("ignore hang up on destroy\n");
      return;
    }

    if (window->HasFlag("button"))
    {
      window->GetDesktop()->RemoveButton(win);
    }

    window->GetDesktop()->RemoveWindow(window, TRUE);

    !incognito_mode?Pager::RedrawWindow():0;
    return;
  }

  Window *window2 = Pager::GetStickyWindowByHWND(win);
  if (window2)
  {
    Pager::RemoveStickyWindow(window2);
    delete window2;
    !incognito_mode?Pager::RedrawWindow():0;
    return;
  }

}

UINT Pager::GetHookMessage()
{
  return m_hookMessage;
}

HWND Pager::GetPagerWindowHWND()
{
  return hwnd_main;
}

void Pager::GetRowandColumnbyNum(int num, int& row, int& col)
{
  LogMethod;

  num++;
  int rest=num%_cols;
  row=num/_cols - ((rest==0)?1:0) +1;
  col=rest?rest:_cols;
}

BOOL CALLBACK Pager::DoSomethingHelper(HWND hwnd, LPARAM lParam)
{
  WCHAR wndClassName[256];
  if (!GetClassNameW(hwnd, wndClassName, 256))
  {
    return true;
  }

  if (lstrcmpiW(L"ToolbarWindow32", wndClassName) == 0 ||
    lstrcmpiW(L"MSTaskListWClass", wndClassName) == 0 )
  {
    RECT quickLaunchRect;
    if (GetWindowRect(hwnd, &quickLaunchRect))
    {
      int panelHeight= (int) lParam;

      SetWindowPos(hwnd, NULL, 0, 0,
        quickLaunchRect.right-quickLaunchRect.left, panelHeight, SWP_NOMOVE|SWP_ASYNCWINDOWPOS);
    }
  }

  return TRUE;
}

void Pager::UpdateToolbars(int panelHeight)
{
  EnumChildWindows(GetSys->GetListWindow(), DoSomethingHelper, (LPARAM) panelHeight);
}

void Pager::UpdatePagerWindowPosHorizontalTaskbar(bool called_from_init, bool pagertoo)
{
  LogMethod;

  int panel_height = (((GetSys->GetPanelWindowRect().bottom -GetSys->GetPanelWindowRect().top)) / 10) * 10;
  trace("panel_height: %d\n", panel_height);
  if (panel_height < 30)
  {
    return;
  }

  int pager_padding_top = 0;
  if (GetWinVersion->IsVista()) {pager_padding_top = 1;}
  if (GetWinVersion->IsXP()) {pager_padding_top = 3;}

  int old_pager_height=pager_height;
  int old_pager_width=pager_width;

  if (GetWinVersion->IsWin2000() || GetWinVersion->IsXPClassic())
  {
    pager_height = panel_height - 8;
    cell_height=pager_height/_rows;
  }

  if (GetSys->GetScreenWidth()==0 || GetSys->GetScreenHeight()==0)
  {
    return;
  }

  if (GetWinVersion->IsVista() || GetWinVersion->IsXP())
  {
    pager_height = panel_height - pager_padding_top;
    cell_height=pager_height/_rows;

    _desktop_padding_horizontal=6;
    _desktop_padding_vertical=3;

    desktop_height = cell_height - _desktop_padding_vertical*2;
    desktop_width = (desktop_height * GetSys->GetScreenWidth()) / GetSys->GetScreenHeight();
    cell_width = desktop_width + _desktop_padding_horizontal*2;
  }else
  {
    _desktop_padding_horizontal=2;
    _desktop_padding_vertical=1;
    desktop_height = cell_height - 2 * FOCUS - 2 * SHADOW;
    desktop_width = (desktop_height * GetSys->GetScreenWidth()) / GetSys->GetScreenHeight();
    cell_width = desktop_width + 2 * SHADOW + 2 * FOCUS;
    pager_width = cell_width * num_desktops + (num_desktops - 1) * space;
  }

  if (GetSys->TaskBarEdge()==ABE_TOP)
  {
    pager_height-=1;
  }

  trace("dekstop width: %d\n", desktop_width);
  trace("pager_height: %d\n", pager_height);

  pager_width = cell_width * _cols + (_cols - 1) * space;

  trace("cell_width: %d\n", cell_width);
  trace("cell_height: %d\n", cell_height);
  trace("pager_widht: %d\n", pager_width);

  int old_pager_rel_left=pager_rel_left;
  int old_pager_rel_top=pager_rel_top;

  pager_rel_left=(GetSys->GetTrayWindowRect().left - pager_width) -
    ((GetWinVersion->IsWin2000() || GetWinVersion->IsXPClassic())? 6 : 0) - 2, pager_padding_top;
  pager_rel_top=pager_padding_top;

  if (called_from_init==TRUE)
  {
    return;
  }

  int list_window_width = pager_rel_left - GetSys->GetListWindowRect().left;
  int list_window_heigth = panel_height;

  SetWindowPos(GetSys->GetListWindow(), NULL, 0, 0,
    list_window_width, list_window_heigth, SWP_NOMOVE|SWP_ASYNCWINDOWPOS);

  if (GetWinVersion->IsSeven())
  {
    UpdateToolbars(panel_height);
  }

  if (pagertoo)
  {
    SetWindowPos(hwnd_main, NULL, pager_rel_left,
      pager_rel_top, pager_width, pager_height, 0);
  }

  if (old_pager_rel_left!=pager_rel_left ||
    old_pager_rel_top!=pager_rel_top ||
    old_pager_height!=pager_height ||
    old_pager_width!=pager_width)
  {
    trace("update minin windows\n");
    vector < Desktop * >::iterator iter = list.begin();
    while (iter != list.end())
    {
      (*iter)->UpdateGeo();
      (*iter)->UpdateMiniWindows();
      iter++;
    }

    Pager::UpdateStickyWindows();
  }
}

void Pager::UpdatePagerWindowPosVerticalTaskbar(bool called_from_init, bool pagertoo)
{
  LogMethod;

  int panel_width = GetSys->GetPanelWindowRect().right -GetSys->GetPanelWindowRect().left;
  trace("panel_width: %d\n", panel_width);
  if (panel_width < 62)
  {
    return;
  }

  int pager_padding_top = 0;
  int pager_padding_left=0;
  int pager_padding_right=0;

  if (GetSys->TaskBarEdge()==ABE_LEFT)
  {
    pager_padding_right=2;
  } else if (GetSys->TaskBarEdge()==ABE_RIGHT)
  {
    pager_padding_left=2;
  }

  pager_width = panel_width -pager_padding_left - pager_padding_right; 

  int old_pager_height=pager_height;
  int old_pager_width=pager_width;

  cell_width=pager_width/_cols;

  _desktop_padding_horizontal=6;
  _desktop_padding_vertical=3;

  desktop_width = cell_width - _desktop_padding_horizontal*2;

  if (GetSys->GetScreenWidth()==0)
  {
    return;
  }

  desktop_height = (desktop_width * GetSys->GetScreenHeight()) / GetSys->GetScreenWidth();

  cell_height = desktop_height + _desktop_padding_vertical*2;

  trace("dekstop height: %d\n", desktop_height);
  trace("pager_width: %d\n", pager_width);

  pager_height = cell_height * _rows;

  trace("cell_height: %d\n", cell_height);
  trace("cell_widht: %d\n", cell_width);
  trace("pager_height: %d\n", pager_height);

  int old_pager_rel_left=pager_rel_left;
  int old_pager_rel_top=pager_rel_top;

  pager_rel_top=GetSys->GetTrayWindowRect().top - pager_height;
  pager_rel_left=pager_padding_left;

  if (called_from_init==TRUE)
  {
    return;
  }

  int list_window_height = pager_rel_top - GetSys->GetListWindowRect().top;
  int list_window_width= panel_width;

  SetWindowPos(GetSys->GetListWindow(), NULL, 0, 0,
    list_window_width, list_window_height, SWP_NOMOVE|SWP_ASYNCWINDOWPOS);

  if (pagertoo)
  {
    SetWindowPos(hwnd_main, NULL, pager_rel_left,
      pager_rel_top, pager_width, pager_height, 0);
  }

  if (old_pager_rel_left!=pager_rel_left ||
    old_pager_rel_top!=pager_rel_top ||
    old_pager_height!=pager_height ||
    old_pager_width!=pager_width)
  {
    vector < Desktop * >::iterator iter = list.begin();
    while (iter != list.end())
    {
      (*iter)->UpdateGeo();
      (*iter)->UpdateMiniWindows();
      iter++;
    }

    Pager::UpdateStickyWindows();
  }
}

void Pager::ResetListwindow(void)
{
  LogMethod;

  if (incognito_mode)
    return;

  int list_window_height=0;
  int list_window_width=0;

  if (GetSys->TaskBarIsHorizontal())
  {

    list_window_height = GetSys->GetListWindowRect().bottom -
      GetSys->GetListWindowRect().top;

    list_window_width = GetSys->GetTrayWindowRect().left -
      GetSys->GetListWindowRect().left;
  } else
  {
    list_window_width = GetSys->GetListWindowRect().right -
      GetSys->GetListWindowRect().left;

    list_window_height = GetSys->GetTrayWindowRect().top -
      GetSys->GetListWindowRect().top;
  }

  SetWindowPos(GetSys->GetListWindow(), NULL, 0, 0, list_window_width,
    list_window_height, SWP_NOMOVE);
}

void Pager::DesktopToPager(Desktop * desktop, RECT & rect)
{
  LogMethod;

  rect.left += desktop->GetDesktopRect().left;
  rect.right += desktop->GetDesktopRect().left;
  rect.top += desktop->GetDesktopRect().top;
  rect.bottom += desktop->GetDesktopRect().top;
}

void Pager::PagerToDesktop(Desktop * desktop, RECT & rect)
{
  LogMethod;

  rect.left -= desktop->GetDesktopRect().left;
  rect.right -= desktop->GetDesktopRect().left;
  rect.top -= desktop->GetDesktopRect().top;
  rect.bottom -= desktop->GetDesktopRect().top;
}

Desktop *Pager::GetDesktopFromPoint(POINT pt)
{
  LogMethod;

  bool found = FALSE;

  vector < Desktop * >::iterator iter = list.begin();

  while (iter != list.end())
  {
    if ((*iter)->PointIsInside(pt))
    {
      found = TRUE;
      break;
    }

    iter++;
  }

  if (!found)
  {
    //MessageBox(hwnd_main, "no desktop from point", "ERROR", MB_ICONWARNING | MB_OK);
  }

  return found ? *iter : NULL;
}

void Pager::RedrawWindow(void)
{
  LogMethod;

  //trace("redraw window\n");
  InvalidateRect(hwnd_main, NULL, FALSE);
  UpdateWindow(hwnd_main);
}

Desktop *Pager::CurrentDesktop(void)
{
  LogMethod;

  return current_desktop;
}

vector < Desktop * >Pager::GetDesktopList(void)
{
  LogMethod;

  return list;
}

void Pager::SetCurrentDesktop(Desktop * desktop)
{
  LogMethod;

  current_desktop = desktop;
}

Window *Pager::GetWindowByHWNDAllDesks(HWND hwnd)
{
  LogMethod;

  Window *window = NULL;

  vector < Desktop * >::iterator iter = list.begin();
  while (iter != list.end())
  {
    window = (*iter)->GetWindowByHWND(hwnd);
    if (window)
    {
      break;
    }

    iter++;
  }

  return window;
}

HWND Pager::GetButtonHWNDAllDesks(HWND hwnd)
{
  LogMethod;

  HWND return_win = NULL;

  vector < Desktop * >::iterator iter = list.begin();

  while (iter != list.end())
  {
    return_win = (*iter)->GetButtonByHWND(hwnd);
    if (return_win)
    {
      break;
    }

    iter++;
  }

  return return_win;
}

int Pager::GetNumberOfDesktops()
{
  LogMethod;

  return num_desktops;
}

bool Pager::IsBadWindow(HWND window)
{
  try
  {
    LogMethod;

    if (window == 0 || !IsWindow(window))
    {
      return TRUE;
    }

    WCHAR wndClassName[256];
    if (!GetClassNameW(window, wndClassName, 256))
    {
      return TRUE;
    }

    if (GetProp(window, "PAGER_IGNORE"))
    {
      trace("found pager_ignore inisbadwdinow\n");
      return true;
    }

    //trace("class: %s\n", wndClassName);


    if (!bad_classes.empty())
    {
      wstring theclass(wndClassName);
      bool found_bad=binary_search(bad_classes.begin(), bad_classes.end(), theclass);
      if (found_bad)
      {
        //trace("found bad class: %s\n", theclass.c_str());
        return TRUE;
      }
    }

    if (GetWinVersion->IsVista())
    {
      if (wcscmp(wndClassName, L"Button") == 0)
      {
        if (GetParent(window) == GetSys->GetPanelWindow())
        {
          if (GetProp(window, "StartButtonTag"))
          {
            return 1;
          }
        }
      }
    }

  } catch (...)
  {
    return TRUE;
  }

  return FALSE;
}

void Pager::MoveWindowFromToDesktop(Desktop *current_desktop, Desktop *new_desktop, Window *window)
{
  LogMethod;

  current_desktop->RemoveWindow(window, FALSE);

  HWND owner = GetWindow(window->GetRealWindowHWND(), GW_OWNER);
  if (owner && owner != window->GetRealWindowHWND())
  {
    Window *owni=current_desktop->GetWindowByHWND(owner);
    if (owni && owni->HasFlag("ignore"))
    {
      trace("move desktop owner have a window (owner= %x\n", owner);
      current_desktop->RemoveButton(owner);
      current_desktop->RemoveWindow(owni, FALSE);
      owni->SetExStyle(WS_EX_NOACTIVATE, 200);
      owni->SetDisabled(200);
      owni->SetWindowPosFake();
      GetSys->HideTaskButton(owner);
      new_desktop->AddButton(owner);
      new_desktop->AddWindow(owni);
    }
  } else
  {
    current_desktop->RemoveButton(window->GetRealWindowHWND());
    new_desktop->AddButton(window->GetRealWindowHWND());

    GetSys->HideTaskButton(window->GetRealWindowHWND());
  }

  new_desktop->AddWindow(window);
  window->Hide();
  window->UpdateMiniWindow();
  window->SystemMenuUpdate();

  new_desktop->SetActiveWindow(window);
  !incognito_mode?Pager::RedrawWindow():0;
}

void Pager::MoveWorkspaceEvent(HWND win, int new_desk, bool switchDesktop)
{
  LogMethod;

  trace("move workspace event: win: %x, new_desk: %d\n", win, new_desk);

  Desktop *new_desktop = list.at(new_desk - 1);
  if (!new_desk)
  {
    return;
  }

  Window *window = current_desktop->GetWindowByHWND(win);
  if (!window)
  {
    return;
  }

  Pager::MoveWindowFromToDesktop(current_desktop, new_desktop, window);

  if (switchDesktop)
  {
    SwitchToDesktop(new_desktop);
  }
}

void Pager::AddStickyWindow(Window * window)
{
  LogMethod;

  sticky_list.push_back(window);
}

void Pager::RemoveStickyWindow(Window * window)
{
  LogMethod;

  vector < Window * >::iterator iter = sticky_list.begin();
  while (iter != sticky_list.end())
  {
    if ((*iter)->GetRealWindowHWND() == window->GetRealWindowHWND())
    {
      iter = sticky_list.erase(iter);
      break;
    }
    iter++;
  }

}

Window *Pager::GetStickyWindowByHWND(HWND hwnd)
{
  LogMethod;

  Window *return_window = NULL;

  vector < Window * >::iterator iter = sticky_list.begin();
  while (iter != sticky_list.end())
  {
    if ((*iter)->GetRealWindowHWND() == hwnd)
    {
      return_window = *iter;
      break;
    }
    iter++;
  }

  return return_window;
}

void Pager::SetActiveStickyWindow(Window * window)
{
  LogMethod;

  if (window != NULL)
  {
    vector < Window * >::iterator iter = sticky_list.begin();
    while (iter != sticky_list.end())
    {
      if (((*iter)->GetRealWindowHWND()) == window->GetRealWindowHWND())
      {
        iter = sticky_list.erase(iter);
        break;
      }
      iter++;
    }

    sticky_list.push_back(window);
  }

  if (!IsIconic(window->GetRealWindowHWND()) && !GetPager->UseIncognitoMode())
  {
    GetPager->RedrawWindow();
  }
}

void Pager::NormalToSticky( Window * window )
{
  LogMethod;

  current_desktop->RemoveWindow(window, FALSE);
  if (!sticky_window_button)
  {
    window->SetExStyle(WS_EX_TOOLWINDOW, 200);
    window->SetWindowPosFake();
  }

  HWND owner = GetWindow(window->GetRealWindowHWND(), GW_OWNER);
  if (owner && owner != window->GetRealWindowHWND())
  {
    Window *owni=current_desktop->GetWindowByHWND(owner);
    if (owni && owni->HasFlag("ignore"))
    {
      trace("sticky window have owner\n");
      current_desktop->RemoveButton(owner);
      current_desktop->RemoveWindow(owni, FALSE);

      if (!sticky_window_button)
      {
        owni->SetExStyle(WS_EX_TOOLWINDOW, 200);
        window->SetWindowPosFake();
        GetSys->HideTaskButton(owner);
      }
      Pager::AddStickyWindow(owni);
    }
  } else
  {
    current_desktop->RemoveButton(window->GetRealWindowHWND());
    !sticky_window_button?GetSys->HideTaskButton(window->GetRealWindowHWND()):0;
  }

  Pager::AddStickyWindow(window);

  window->UpdateAlwaysVisible(true);
  window->SystemMenuUpdate();

  !incognito_mode?Pager::RedrawWindow():0;
}

void Pager::StickyToNormal(Window * window)
{
  LogMethod;

  bool on_2=false;

  if (GetSys->GetMultiMonitorMode()==PAGER_ON_PRIMARY &&
    GetSys->GetMonitor(window->GetRealWindowHWND())==SECONDARY_MONITOR)
  {
    trace("window is on secondary monitor after sticky\n");
    window->SetFlag("OnSecondaryMonitor");
    on_2=true;
  }
  Pager::RemoveStickyWindow(window);
  current_desktop->AddWindow(window);

  if (on_2)
  {
    if (sticky_window_button)
    {
      window->SetExStyle(WS_EX_TOOLWINDOW, 200);
      window->SetWindowPosFake();
    }
  } else
  {
    if (!sticky_window_button)
    {
      window->RemoveExStyle(WS_EX_TOOLWINDOW, 200);
      window->SetWindowPosFake();
    }
  }

  HWND owner = GetWindow(window->GetRealWindowHWND(), GW_OWNER);
  if (owner && owner != window->GetRealWindowHWND())
  {
    window->GetDesktop()->AddButton(owner);
    Window *owni = Pager::GetStickyWindowByHWND(owner);
    if (owni && owni->HasFlag("ignore"))
    {
      trace("old stick has owner\n");
      Pager::RemoveStickyWindow(owni);
      current_desktop->AddWindow(owni);
      current_desktop->AddButton(owner);

      if (on_2)
      {
        if (sticky_window_button)
        {
          owni->SetExStyle(WS_EX_TOOLWINDOW, 200);
          owni->SetWindowPosFake();
          GetSys->HideTaskButton(owner);
        }

      } else
      {
        if (!sticky_window_button)
        {
          owni->RemoveExStyle(WS_EX_TOOLWINDOW, 200);
          owni->SetWindowPosFake();
          GetSys->ShowTaskButton(owner);
        }
      }
    }
  } else
  {
    current_desktop->AddButton(window->GetRealWindowHWND());
    if (on_2)
    {
      sticky_window_button?GetSys->HideTaskButton(window->GetRealWindowHWND()):0;
    } else
    {
      !sticky_window_button?GetSys->ShowTaskButton(window->GetRealWindowHWND()):0;
    }
  }

  window->UpdateMiniWindow();
  current_desktop->SetActiveWindow(window);

  window->UpdateAlwaysVisible(false);

  window->SystemMenuUpdate();

  !incognito_mode?Pager::RedrawWindow():0;
}

void Pager::ToggleStickyEvent(HWND win)
{
  LogMethod;

  trace("toggle sticky menu: %x\n", win);

  Window *window = current_desktop->GetWindowByHWND(win);
  if (window)
  {
    NormalToSticky(window);
    return;
  }

  Window *window2 = Pager::GetStickyWindowByHWND(win);
  if (window2)
  {
    StickyToNormal(window2);
  }
}

void Pager::WindowSizeEvent(HWND win)
{
  LogMethod;


  if (clicked_mini_window && clicked_mini_window->GetRealWindowHWND()==win)
  {
    return;
  }

  Window *window = Pager::GetWindowByHWNDAllDesks(win);
  if (window)
  {
    if (window->SizeEvent())
    {
      !incognito_mode?Pager::RedrawWindow():0;
    }

    return;
  }

  if (!incognito_mode)
  {
    Window *window2 = Pager::GetStickyWindowByHWND(win);
    if (window2)
    {

      if (GetSys->GetMultiMonitorMode()==PAGER_ON_PRIMARY)
      {
        int monitor=GetSys->GetMonitor(window2->GetRealWindowHWND());
        if (monitor== PRIMARY_MONITOR && window2->HasFlag("OnSecondaryMonitor"))
        {
          trace("sticky secondary to primary.............................\n");
          window2->DeleteFlag("OnSecondaryMonitor");
        } else if (monitor==SECONDARY_MONITOR && !window2->HasFlag("OnSecondaryMonitor"))
        {
          trace("sticky primary to secondary.........................\n");
          window2->SetFlag("OnSecondaryMonitor");
        }
      } 

      window2->UpdateMiniWindow();
      Pager::RedrawWindow();
      return;
    }
  }
}

void Pager::WindowHideEvent(HWND win)
{
  LogMethod;

  Window *window = Pager::GetWindowByHWNDAllDesks(win);
  if (window)
  {
    trace ("window hide event: %x\n", win);
    if (clicked_mini_window_maybe!=NULL)
    {
      if (window->GetRealWindowHWND()==clicked_mini_window_maybe->GetRealWindowHWND())
      {
        clicked_mini_window_maybe=NULL;
        drag_timer_id?KillTimer(hwnd_main, drag_timer_id):0;
        drag_timer_id=0;
        maybe_window_destroyed=true;
      }
    }

    if (clicked_mini_window!=NULL)
    {
      if (window->GetRealWindowHWND()==clicked_mini_window->GetRealWindowHWND())
      {
        ReleaseCapture();
        ClipCursor(NULL);
        clicked_mini_window=NULL;
        clicked_mini_window_destroyed=true;
      }
    }

    if (window->HasFlag("hangup"))
    {
      trace("ignore hang up on hide\n");
      return;
    }

    window->SystemMenuReset();

    if (window->GetDesktop()->GetDesktopNumber() != current_desktop->GetDesktopNumber())
    {
      trace("hiding window is on foregn desk\n");
      window->RemoveNoactivateStyle();
      window->RemoveDisabled(200);
    }

    if (window->HasFlag("button"))
    {
      window->GetDesktop()->RemoveButton(win);
    }

    Desktop *tmp = window->GetDesktop();

    trace("remove widnow: %x\n", window->GetRealWindowHWND());
    tmp->RemoveWindow(window, TRUE);
    !incognito_mode?Pager::RedrawWindow():0;
    return;
  }

  if (!incognito_mode)
  {
    Window *window2 = Pager::GetStickyWindowByHWND(win);
    if (window2)
    {
      Pager::RedrawWindow();
      return;
    }
  }
}

void Pager::DeleteButtonAllDesks(HWND win)
{
  LogMethod;

  vector < Desktop * >::iterator iter = list.begin();
  while (iter != list.end())
  {
    (*iter)->RemoveButton(win);
    iter++;
  }
}

UINT Pager::GetShellMessageID()
{
  return shellMessage;
}

void Pager::SwitchToDesktop(Desktop * new_desk)
{
  LogMethod;


  Desktop *old_desk = current_desktop;
  current_desktop = new_desk;

  //osd->HideOSD();

  !incognito_mode?new_desk->StopFlash():0;

  old_desk->Hide();

  if (old_desk->GetWallpaperPath()==new_desk->GetWallpaperPath())
  {
    // trace("old desk gleich new desk\n");
  } else
  {
    new_desk->ChangeWallpaper();
  }

  new_desk->Show();

  if (incognito_mode)
  {
    tray->SetIcon(new_desk->GetDesktopNumber()+1, new_desk->GetTrayIcon());
  }

  !incognito_mode?Pager::RedrawWindow():0;
}

void Pager::ShowHelpDialog(void)
{
  LogMethod;

  if (!GetSys->GotoURL("http://windowspager.sourceforge.net/documentation.html"))
  {
    GetSys->ShowMessageBoxModless(
      "Unable to open\n\"http://windowspager.sourceforge.net/documentation.html\".",
      MB_ICONERROR);
  }
}

void Pager::MessageAdobeShit(HWND win)
{
  LogMethod;

  Window *window = Pager::GetWindowByHWNDAllDesks(win);
  if (window)
  {
    int x=(int) GetProp(win, "PAGER_ICONIC_X");
    int y=(int) GetProp(win, "PAGER_ICONIC_Y");
    int w=(int) GetProp(win, "PAGER_ICONIC_W");
    int h=(int) GetProp(win, "PAGER_ICONIC_H");

    trace("got adobe shit message win: %x: x: %d, y: %d, w:%d, h:%d\n", win, x , y,w,h);

    RECT rect;
    SetRect(&rect, x, y, x+w, y+h);
    window->SetBackupRect(rect);
    window->DeleteFlag("iconic");

    RemoveProp(win, "PAGER_ICONIC");
    RemoveProp(win, "PAGER_ICONIC_X");
    RemoveProp(win, "PAGER_ICONIC_Y");
    RemoveProp(win, "PAGER_ICONIC_W");
    RemoveProp(win, "PAGER_ICONIC_H");

    !incognito_mode?window->UpdateMiniWindow():0;
    !incognito_mode?Pager::RedrawWindow():0;
  }
}

void Pager::MessageTrayNotify( LPARAM lParam )
{
  LogMethod;

  switch ( lParam )
  {
  case WM_RBUTTONUP:
    {    
      POINT loc;
      GetCursorPos( &loc );
      trace("before popup\n");
      int cmd = TrackPopupMenu(tray->GetTrayMenu(), TPM_CENTERALIGN| TPM_VERNEGANIMATION,
        loc.x, loc.y, 0, hwnd_main, NULL);
    }
    return;

  default:
    break;
  }
}

void Pager::MessagePaint()
{
  LogMethod;

  if (!incognito_mode)
  {
    HDC hdc;
    PAINTSTRUCT ps;
    hdc = BeginPaint(hwnd_main, &ps);
    Graphics graphics(hdc);
    Bitmap bmp(pager_width, pager_height);
    Graphics* buffer_graph = Graphics::FromImage(&bmp);
    Pager::OnPaint(buffer_graph);
    graphics.DrawImage(&bmp, 0,0);
    delete buffer_graph;
    EndPaint(hwnd_main, &ps);
  }
}

void Pager::MessageLeftMouseDown( LPARAM lParam, WPARAM wParam )
{
  LogMethod;

  if (incognito_mode) return;

  POINT pt;
  pt.x = (LONG) LOWORD(lParam);
  pt.y = (LONG) HIWORD(lParam);

  trace("wparam: %x\n", wParam);
  short state=GetKeyState(VK_LWIN);
  if ((wParam & MK_CONTROL) && (GetKeyState(VK_LWIN) < 0))
  {
    //
  } else
  {
    Pager::MouseDown(pt);
  }
}

void Pager::MessageLeftMouseUp( LPARAM lParam )
{
  LogMethod;

  if (incognito_mode) return;

  clicked_mini_window_maybe=NULL;
  drag_timer_id?KillTimer(hwnd_main, drag_timer_id):0;
  drag_timer_id=0;

  if (maybe_window_destroyed)
  {
    maybe_window_destroyed=false;
    return;
  }

  if (clicked_mini_window_destroyed)
  {
    clicked_mini_window_destroyed=false;
    return;
  }

  POINT pt;
  pt.x = (LONG) LOWORD(lParam);
  pt.y = (LONG) HIWORD(lParam);

  if (clicked_mini_window)
  {
    Pager::MouseUpWithMiniWindow();
    clicked_desktop=NULL;
    return;
  } else
  {
    Pager::MouseUpNoMiniWindow(pt);
    clicked_desktop=NULL;
    return;
  }
}

void Pager::MessageMouseMove( LPARAM lParam, WPARAM wParam )
{
  LogMethod;

  if (incognito_mode) return;

  POINT pt;
  pt.x = (LONG) LOWORD(lParam);
  pt.y = (LONG) HIWORD(lParam);

  //trace("mouse x: %d, mouse y: %d\n", pt.x, pt.y);

  if (clicked_mini_window_destroyed || maybe_window_destroyed)
  {
    return;
  }

  if (clicked_mini_window && (wParam & MK_LBUTTON)) 
  {
    Pager::MouseMoveWithMiniWindow(pt);
  } else
  {
    Pager::MouseMoveNoMiniWindow(pt);
  }
}

void Pager::MessageContextMenu( LPARAM lParam, HWND hWnd )
{
  LogMethod;

  trace("wmcontext menu\n");

  if (clicked_mini_window || clicked_mini_window_maybe)
  {
    trace("rect wm_contextmenu\n");
    return;
  }

  if (incognito_mode) return;

  POINT pt;
  pt.x = (LONG) LOWORD(lParam);
  pt.y = (LONG) HIWORD(lParam);
  int cmd = TrackPopupMenu(menu, TPM_LEFTALIGN,
    pt.x, pt.y, 0, hWnd, NULL);
}

void Pager::MessageCommand( WPARAM wParam )
{
  LogMethod;

  trace("wm_command: %d\n", LOWORD(wParam));
  if (LOWORD(wParam) > MENUSWITCHTO && LOWORD(wParam)<= MENUSWITCHTO+num_desktops)
  {
    trace("tray switchto event\n");
    int number=LOWORD(wParam)-MENUSWITCHTO-1;
    if (number == current_desktop->GetDesktopNumber())
    {
      return;
    }
    Desktop *new_desk = Pager::list.at(number);
    Pager::SwitchToDesktop(new_desk);

    return;
  }

  switch (LOWORD(wParam))
  {
  case MENUQUIT:
    {
      RestoreWindowsBeforeExit();
      delete GetPager;
      PostQuitMessage(0);
    }
    return;

  case MENUHELP:
    {
      ShowHelpDialog();
    }
    return;

  case MENUDOWNLOAD:
    {
      if (!GetSys->GotoURL("http://sourceforge.net/projects/windowspager/"))
      {
        GetSys->ShowMessageBoxModless(
          "Unable to open\n\"http://sourceforge.net/projects/windowspager/\".",
          MB_ICONERROR);
      }
    }
    return;

  case MENUABOUT:
    {

#ifdef _WIN64

      GetSys->ShowMessageBoxModless("WindowsPager Version: " WPVERSION " 64 Bit\n"
        "(C) 2007, 2008, 2009, 2010, 2011 Jochen Baier, email@Jochen-Baier.de\n",
        MB_ICONINFORMATION);
#else
      GetSys->ShowMessageBoxModless("WindowsPager Version: " WPVERSION " 32 Bit\n"
        "(C) 2007, 2008, 2009, 2010, 2011 Jochen Baier, email@Jochen-Baier.de\n",
        MB_ICONINFORMATION);
#endif

    }
    return;

  case MENUSELECTOR:
    {
      trace("seletor selected\n");
      WindowSelector *ws= new WindowSelector();
    }
    return;

  default:
    break;
  }
}

bool Pager::StickyWindowsHaveButtons( void )
{
  LogMethod;

  return sticky_window_button;
}

ATOM Pager::GetHookAtom( void )
{
  LogMethod;

  return hookatom;
}

bool Pager::UseHungUpInfoWindow( void )
{
  LogMethod;

  return hung_up_information_window;
}

bool Pager::UseIncognitoMode( void )
{
  LogMethod;

  return incognito_mode;
}

bool Pager::RememberActiveWindow(void)
{
  LogMethod;

  return rememberActiveWindow;
}

bool Pager::InitReady( void )
{
  return init_ready;
}

void Pager::Key2ValueInsert( UINT_PTR key, UINT_PTR value )
{
  LogMethod;

  key2value.insert(std::make_pair(key, value));
}

UINT_PTR Pager::Key2ValueGetValue( UINT_PTR id )
{
  LogMethod;

  std::map <UINT_PTR, UINT_PTR>::iterator iter =key2value.find(id);
  if (iter != key2value.end())
  {
    return iter->second;
  } else
  {
    return 0;
  }
}

void Pager::Key2ValueDelete( UINT_PTR key )
{
  LogMethod;

  key2value.erase(key);
}

UINT_PTR Pager::CreateUniqueTimerID( void )
{
  LogMethod;

  if (unique_id == 100000001)
  {
    unique_id=0;
  }

  return ++unique_id;
}

void Pager::SetDragTimerDesktop( int desk )
{
  LogMethod;

  _timer_set_on_desk=desk;
}

int Pager::GetDragTimerDesktop( void )
{
  LogMethod;

  return _timer_set_on_desk;
}

int Pager::GetSpace( void )
{
  LogMethod;

  return space;
}

int Pager::GetCellHeight( void )
{
  LogMethod;

  return cell_height;
}

int Pager::GetCellWidth( void )
{
  LogMethod;

  return cell_width;
}

int Pager::GetDesktopHeight( void )
{
  LogMethod;

  return desktop_height;
}

int Pager::GetDesktopWidth( void )
{
  LogMethod;

  return desktop_width;
}

int Pager::GetPagerWidth( void )
{
  LogMethod;

  return pager_width;
}

int Pager::GetPagerHeight( void )
{
  LogMethod;

  return pager_height;
}

int Pager::GetCols( void )
{
  LogMethod;

  return _cols;
}

int Pager::GetRows( void )
{
  LogMethod;

  return _rows;
}

int Pager::GetPagerPaddingHorizontal( void )
{
  LogMethod;

  return _desktop_padding_horizontal;
}

int Pager::GetPagerPaddingVertical( void )
{
  LogMethod;

  return _desktop_padding_vertical;
}

int Pager::CrashReportingOption(void)
{
  return crashReportingOption;
}
