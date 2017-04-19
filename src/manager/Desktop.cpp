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

#include "..\common\stdafx.h"
#include <commctrl.h>

#include "XWinVer.h"
#include "System.h"
#include "Desktop.h"
#include "Window.h"
#include "Pager.h"
#include "SimpleTray.h"

using namespace std;
using namespace boost;
using namespace Gdiplus;

Desktop::Desktop(int _desktop_number)
{
  LogMethod;

  number = _desktop_number;
  preview_row=0;
  preview_column=0;
  flash_timer_id=0;
  list_zorder.clear();
  list_torder.clear();
  tray_icon=NULL;
  activeWindow=NULL;

  WCHAR key[20];
  swprintf_s (key, 20, L"desktop%d", number+1);
  wstring key_string(key);

  {
    UINT modifier_id=GetSys->GetIniFile()->GetHotkeyModifier(L"HotkeysDesktopSwitch");
    // trace("modifier switch: %d\n", modifier_id);

    UINT vcode=GetSys->GetIniFile()->GetHotkeyKey(number+1, L"HotkeysDesktopSwitch", key_string);
    if (vcode!=-1)
    {
      char buf[30];
      sprintf_s(buf, 30, "PAGER_HOTKEY_DESKTOP_%d", number+1);
      hotkeyatom = GlobalAddAtom(buf);
      RegisterHotKey(GetPager->GetPagerWindowHWND(), hotkeyatom, modifier_id, vcode);
    }
  }

  {
    UINT modifier_id=GetSys->GetIniFile()->GetHotkeyModifier(L"HotkeysMoveToDesktop");
    UINT vcode=GetSys->GetIniFile()->GetHotkeyKey(number+1, L"HotkeysMoveToDesktop", key_string);
    if (vcode!=-1)
    {
      char buf[35];
      sprintf_s(buf, 35, "PAGER_HOTKEY_MOVE_TO_DESKTOP_%d", number+1);
      hotkeyatom_move_to = GlobalAddAtom(buf);
      RegisterHotKey(GetPager->GetPagerWindowHWND(), hotkeyatom_move_to, modifier_id, 
        vcode);
    }
  }


  wallpaper_path=GetSys->GetIniFile()->GetKeyValue(L"Wallpaper", key);
  if (wallpaper_path==L"error")
  {
    wallpaper_path=L"default";
  } 

  if (wallpaper_path!=L"default" && !GetSys->CheckIfFileExists(wallpaper_path))
  {
    WCHAR buf[256];
    swprintf_s(buf, 256, L"No valid wallpaper path for key '%s' in 'windowspager.ini'. "\
      L"\nPlease repair the 'windowspager.ini'. Will quit now.", key);
    MessageBoxW(0, buf, L"WindowsPager", MB_ICONHAND);
    exit(1);
  }

  flash_status=false;
  flashing=false;

  if (GetPager->UseIncognitoMode())
  {
    tray_icon=CSimpleTray::CreateIconFromNumber(number+1);
  }

  UpdateGeo();
}

Desktop::~Desktop()
{
  LogMethod;

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    delete *iter;
    iter++;
  }

  if (tray_icon)
  {
    DestroyIcon(tray_icon);
  }

  list_zorder.clear();
  list_torder.clear();

  UnregisterHotKey(GetPager->GetPagerWindowHWND(), hotkeyatom);
  GlobalDeleteAtom(hotkeyatom);
  UnregisterHotKey(GetPager->GetPagerWindowHWND(), hotkeyatom_move_to);
  GlobalDeleteAtom(hotkeyatom_move_to);
}

void Desktop::UpdateGeo(void)
{
  LogMethod;

  if (GetPager->UseIncognitoMode())
  {
    return;
  }

  GetPager->GetRowandColumnbyNum(number, preview_row, preview_column);
  int left, top;

  do
  {
    if (preview_column==1)
    {
      left=0;
      break;
    }

    if (preview_column==2)
    {
      left = GetPager->GetCellWidth()-1 +  GetPager->GetSpace() +1;
      break;
    }
    left = (preview_column-1) * GetPager->GetCellWidth() + (preview_column-1) * GetPager->GetSpace();
    break;
  }while(1);

  do
  {
    if (preview_row==1)
    {
      top=0;
      break;
    }

    if (preview_row==2)
    {
      top = GetPager->GetCellHeight(); 
      break;
    }
    top = (preview_row-1) * GetPager->GetCellHeight() - 1; 

    break;
  }while(1);

  SetRect(&cell_rect, left, top,  
    (preview_column==GetPager->GetCols())?
    GetPager->GetPagerWidth()-1: (left + GetPager->GetCellWidth()-1), 
    (preview_row==GetPager->GetRows())?
    GetPager->GetPagerHeight()-1:(top+ GetPager->GetCellHeight() - 1));

  if (GetWinVersion->IsVista() || GetWinVersion->IsXP())
  {
    SetRect(&desk_rect, cell_rect.left +GetPager->GetPagerPaddingHorizontal(),
      cell_rect.top +GetPager->GetPagerPaddingVertical(),
      cell_rect.right -GetPager->GetPagerPaddingHorizontal(),
      cell_rect.bottom - GetPager->GetPagerPaddingVertical());
  }else
  {
    SetRect(&desk_rect, cell_rect.left + FOCUS + SHADOW, cell_rect.top + FOCUS + SHADOW,
      cell_rect.right - SHADOW - FOCUS, cell_rect.bottom - SHADOW - FOCUS);
  }

  do
  {
    if (preview_column==GetPager->GetCols())
    {
      SetRect(&mouse_rect, cell_rect.left, cell_rect.top,
        cell_rect.right, cell_rect.bottom);
      break;
    }

    SetRect(&mouse_rect, cell_rect.left, cell_rect.top,
      cell_rect.right +GetPager->GetSpace(), cell_rect.bottom);
    break;
  }
  while (1);

}

RECT Desktop::GetDesktopRect()
{
  LogMethod;

  return desk_rect;
}

bool Desktop::PointIsInside(POINT pt)
{
  LogMethod;

  return GetSys->PointIsInsideRect(mouse_rect, pt);
}

void Desktop::SetActiveWindow(Window * window)
{
  LogMethod;

  bool window_found = FALSE;

  if (window == NULL  || GetPager->UseIncognitoMode())
  {
    return;
  }

  activeWindow=NULL;

  if (window->GetRealWindowHWND()!=GetForegroundWindow())
  {
    //trace("active event but not forgournd window: %x\n", window);
    return;
  }

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    if ((*iter)->HasFlag("ignore"))
    {
      iter++;
      continue;
    }

    if (((*iter)->GetRealWindowHWND()) == window->GetRealWindowHWND())
    {
      iter = list_zorder.erase(iter);
      window_found = TRUE;
      break;
    }

    iter++;
  }

  if (window_found)
  {
    list_zorder.push_back(window);
    activeWindow=window->GetRealWindowHWND();
   // trace("new active window: %x\n", activeWindow);
  } 

}

void Desktop::AddWindow(Window * window)
{
  LogMethod;

  window->SetDesktop(this);
  Desktop::list_zorder.push_back(window);
}

void Desktop::AddButton(HWND window)
{
  LogMethod;

  Desktop::list_torder.push_back(window);
}

bool Desktop::IsButton(HWND window)
{
  LogMethod;

  bool is = FALSE;

  vector < HWND >::iterator iter = list_torder.begin();
  while (iter != list_torder.end())
  {
    if ((*iter) == window)
    {
      is = TRUE;
      break;
    }
    iter++;
  }

  return is;
}

void Desktop::RemoveButton(HWND window)
{
  LogMethod;

  vector < HWND >::iterator iter = list_torder.begin();
  while (iter != list_torder.end())
  {
    if ((*iter) == window)
    {
      iter = list_torder.erase(iter);
      break;
    }
    iter++;
  }
}

void Desktop::RemoveWindow(Window * window, bool deleteit)
{
  LogMethod;

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    if (((*iter)->GetRealWindowHWND()) == window->GetRealWindowHWND())
    {
      if (deleteit)
      {
        delete(*iter);
      }

      iter = list_zorder.erase(iter);
      break;
    }
    iter++;
  }
}

void Desktop::RestoreWindowsBeforeExitOnHidden(void)
{
  LogMethod;

  vector < HWND >::iterator witer = list_torder.begin();
  while (witer != list_torder.end())
  {
    GetSys->ShowTaskButton(*witer);
    witer++;
  }

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    (*iter)->ShowOnExit();
    iter++;
  }
}

void Desktop::UpdateIgnoreStuff(void)
{
  LogMethod;

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    wstring *exe_compl=(*iter)->GetFileName();
    if (exe_compl==NULL)
    {
      ++iter;
      continue;
    }

    int last_backslash=exe_compl->find_last_of('\\');
    if (last_backslash==wstring::npos)
    {
      ++iter;
      continue;
    }

    wstring exe_name=exe_compl->substr(last_backslash+1);
    to_lower(exe_name);

    wstring *class_string=(*iter)->GetClassString();
    if (exe_name.empty() || class_string==NULL)
    {
      ++iter;
      continue;
    }

    if (GetPager->CheckIfOnIgnoreList(exe_name, *class_string))
    {
      trace("window %x is now on ignore, delete it.\n", (*iter)->GetRealWindowHWND());
      SetProp((*iter)->GetRealWindowHWND(), "PAGER_IGNORE", (void*) 1);
      if ((*iter)->HasFlag("button"))
      {
        (*iter)->GetDesktop()->RemoveButton((*iter)->GetRealWindowHWND());
      }
      (*iter)->RemoveSubclassOrHook();

	  (*iter)->SystemMenuReset();

      delete(*iter);
      iter = list_zorder.erase(iter);
      continue;
    }
    iter++;
  }
}

void Desktop::RestoreWindowsBeforeExitOnCurrent(void)
{
  LogMethod;

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    (*iter)->RestoreOnExit();
    iter++;
  }
}

void Desktop::ShowTaskbarButtons(void)
{
  LogMethod;

  if (!list_torder.empty())
  {
    vector < HWND >::iterator witer = list_torder.begin();
    while (witer != list_torder.end())
    {
      if (!IsWindow(*witer) || IsHungAppWindow(*witer))
      {
        witer++;
        continue;
      }

      Window *windi=Desktop::GetWindowByHWND(*witer);
      if (!windi)
      {
        witer++;
        continue;
      }

      windi->ResetFlashing();

      if (windi->HasFlag("hangup"))
      {
        trace("ignoreing hangup window during show tasklistbutton %x\n", *witer);
        witer++;
        continue;
      }

      if (GetSys->GetMultiMonitorMode()==PAGER_ON_PRIMARY)
      {
        if (windi->HasFlag("OnSecondaryMonitor"))
        {
          trace("ignoreing OnSecondaryMonitor during show tasklistbutton\n");
          witer++;
          continue;
        }
      }

      GetSys->ShowTaskButton(*witer);

      witer++;
    }
  }
}

void Desktop::Show()
{
  LogMethod;
  LogExtra("Show() Desktop ID: %x\n", number);

  if (list_zorder.empty())
  {
    return;
  }

  Desktop::ShowTaskbarButtons();

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    if ((*iter)->HasFlag("hangup"))
    {
      trace("ignore hangup on show: %x\n", (*iter)->GetRealWindowHWND());
      iter++;
      continue;
    }

    if ((*iter)->DeathCheckPositiv())
    {
      if ((*iter)->HasFlag("button"))
      {
        Desktop::RemoveButton((*iter)->GetRealWindowHWND());
      } 
      delete *iter;
      iter = list_zorder.erase(iter);
      continue;
    }

    if (GetSys->GetMultiMonitorMode()==PAGER_ON_PRIMARY)
    { 
      if ((*iter)->HasFlag("OnSecondaryMonitor"))
      {
        trace("ignore on secondary monitor during show\n");
        iter++;
        continue;
      }
    }

    trace("desktop show: remeber: GetPager->RememberActiveWindow(): %d\n", GetPager->RememberActiveWindow());


    if (GetPager->RememberActiveWindow()==true && activeWindow!=NULL && IsWindow(activeWindow) && 
      activeWindow==(*iter)->GetRealWindowHWND() )
    {
      LogExtraW(L"Aktiviere Atkvie window: %x, class: %s\n", (*iter)->GetRealWindowHWND(), (*iter)->GetClassString()->c_str());
      (*iter)->Show(true, false, true);
    } else
    {
      (*iter)->Show(true, false, false);

    }

    iter++;
  }
}

void Desktop::HideTaskbarButtons(void)
{
  LogMethod;

  if (!list_torder.empty())
  {
    vector < HWND >::iterator riter = list_torder.begin();
    while (riter != list_torder.end())
    {
      if (!IsWindow(*riter) || IsHungAppWindow(*riter))
      {
        riter++;
        continue;
      }

      Window *windi=Desktop::GetWindowByHWND(*riter);
      if (!windi)
      {
        riter++;
        continue;
      }

      if (GetSys->GetMultiMonitorMode()==PAGER_ON_PRIMARY)
      {
        if (windi->HasFlag("OnSecondaryMonitor"))
        {
          trace("ignoreing OnSecondaryMonitor hide tasklistbutton\n");
          riter++;
          continue;
        }
      }


      GetSys->HideTaskButton(*riter);

      riter++;
    }
  }
}

void Desktop::Hide(void)
{
  LogMethod;


  // trace("hide\n");

  if (list_zorder.empty())
  {
    if (!GetSys->Debuging())
    {
      Desktop::CheckForStillVisibleWindows();
    }
    return;
  }

  Desktop::HideTaskbarButtons();

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    if ((*iter)->HasFlag("hangup"))
    {
      trace("ignore hangup on hide\n");
      iter++;
      continue;
    }

    if (GetSys->GetMultiMonitorMode()==PAGER_ON_PRIMARY)
    {
      if ((*iter)->HasFlag("OnSecondaryMonitor"))
      {
        trace("ignore on secondary monitor during hide\n");
        iter++;
        continue;
      }
    }

    if (!IsWindow((*iter)->GetRealWindowHWND()))
    {
      if ((*iter)->HasFlag("button"))
      {
        Desktop::RemoveButton((*iter)->GetRealWindowHWND());
      } 
      delete *iter;
      iter = list_zorder.erase(iter);
      continue;
    }

    (*iter)->Hide();

    iter++;
  }

  if (!GetSys->Debuging())
  {
    Desktop::CheckForStillVisibleWindows();
  }

}

Window *Desktop::GetWindowByHWND(HWND id)
{
  LogMethod;

  vector < Window * >::iterator iter = list_zorder.begin();

  if (id == 0)
  {
    return NULL;
  }

  Window *ret_win = NULL;

  while (iter != list_zorder.end())
  {
    if ((*iter)->GetRealWindowHWND() == id)
    {
      ret_win = *iter;
      break;
    }
    iter++;
  }

  return ret_win;
}

HWND Desktop::GetButtonByHWND(HWND window)
{
  LogMethod;

  HWND return_win = NULL;

  vector < HWND >::iterator iter = list_torder.begin();
  while (iter != list_torder.end())
  {
    if ((*iter) == window)
    {
      return_win = *iter;
      break;
    }
    iter++;
  }

  return return_win;
}

int Desktop::GetDesktopNumber()
{
  LogMethod;

  return number;
}

void Desktop::DrawMiniWindows(Gdiplus::Graphics *g)
{
  LogMethod;

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    if (!(*iter)->HasFlag("ignore") &&
      !(*iter)->HasFlag("iconic") &&
      !(*iter)->HasFlag("OnSecondaryMonitor") &&
      !(*iter)->GetWmExTransparent() &&
      !(*iter)->IsConsoleWindow())
    {
      (*iter)->DrawMiniWindow(g, 0, 0, FALSE);
    } 

    iter++;
  }
}

Window *Desktop::GetMiniWindowByPoint(POINT pt)
{
  LogMethod;

  Window *mini_window = NULL;
  vector < Window * >::reverse_iterator iter = list_zorder.rbegin();
  while (iter != list_zorder.rend())
  {
    if (!(*iter)->HasFlag("ignore") && 
      !(*iter)->HasFlag("iconic") &&
      !(*iter)->GetWmExTransparent() &&
      !(*iter)->IsConsoleWindow())
    {
      RECT rect = (*iter)->GetMiniWindowRect();
      if (GetSys->PointIsInsideRect(rect, pt))
      {
        mini_window = *iter;
        break;
      }

    }
    iter++;
  }

  return mini_window;
}

void Desktop::AddToplevelWindowsFromTasklistVistaXP()
{
  LogMethod;

  int b2 = 0;
  TBBUTTON tbButton;
  DWORD dwProcessId = 0, dwThreadId = 0;

  HWND hDesktop =::GetDesktopWindow();
  HWND hTray =::FindWindowEx(hDesktop, 0, ("Shell_TrayWnd"), NULL);
  HWND hReBar =::FindWindowEx(hTray, 0, ("ReBarWindow32"), NULL);
  HWND hTask =::FindWindowEx(hReBar, 0, ("MSTaskSwWClass"), NULL);
  HWND hToolbar =::FindWindowEx(hTask, 0, ("ToolbarWindow32"), NULL);

  LRESULT count =::SendMessage(hToolbar, TB_BUTTONCOUNT, 0, 0);
  dwThreadId = GetWindowThreadProcessId(hToolbar, &dwProcessId);

  shared_ptr<void> hProcess (OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId), CloseHandle);
  if (NULL == hProcess.get())
  {
    return;
  }

  memset(&tbButton, 0, sizeof(TBBUTTON));

  for (int i = 0; i < count; i++)
  {
    memset(&tbButton, 0, sizeof(TBBUTTON));

    shared_ptr<void> lpRemoteBuffer (
      VirtualAllocEx(hProcess.get(), NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE), 
      bind<BOOL>(VirtualFreeEx, hProcess.get(), _1, 0, MEM_RELEASE));
    if (NULL == lpRemoteBuffer.get())
    {
      return;
    }

    SendMessage(hToolbar, TB_GETBUTTON, i, (LPARAM) lpRemoteBuffer.get());

    b2 = ReadProcessMemory(hProcess.get(), lpRemoteBuffer.get(),
      (LPVOID) & tbButton, sizeof(TBBUTTON), NULL);
    if (0 == b2)
    {
      continue;
    }

    BYTE localBuffer[0x1000];
    BYTE *pLocalBuffer = localBuffer;
    DWORD_PTR ipLocalBuffer = (DWORD_PTR) pLocalBuffer;
    pLocalBuffer = localBuffer;
    ipLocalBuffer = (DWORD_PTR) pLocalBuffer;
    DWORD_PTR lpRemoteData = (DWORD_PTR) tbButton.dwData;

    ReadProcessMemory(hProcess.get(), (LPVOID) lpRemoteData, (LPVOID) ipLocalBuffer,
      sizeof(DWORD_PTR), NULL);

    HWND windowHandle;
    memcpy(&windowHandle, (void *) ipLocalBuffer, 4);

    if (windowHandle != NULL)
    {
      trace ("adding button: %x\n", windowHandle);
      if (GetSys->WindowIsOwnedbyMe(windowHandle) && !GetPager->IsBadWindow(windowHandle))
      {
        if (GetProp(windowHandle, "PAGER_IGNORE"))
        {
          RemoveProp(windowHandle, "PAGER_IGNORE");
        }

        if (GetProp(windowHandle, "SystemMenuHooked"))
        {
          RemoveProp(windowHandle, "SystemMenuHooked");
        }

        Window *NewToplevel = new Window(this, windowHandle, true);
        if (NewToplevel->IsDead())
        {
          delete NewToplevel;
        } else
        {
          MarkInjectedWindows(windowHandle);

          Desktop::AddButton(windowHandle);
          Desktop::AddWindow(NewToplevel);
          NewToplevel->UpdateMiniWindow();
        }
      }
    }
  }
}

void Desktop::AddToplevelWindows()
{
  LogMethod;

  if (GetWinVersion->IsSeven() || GetWinVersion->IsWin2000())
  {
    Desktop::AddToplevelWindowsWin7();
  } else
  {
    Desktop::AddToplevelWindowsFromTasklistVistaXP();
    Desktop::AddToplevelWindowsNotTasklistVistaXP();
  }
}

void Desktop::CheckForStillVisibleWindows(void)
{
  LogMethod;

  vector < HWND > zorder;
  HWND hwnd = GetTopWindow(NULL);
  if (!hwnd)
  {
    return;
  }

  while (hwnd)
  {
    zorder.push_back(hwnd);
    hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
  }

  reverse(zorder.begin(), zorder.end());

  BOOST_FOREACH(HWND win, zorder)
  {
    if (!IsWindowVisible(win))
    {
      continue;
    }

    if (GetPager->IsBadWindow(win))
    {
      continue;
    }

    if (GetProp(win, "PAGER_WINDOW"))
    {
      continue;
    }

    LONG_PTR ex_style = GetSys->GetWindowLongPTR(win, GWL_EXSTYLE);
    if (ex_style==-1)
    {
      continue;
    }

    if (GetPager->GetWindowByHWNDAllDesks(win))
    {
      continue;
    }

    if (GetPager->GetStickyWindowByHWND(win))
    {
      continue;
    }

    if (ex_style & WS_EX_APPWINDOW)
    {
      trace("  found appwindow\n");
      AddNewTaskBarWindow(win, true);
      continue;
    }

    HWND owner=GetWindow(win, GW_OWNER);
    if (owner==NULL)
    {
      trace("  owner null\n");
      if (!((ex_style & WS_EX_NOACTIVATE) || (ex_style & WS_EX_TOOLWINDOW)))
      {
        trace("  no noactive or toolwinodw->new taskbar window\n");
        AddNewTaskBarWindow(win, true);
        continue;
      } else
      {
        trace("  found noactive or toolstyle->maybeaddWndow()\n");
        MaybeAddWindow(win, true);
        continue;
      }
    } 

    trace("  found owner->maybe window\n");
    MaybeAddWindow(win, true);
  }
}

void Desktop::AddNewTaskBarWindow(HWND window, bool hide/*=false*/)
{
  LogMethod;

  Window *NewToplevel = new Window(this, window, true);
  if (NewToplevel->IsDead())
  {
    delete NewToplevel;
  } else
  {
    Desktop::AddWindow(NewToplevel);
    Desktop::AddButton(window);
    NewToplevel->UpdateMiniWindow();
    if (hide && !NewToplevel->Window::HasFlag("OnSecondaryMonitor"))
    {
      NewToplevel->Hide();
    }
  }
}

void Desktop::MarkInjectedWindows(HWND win)
{
  DWORD pId;
  if (!GetWindowThreadProcessId(win, &pId))
  {
    return;
  }

  if (pId == GetCurrentProcessId())
  {
    return;
  }

  if (pId == GetPager->GetExplorerPID())
  {
    return;
  }

  if (GetPager->PIDKnown(pId))
  {
    return;
  }

  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |  PROCESS_VM_READ, FALSE, pId);
  if(hProcess)
  {
    bool is64BitPros= GetSys->Is64BitProcess(hProcess);

    if (GetSys->WpagerApiHookAlreadInjected (hProcess, is64BitPros))
    {
     GetPager->AddPID(pId);
    }
  }
}

void Desktop::AddToplevelWindowsWin7(void)
{
  LogMethod;

  vector < HWND > zorder;
  HWND hwnd = GetTopWindow(NULL);
  if (!hwnd)
  {
    return;
  }

  while (hwnd)
  {
    zorder.push_back(hwnd);
    hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
  }

  reverse(zorder.begin(), zorder.end());

  BOOST_FOREACH(HWND win, zorder)
  {
    if (GetProp(win, "PAGER_IGNORE"))
    {
      RemoveProp(win, "PAGER_IGNORE");
    }

    if (GetProp(win, "SystemMenuHooked"))
    {
      RemoveProp(win, "SystemMenuHooked");
    }

    if (!IsWindowVisible(win))
    {
      continue;
    }

    if (GetPager->IsBadWindow(win))
    {
      continue;
    }

    LONG_PTR ex_style = GetSys->GetWindowLongPTR(win, GWL_EXSTYLE);
    if (ex_style==-1)
    {
      continue;
    }

    if (ex_style & WS_EX_APPWINDOW)
    {
      trace("  found appwindow\n");

      MarkInjectedWindows(win);
      AddNewTaskBarWindow(win);
      continue;
    }

    HWND owner=GetWindow(win, GW_OWNER);
    if (owner==NULL)
    {
      trace("  owner null\n");
      if (!((ex_style & WS_EX_NOACTIVATE) || (ex_style & WS_EX_TOOLWINDOW)))
      {
        trace("  no noactive or toolwinodw->new taskbar window\n");
        MarkInjectedWindows(win);
        AddNewTaskBarWindow(win);
        continue;
      } else
      {
        trace("  found noactive or toolstyle->maybeaddWndow()\n");
        MarkInjectedWindows(win);
        MaybeAddWindow(win);
        continue;
      }
    } 

    trace("  found owner->maybe window\n");
    MarkInjectedWindows(win);
    MaybeAddWindow(win);
  }
}

void Desktop::AddToplevelWindowsNotTasklistVistaXP(void)
{
  LogMethod;

  vector < HWND > zorder;
  HWND hwnd = GetTopWindow(NULL);
  if (!hwnd)
  {
    return;
  }

  while (hwnd)
  {
    zorder.push_back(hwnd);
    hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
  }

  reverse(zorder.begin(), zorder.end());

  BOOST_FOREACH(HWND win, zorder)
  {
    if (GetProp(win, "PAGER_IGNORE"))
    {
      RemoveProp(win, "PAGER_IGNORE");
    }

    if (GetProp(win, "SystemMenuHooked"))
    {
      RemoveProp(win, "SystemMenuHooked");
    }

    if (!IsWindowVisible(win))
    {
      continue;
    }

    if (GetPager->IsBadWindow(win))
    {
      continue;
    }

    if (Desktop::IsButton(win))
    {
      continue;
    }

    LONG_PTR ex_style = GetSys->GetWindowLongPTR(win, GWL_EXSTYLE);
    if (ex_style==-1)
    {
      continue;
    }

    if (ex_style & WS_EX_APPWINDOW)
    {
      continue;
    }

    HWND owner=GetWindow(win, GW_OWNER);
    if (owner==NULL)
    {
      if (!((ex_style & WS_EX_NOACTIVATE) || (ex_style & WS_EX_TOOLWINDOW)))
      {
        continue;
      } else
      {
        MarkInjectedWindows(win);
        MaybeAddWindow(win);
        continue;
      }
    } 

    MarkInjectedWindows(win);
    MaybeAddWindow(win);
  }
}

RECT Desktop::GetCellRect()
{
  LogMethod;

  return cell_rect;
}

int CALLBACK Desktop::FlashTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
  LogMethod;

  UINT desk_nr = (UINT) GetPager->Key2ValueGetValue(idTimer);
  std::vector <Desktop*> list=GetPager->GetDesktopList();
  list[desk_nr]->SetFlashStatus(!list[desk_nr]->GetFlashStatus());
  GetPager->RedrawWindow();
  return 1;
}

void Desktop::StopFlash(void)
{
  LogMethod;

  if (flash_timer_id !=0)
  {
    KillTimer(GetPager->GetPagerWindowHWND(), flash_timer_id);
    GetPager->Key2ValueDelete(flash_timer_id);
    flash_timer_id=0;
  }

  if (flashing)
  {
    flashing=false;
    flash_status=false;
  }
}

void Desktop::ManageNotification(void)
{
  LogMethod;

  if (flashing==true)
  {
    return;
  }

  flashing=true;
  flash_status=false;
  flash_timer_id=GetPager->CreateUniqueTimerID();

  GetPager->Key2ValueInsert(flash_timer_id, Desktop::number);
  SetTimer(GetPager->GetPagerWindowHWND(), flash_timer_id, 500,
    (TIMERPROC) Desktop::FlashTimer);
}

void Desktop::DrawVista(Gdiplus::Graphics *g)
{
  LogMethod;

  Rect cellrect;
  GetSys->RectToGDIPlusRect(&cellrect, cell_rect);

  Rect deskrect;
  GetSys->RectToGDIPlusRect(&deskrect, desk_rect);


  if (number == GetPager->CurrentDesktop()->GetDesktopNumber())
  {
    LinearGradientBrush linGrBrush(
      Point(0,cellrect.Y-1), Point(0, cellrect.GetBottom()+1), 
      Color(255, 255, 246, 212), Color(255, 247, 136, 4));
    g->FillRectangle(&linGrBrush, cellrect.X, cellrect.Y, cellrect.Width+1, cellrect.Height+1);
  } else
  {
    LinearGradientBrush linGrBrush(
      Point(0,cellrect.Y-1), Point(0, cellrect.GetBottom()+1),
      Color(255, 240, 240, 240), Color(255, 146, 146, 146));
    g->FillRectangle(&linGrBrush, cellrect.X, cellrect.Y, cellrect.Width+1, cellrect.Height+1);
  }

  REAL positions[3] = {0.0f, 0.3f, 1.0f};
  REAL factors[3] =   {0.0f, 0.6f, 1.0f};
  LinearGradientBrush linGrBrush(Point(0,deskrect.Y-1), Point(0, deskrect.GetBottom()+1),
    Color(255, 140, 140, 140),  Color(255, 200, 200, 200));
  linGrBrush.SetBlend(factors, positions, 3);
  g->FillRectangle(&linGrBrush, deskrect.X, deskrect.Y, deskrect.Width+1, deskrect.Height+1);
}

void Desktop::DrawWin2000(Gdiplus::Graphics *g)
{
  LogMethod;

  Rect cellrect;
  GetSys->RectToGDIPlusRect(&cellrect, cell_rect);

  if (number == GetPager->CurrentDesktop()->GetDesktopNumber())
  {
    Pen pen1(Color(255, 255, 0, 255), 1);
    g->DrawRectangle(&pen1, cellrect);
  }

  Pen pen_dark(Color(255, 128, 128, 128), 1);
  g->DrawLine(&pen_dark,
    cell_rect.left + 1, cell_rect.top +1, 
    cell_rect.left + 1, cell_rect.bottom - 1);

  g->DrawLine(&pen_dark,
    cell_rect.left + 1, cell_rect.top +1, 
    cell_rect.right -1, cell_rect.top +1);

  Pen pen_light(Color(255, 220, 220, 220), 1);
  g->DrawLine(&pen_light,
    cell_rect.right -1, cell_rect.top +2, 
    cell_rect.right -1, cell_rect.bottom -1);

  g->DrawLine(&pen_light,
    cell_rect.right -1, cell_rect.bottom -1, 
    cell_rect.left + 2, cell_rect.bottom -1);
}

void Desktop::DrawFlash(Gdiplus::Graphics *g)
{
  LogMethod;

  if (flashing && flash_status==true)
  {
    Rect cellrect;
    GetSys->RectToGDIPlusRect(&cellrect, cell_rect);
    LinearGradientBrush linGrBrush(
      Point(0, cellrect.Y-1), Point(0, cellrect.GetBottom()+1),
      Color(200, 255, 171, 171), Color(200, 255, 0, 0));
    g->FillRectangle(&linGrBrush, cellrect.X, cellrect.Y, cellrect.Width+1, cellrect.Height+1);
  }
}

void Desktop::DrawDesktop(Gdiplus::Graphics *g)
{
  LogMethod;

  if (GetWinVersion->IsVista() || GetWinVersion->IsXP())
  {
    Desktop::DrawVista(g);
  } else if (GetWinVersion->IsWin2000() || GetWinVersion->IsXPClassic())
  {
    Desktop::DrawWin2000(g);
  }
}

void Desktop::WindowActivated(Window * window)
{
  LogMethod;

  Desktop::SetActiveWindow(window);

  if (!IsIconic(window->GetRealWindowHWND()) && !GetPager->UseIncognitoMode())
  {
    GetPager->RedrawWindow();
  }
}

bool Desktop::AddHiddenGedoens(HWND window, HWND owner, RECT real_rect)
{
  LogMethod;

  if (owner==NULL)
  {
    trace("owner is null->move to current desk\n");
    SetWindowPos(window, NULL, real_rect.left-15000, real_rect.top,
      0, 0, SWP_NOSIZE | SWP_FRAMECHANGED |SWP_NOSENDCHANGING);
    return false;
  }

  Window *owni=GetPager->GetWindowByHWNDAllDesks(owner);
  if (owni==NULL)
  {
    trace("ownI is null->move to current desk\n");
    SetWindowPos(window, NULL, real_rect.left-15000, real_rect.top,
      0, 0, SWP_NOSIZE | SWP_FRAMECHANGED | SWP_NOSENDCHANGING);
    return false;
  }

  trace("found owni on desk: %d\n", owni->GetDesktop()->GetDesktopNumber());

  Window *NewToplevel = new Window(owni->GetDesktop(), window, false);
  if (NewToplevel->IsDead())
  {
    delete NewToplevel;
  } else
  {
    RECT new_rect;  
    SetRect(&new_rect, real_rect.left-15000, real_rect.top, 
      real_rect.right-15000, real_rect.bottom);

    owni->GetDesktop()->AddWindow(NewToplevel);
    NewToplevel->SetBackupRect(new_rect);
    NewToplevel->SetHidden();
    NewToplevel->DeleteFlag("ignore");
    NewToplevel->UpdateMiniWindow();

    // owni->GetDesktop()->ManageNotification(GetPager->GetOSD());
  }

  return true;
}

bool Desktop::MaybeAddWindow( HWND window, bool hide/*=false*/ )
{
  LogMethod;

  LONG_PTR normal_style=0;
  LONG_PTR ex_style=0;
  RECT window_rect;

  /*{
  char wndClassName[256];
  GetClassName(window, wndClassName, 256);
  trace ("adding maybe window oben: %x(%s)\n", window, wndClassName);
  }*/

  if (!GetSys->WindowIsOwnedbyMe(window))
  {
    return false;
  }

  HWND parent= GetAncestor(window, GA_PARENT);  
  if (!parent)
  {
    return false;
  }

  if (parent!=GetSys->GetRootWindow())
  {
    return false;
  }

  ex_style = GetSys->GetWindowLongPTR(window, GWL_EXSTYLE);
  if (ex_style==-1)
  {
    return false;
  }

  if (ex_style & WS_EX_APPWINDOW)
  {
    return false;
  }

  normal_style = GetSys->GetWindowLongPTR(window, GWL_STYLE);
  if (normal_style==-1)
  {
    trace("normal style failed\n");
    return false;
  }

  HWND owner=GetWindow(window, GW_OWNER);
  if (owner==NULL)
  {
    trace("owner null von %x\n", window);
    if (!((ex_style & WS_EX_NOACTIVATE) || (ex_style & WS_EX_TOOLWINDOW)))
    {
      trace("no noactive or toolwinodw->skip\n");
      return false;
    } else
    {
      trace("found noactive or toolstyle\n");
    }
  } else
  {
    trace("found owner\n");
  }

  if (!GetWindowRect(window, &window_rect))
  {
    return false;
  }

  if (EqualRect(&window_rect, &GetSys->GetPrimaryRect()))
  {
    return false;
  }

  if (EqualRect(&window_rect, &GetSys->GetFullScreenRect()))
  {
    trace ("fullscreen widnow-> skip\n");
    return false;
  }

  if (abs(window_rect.left - window_rect.right) <= 1 &&
    abs(window_rect.bottom - window_rect.top) <= 1)
  {
    trace ("window too small\n");
    return false; 
  }

  if (!GetSys->WindowIsVisibleOnAMonitor(window))
  {
    RECT inti2;
    int intersect = IntersectRect(&inti2, &window_rect, &GetSys->GetHiddenRect());
    if (intersect)
    {
      trace("hidden_gedönes\n");
      if (AddHiddenGedoens(window, owner, window_rect))
      {
        return true;
      }
    } else
    {
      return false;
    }
  }

  char wndClassName[256];
  GetClassName(window, wndClassName, 256);
  trace ("adding maybe window ___: %x(%s)\n", window, wndClassName);

  Window *NewToplevel = new Window(this, window, false);
  if (NewToplevel->IsDead())
  {
    delete NewToplevel;
    return false;
  } else
  {

    Desktop::AddWindow(NewToplevel);
    NewToplevel->UpdateMiniWindow();

    if (hide && !NewToplevel->Window::HasFlag("OnSecondaryMonitor"))
    {
      NewToplevel->Hide();
    }

    return true;
  }
}

void Desktop::UpdateMiniWindows(void)
{
  LogMethod;

  vector < Window * >::iterator iter = list_zorder.begin();
  while (iter != list_zorder.end())
  {
    (*iter)->UpdateMiniWindow();
    iter++;
  }
}

void Desktop::ChangeWallpaper( void )
{
  LogMethod;

  if (wallpaper_path.compare(L"default")==0)
  {
    bool b=SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, 
      (void*) GetSys->GetDefaultWallpaper().c_str(), 0);
  } else
  {
    bool b=SystemParametersInfoW(SPI_SETDESKWALLPAPER, 0, 
      (void*) wallpaper_path.c_str(), 0);

    trace("bool ist: %d\n", b);
  }
}

std::wstring Desktop::GetWallpaperPath( void )
{
  LogMethod;
  return wallpaper_path;
}

HICON Desktop::GetTrayIcon(void)
{
  LogMethod;
  return tray_icon;
}

bool Desktop::GetFlashStatus(void)
{
  LogMethod;
  return flash_status;
}

void Desktop::SetFlashStatus( bool new_status )
{
  LogMethod;
  flash_status=new_status;
}