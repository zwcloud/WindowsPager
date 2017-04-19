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
#include "System.h"

#include "Window.h"
#include "Desktop.h"
#include "Pager.h"
#include "XWinVer.h"

#include "HangUpInfoWindow.h"

using namespace std;
using namespace Gdiplus;
using namespace boost;


void Window::SecondaryToPrimary(void)
{
  LogMethod;

  Window *owni=NULL;
  DeleteFlag("OnSecondaryMonitor");

  if (!Window::HasFlag("WS_EX_TOOLWINDOW"))
  {
    RemoveExStyle(WS_EX_TOOLWINDOW, 200);
    SetWindowPosFake();
  }

  Desktop *old_desk=desktop;

  HWND owner = GetWindow(real_window, GW_OWNER);
  if (owner && owner != real_window)
  {
    owni=old_desk->GetWindowByHWND(owner);
    if (owni && owni->HasFlag("ignore"))
    {
      owni->DeleteFlag("OnSecondaryMonitor");
      owni->RemoveExStyle(WS_EX_TOOLWINDOW, 200);
      owni->SetWindowPosFake();
    }

    GetSys->ShowTaskButton(owner);
  } else
  {
    if(!Window::HasFlag("WS_EX_TOOLWINDOW"))
    {
      GetSys->ShowTaskButton(real_window);
    }
  } 

  if (desktop->GetDesktopNumber()!= GetPager->CurrentDesktop()->GetDesktopNumber())
  {
    trace("window from diffrent desktop\n");
    if (owni && owni->HasFlag("ignore"))
    {
      old_desk->RemoveButton(owner);
      old_desk->RemoveWindow(owni, FALSE);
      GetPager->CurrentDesktop()->AddButton(owner);
      GetPager->CurrentDesktop()->AddWindow(owni);
    } else
    {
      old_desk->RemoveButton(real_window);
      GetPager->CurrentDesktop()->AddButton(real_window);
    }
    old_desk->RemoveWindow(this, false);
    GetPager->CurrentDesktop()->AddWindow(this);
  }

  UpdateAlwaysVisible(false);
  SystemMenuUpdate();
}

void Window::PrimaryToSecondary(void)
{
  LogMethod;

  Window *owni=NULL;
  SetFlag("OnSecondaryMonitor");

  HWND owner = GetWindow(real_window, GW_OWNER);
  if (owner && owner != real_window)
  {
    Window *owni=desktop->GetWindowByHWND(owner);
    if (owni && owni->HasFlag("ignore"))
    {
      owni->SetFlag("OnSecondaryMonitor");
      owni->SetExStyle(WS_EX_TOOLWINDOW, 200);
      owni->SetWindowPosFake();
    }
    GetSys->HideTaskButton(owner);
  } else
  {
    if (!Window::HasFlag("WS_EX_TOOLWINDOW"))
    {
      GetSys->HideTaskButton(real_window);
    }
  } 

  UpdateAlwaysVisible(false);
  SystemMenuUpdate();

  if (!Window::HasFlag("WS_EX_TOOLWINDOW"))
  {
    SetExStyle(WS_EX_TOOLWINDOW, 200);
    SetWindowPosFake();
  }
}


bool Window::SizeEvent(void)
{
  LogMethod;

  //trace("window size event\n");

  if (!IsWindowVisible(real_window) && !HasFlag("button"))
  {
    char wndClassName[256];
    if (GetClassName(real_window, wndClassName, 256))
    {
      if (lstrcmpi("TAIMPPlaylistDockFormW", wndClassName) == 0)
      {
        trace("size window death or not visible\n");
        desktop->RemoveWindow(this, false);
        delete this;
        return true;
      }
    }
  }

  if (HasFlag("hangup"))
  {
    trace("ignore hang up on size\n");
    return false;
  }

  if (HasFlag("resize_known"))
  {
    trace("but resize known(%x)\n", real_window);
    DeleteFlag("resize_known");
    return false;
  }

  if (HasFlag("hidden"))
  {
    trace("ignore hidden window on size\n");
    return false;
  }

  if (HasFlag("ignore"))
  {
    trace("resize but ingore set\n");
    return false;
  }

  if (GetSys->GetMultiMonitorMode()==PAGER_ON_PRIMARY)
  {
    int monitor=GetSys->GetMonitor(real_window);
    if (monitor== PRIMARY_MONITOR && HasFlag("OnSecondaryMonitor"))
    {
      trace("secondary to primary.............................\n");
      SecondaryToPrimary();
    } else if (monitor==SECONDARY_MONITOR && !HasFlag("OnSecondaryMonitor"))
    {
      trace("primary to secondary.........................\n");
      PrimaryToSecondary();
    }
  }      

  if (GetPager->UseIncognitoMode())
  {
    return false;
  }

  UpdateMiniWindow();
  return true;
}

void Window::RemoveSubclassOrHook(void)
{
  LogMethod;

  if (!explorer && !console_window && !HasFlag("ignore"))
  {
    LRESULT res=SendMessageTimeout(real_window, MSG_RESET_SUBCLASS, 
      (WPARAM) GetPager->GetHookAtom(), 0, SMTO_ABORTIFHUNG|SMTO_BLOCK, 200, NULL);
  }
}

void Window::RestoreOnExitSticky(void)
{
  LogMethod;

  if (!GetPager->StickyWindowsHaveButtons())
  {
    Window::RemoveExStyle(WS_EX_TOOLWINDOW, 200);
    Window::SetWindowPosFake();
    GetSys->ShowTaskButton(real_window);
  }

  RemoveSubclassOrHook();
  SystemMenuReset();

}

void Window::RestoreOnExit(void)
{
  LogMethod;

  trace("restore on exit: %x\n", real_window);
  trace("obseravtion_timer: %x\n", observation_timer);

  if (observation_timer !=0)
  {
    trace("kill obseravtiont imer on exit\n");
    KillTimer(GetPager->GetPagerWindowHWND(), observation_timer);
    GetPager->Key2ValueDelete(observation_timer);
  }

  RemoveSubclassOrHook();
  SystemMenuReset();

  if (Window::HasFlag("OnSecondaryMonitor"))
  {
    Window::RemoveExStyle(WS_EX_TOOLWINDOW, 200);  
    Window::SetWindowPosFake();
    GetSys->ShowTaskButton(real_window);
    return;
  }
}

void Window::ShowOnExit()
{
  LogMethod;

  if (observation_timer !=0)
  {
    trace("kill obseravtiont imer on exit\n");
    KillTimer(GetPager->GetPagerWindowHWND(), observation_timer);
    GetPager->Key2ValueDelete(observation_timer);
  }

  RemoveSubclassOrHook();
  SystemMenuReset();

  if (Window::HasFlag("OnSecondaryMonitor"))
  {
    Window::RemoveExStyle(WS_EX_TOOLWINDOW, 200);  
    Window::SetWindowPosFake();
    GetSys->ShowTaskButton(real_window);
    return;
  }

  Window::Show(false, false, false);
}

void Window::HideConsoleWindow(void)
{
  LogMethod;

  trace("hide console windo\n");
  RECT backup_rect;
  if(!GetWindowRect(real_window, &backup_rect))
  {
    trace ("get window rect failed!\n");
    return;
  }

  //trace("hide x: %d, y: %d\n", backup_rect.left, backup_rect.top);

  SetWindowPos(real_window, NULL, backup_rect.left+15000, backup_rect.top,
    0, 0, SWP_NOSIZE| SWP_NOZORDER | SWP_NOACTIVATE |
    SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS | SWP_FRAMECHANGED);
}

void Window::Hide(void)
{
  LogMethod;

  if (console_window)
  {
    Window::HideConsoleWindow();
    return;
  }

  if (IsHungAppWindow(real_window))
  {
    trace("found hang up during hide, adding to uberwachung\n");
    if (!Window::HasFlag("ignore") && !Window::HasFlag("iconic"))
    {
      trace("start observation\n");
      Window::SetFlag("hangup");
      Window::StartObservation(true);
    }
    return;
  }

  if (!SetExStyle(WS_EX_NOACTIVATE | (useToolWindowFlag?WS_EX_TOOLWINDOW:0), 200))
  {
    trace("set ex failed\n");
    if (!Window::HasFlag("ignore") && !Window::HasFlag("iconic"))
    {
      Window::SetFlag("hangup");
      Window::StartObservation(true);
    }
    return;
  }

  if (!Window::SetDisabled(200))
  {
    if (!Window::HasFlag("ignore") && !Window::HasFlag("iconic") )
    {
      Window::SetFlag("hangup");
      Window::StartObservation(true);
    }
    return;
  }

  SetHidden();

  if (HasFlag("iconic"))
  {
    SetProp(real_window, "PAGER_ICONIC", (void*) 1);
    Window::SetWindowPosFake();
    return;
  }

  RECT inter;
  if (IntersectRect(&inter, &GetSys->GetHiddenRect(), &backup_rect))
  {
    trace("backup rect is in hidden rect\n");
    int width=backup_rect.right-backup_rect.left;
    int height=backup_rect.bottom-backup_rect.top;
    SetRect(&backup_rect, 10, 10, 10+width, 10+height);
  }

  SetWindowPos(real_window, NULL, backup_rect.left+15000, backup_rect.top,
    0, 0, SWP_NOSIZE| SWP_NOZORDER | SWP_NOACTIVATE |
    SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS | SWP_FRAMECHANGED);
}

bool Window::RemoveNoactivateStyle(void)
{
  LogMethod;

  if (!Window::RemoveExStyle(WS_EX_NOACTIVATE | (useToolWindowFlag?WS_EX_TOOLWINDOW:0), 200))
  {
    return false;
  }

  return true;
}

void Window::ShowConsoleWindow(void)
{
  LogMethod;

  trace("show console window\n");
  RECT backup_rect;
  if(!GetWindowRect(real_window, &backup_rect))
  {
    trace ("get window rect failed!\n");
    return;
  }

  SetWindowPos(real_window, NULL, backup_rect.left-15000, backup_rect.top,
    0, 0, SWP_NOSIZE| SWP_NOZORDER | SWP_NOACTIVATE |
    SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS | SWP_FRAMECHANGED);
}

void Window::Show(bool observation, bool noAsync/*=false*/, bool activate/*=false*/)
{
  LogMethod;
  if (console_window)
  {
    Window::ShowConsoleWindow();
    return;
  }

  DeleteHidden();

  if (IsHungAppWindow(real_window))
  {
    trace("found hang up on show, adding to uberwachung\n");
    if (!Window::HasFlag("ignore") && !Window::HasFlag("iconic") && observation)
    {
      if (observation)
      {
        Window::SetFlag("hangup");
        Window::StartObservation(false);
      }
    }
    return;
  }

  if (!Window::RemoveNoactivateStyle())
  {
    if (!Window::HasFlag("ignore") && !Window::HasFlag("iconic") && observation)
    {
      Window::SetFlag("hangup");
      Window::StartObservation(false);
    }
    return;
  }

  useToolWindowFlag=false;

  if (!Window::RemoveDisabled(200))
  {
    if (!Window::HasFlag("ignore") && !Window::HasFlag("iconic") && observation)
    {
      if (observation)
      {
        Window::SetFlag("hangup");
        Window::StartObservation(false);
      }
    }
    return;
  }


  if (HasFlag("iconic"))
  {
    Window::SetWindowPosFake();

    RemoveProp(real_window, "PAGER_ICONIC");
    return;
  }

  Window::SetFlag("resize_known");

  if (noAsync)
  {
    trace("show with no async: %x\n", real_window);
    SetWindowPos(real_window, NULL,
      backup_rect.left,
      backup_rect.top,
      0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
      SWP_NOSENDCHANGING | SWP_FRAMECHANGED);
  } else
  {
    SetWindowPos(real_window, NULL,
      backup_rect.left,
      backup_rect.top,
      0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
      SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS | SWP_FRAMECHANGED);
  }

  if (activate==true)
  {
    trace("******************set foregroundWindow!\n");
    SetForegroundWindow(real_window);
  }

}

void Window::SystemMenuInitTrigger(void)
{
  LogMethod;
  trace("send msg_system_menu_init_trigger");
  SendNotifyMessageA(real_window, MSG_SYSTEM_MENU_INIT_TRIGGER, (WPARAM) GetPager->GetHookAtom(), (LPARAM) 0);
}

void Window::SystemMenuReset(void)
{
  LogMethod;

  HMENU system_menu = GetSystemMenu(real_window, FALSE);
  if (system_menu)
  {
    RemoveMenu(system_menu, MENU_SEP1, MF_BYCOMMAND);
    RemoveMenu(system_menu, MENU_SEP2, MF_BYCOMMAND);

    int i;
    for (i = 1; i <= GetPager->GetNumberOfDesktops(); i++)
    {
      RemoveMenu(system_menu, MENU_ITEM + i, MF_BYCOMMAND);
    }

    RemoveMenu(system_menu,
      MENU_ITEM + GetPager->GetNumberOfDesktops() + 1, MF_BYCOMMAND);
    RemoveMenu(system_menu,
      MENU_ITEM + GetPager->GetNumberOfDesktops() + 2, MF_BYCOMMAND);
  }
}

void Window::SystemMenuUpdate(void)
{
 LogMethod;
  
 SetProp(real_window, "PAGER_CURRENT_DESKTOP", (void*) desktop->GetDesktopNumber());

 if (HasFlag("OnSecondaryMonitor"))
 {
   SetProp(real_window, "PAGER_2MONITOR", (void*) 1);
 } else
 {
   RemoveProp(real_window, "PAGER_2MONITOR");
 }

  SendNotifyMessageA(real_window, MSG_SYSTEM_MENU_INIT_TRIGGER, 
    (WPARAM) GetPager->GetHookAtom(), 0);
}

void Window::UpdateAlwaysVisible(bool checked)
{
  LogMethod;

  if (checked)
  {
    SetProp(real_window, "PAGER_STICKY", (void*)1);
  } else
  {
    RemoveProp(real_window, "PAGER_STICKY");
  }
}

void Window::RestoreFromDuringHideToCurrent()
{
  LogMethod;

  trace("hang window belongs to current desktop\n");

  RemoveHiddenStyles();

  if (HasFlag("button"))
  {
    GetSys->ShowTaskButton(real_window);
  } else
  {
    HWND owner = GetWindow(real_window, GW_OWNER);
    if (owner && owner != real_window)
    {
      Window *owni=desktop->GetWindowByHWND(owner);
      if (owni && owni->HasFlag("ignore"))
      {
        trace("bad window has owner on current desktop\n");
        owni->RemoveDisabled(200);
        owni->RemoveExStyle(WS_EX_NOACTIVATE, 200);
        GetSys->ShowTaskButton(owner);
      }
    }
  }
}

void Window::RestoreFromDuringHideToHidden()
{
  LogMethod;

  trace("window belongs to different desktop, hide it\n");

  Window::Hide();

  if (HasFlag("button"))
  {
    GetSys->HideTaskButton(real_window);
  }

  HWND owner = GetWindow(real_window, GW_OWNER);
  if (owner && owner != real_window)
  {
    Window *owni=desktop->GetWindowByHWND(owner);
    if (owni && owni->HasFlag("ignore"))
    {
      trace("bad window has owner\n");
      owni->SetDisabled(200);
      owni->SetExStyle(WS_EX_NOACTIVATE, 200);
      GetSys->HideTaskButton(owner);
    }
  }
}

void Window::RestoreFromDuringHide(void)
{
  LogMethod;

  trace("restore from during hide\n");
  if (desktop->GetDesktopNumber()== GetPager->CurrentDesktop()->GetDesktopNumber())
  {
    RestoreFromDuringHideToCurrent();
  } else
  {
    RestoreFromDuringHideToHidden();
  }
}

void Window::RestoreFromDuringShow(void)
{
  LogMethod;

  trace("resotore from during show\n");

  if (desktop->GetDesktopNumber()==GetPager->CurrentDesktop()->GetDesktopNumber())
  {
    trace("currrent destkop\n");

    Window::Show(false, false, false);
    if (HasFlag("button"))
    {
      GetSys->ShowTaskButton(real_window);
    }
  } else
  {
    SetHiddenStyles();

    if (HasFlag("button"))
    {
      GetSys->HideTaskButton(real_window);
    }
    trace("window belongs to different desktop, do nothing\n");
  }
}   

int CALLBACK Window::ObservationTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
  LogMethod;

  HWND win = (HWND) GetPager->Key2ValueGetValue(idTimer);
  if (win==0)
  {
    KillTimer(hwnd, idTimer);
    return 0;
  }

  Window *window=GetPager->GetWindowByHWNDAllDesks(win);
  if (!window)
  {
    KillTimer(hwnd, idTimer);
    GetPager->Key2ValueDelete(idTimer);
    return 0;
  }

  trace("observation timer\n");

  window->IncreaseHangupCount();

  if (window->DeathCheckPositiv())
  {
    trace("observation window is death\n");
    KillTimer(hwnd, idTimer);
    GetPager->Key2ValueDelete(idTimer);

    if (GetPager->UseHungUpInfoWindow() && !window->RunningInsideVisualStudio())
    {
      delete window->GetDummyWindow();
      window->SetDummyWindowNULL();
    }

    if (window->HasFlag("button"))
    {
      window->GetDesktop()->RemoveButton(win);
    }

    window->GetDesktop()->RemoveWindow(window, TRUE);
    !GetPager->UseIncognitoMode()?GetPager->RedrawWindow():0;
    return 0;
  }

  if (GetSys->WindowIsHungTimeoutCheck(window->GetRealWindowHWND(), 50))
  {
    trace("still hung app\n");

    if (window->GetHangupCount()==(window->WasDuringHide()?8:2))
    {
      if (GetPager->UseHungUpInfoWindow() && !window->RunningInsideVisualStudio())
      {
        window->GetDummyWindow()->Show();
      }
    }

    if (window->GetHangupCount()>=2)
    {
      int mode=window->GetHangupCount() %2;
      trace("mode: %d\n", mode);
      if (mode ==0)
      {
        if (GetPager->UseHungUpInfoWindow() && !window->RunningInsideVisualStudio())
        {
          window->GetDummyWindow()->Update();
        }
      }
    }

    trace("hanup count: %d\n", window->GetHangupCount());

    return 0;
  }


  trace("hang up over\n");
  KillTimer(hwnd, idTimer);
  window->ResetObservationTimer();
  window->DeleteFlag("hangup");

  if (GetPager->UseHungUpInfoWindow() && !window->RunningInsideVisualStudio())
  {
    window->GetDummyWindow()->DieSlowly();
    window->SetDummyWindowNULL();  
  }

  if (window->WasDuringHide())
  {
    window->RestoreFromDuringHide();
  } else
  {
    window->RestoreFromDuringShow();
  }
  return 0;
}

void Window::StartObservation(bool during_hide)
{
  LogMethod;

  trace("start observation of : %x\n", real_window);
  duringhide=during_hide;
  if (GetPager->UseHungUpInfoWindow() && !runningInsideVisualStudio)
  {
    dummy_window= new HangUpInfoWindow(real_window, smallicon, during_hide);
  }

  observation_timer=GetPager->CreateUniqueTimerID();
  GetPager->Key2ValueInsert(observation_timer, (UINT_PTR) real_window);
  hangup_count=0; 
  SetTimer(GetPager->GetPagerWindowHWND(), observation_timer, 500, (TIMERPROC) Window::ObservationTimer);
}

int CALLBACK Window::MenuTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
  LogMethod;
  KillTimer(hwnd, idTimer);

  HWND win = (HWND) GetPager->Key2ValueGetValue(idTimer);
  if (win==0)
  {
    KillTimer(hwnd, idTimer);
    return 0;
  }

  GetPager->Key2ValueDelete(idTimer);

  Window *window=GetPager->GetWindowByHWNDAllDesks(win);
  if (!window)
  {
    KillTimer(hwnd, idTimer);
    return 0;
  }

  window->SystemMenuInitTrigger();
  return false;
}


Window::Window(Desktop * _desktop, HWND _real_window, bool tasklist_window)
{
  LogMethod;

  real_window = _real_window;
  desktop = _desktop;
  mini_window_bitmap = NULL;
  dragged = FALSE;
  flash_timer_id=0;
  dead=false;
  counting_events=false;
  nc_events=0;
  hangup_count=0;
  dummy_window=NULL;
  flags.clear();
  duringhide=false;
  filename=NULL;
  class_string=NULL;
  hover_desktop=NULL;
  observation_timer=0;
  explorer=false;
  wm_ex_transparent=false;
  console_window=false;
  runningInsideVisualStudio=false;
  useToolWindowFlag=false;
  
  WCHAR wndClassName[256];
  if (!GetClassNameW(real_window, wndClassName, 256))
  {
    dead=true;
    return;
  }

  if (lstrcmpiW(L"ConsoleWindowClass", wndClassName) == 0)
  {
    trace("window is console window\n");
    console_window=true;
  }

  if (lstrcmpiW(L"MozillaWindowClass", wndClassName) == 0)
  {
    useToolWindowFlag=true;
  }

  if (!console_window)
  {
    if (!SetProp(_real_window, "PAGER_WINDOW", (void *) 1))
    {
      trace("set prop failed\n");
      dead=true;
      return;
    }
  }

  class_string=new wstring(wndClassName);

  if (IsIconic(real_window))
  {
    Window::SetFlag("iconic");
  }

  if (tasklist_window)
  {
    Window::SetFlag("button");
    !console_window?SetProp(real_window, "PAGER_WINDOW_BUTTON", (void *) 1):0;

    //Hack für steam
    if (!class_string->find(L"USurface_", 9))
    {
      if (!GetSys->WindowIsVisibleOnAMonitor(real_window))
      {
        Window::SetFlag("ignore");
        trace("set ignore to: %x",real_window);
      }
    }
  }

  if (console_window)
  {
    return;
  }

  LONG_PTR style=GetSys->GetWindowLongPTR(real_window, GWL_EXSTYLE);
  if (style!=-1 && (style & WS_EX_TOOLWINDOW))
  {
    Window::SetFlag("WS_EX_TOOLWINDOW");
  }

  int topmost=0;
  if (style!=-1 && (style & WS_EX_TOPMOST))
  {
    topmost=1;
    SetProp(real_window, "PAGER_TOPMOST", (void*) 1);
  }

  if (GetSys->GetMultiMonitorMode()==PAGER_ON_PRIMARY &&
    GetSys->GetMonitor(real_window)==SECONDARY_MONITOR)
  {
    trace("window is on secondary monitor on start\n");
    Window::SetFlag("OnSecondaryMonitor");

    if (!Window::HasFlag("WS_EX_TOOLWINDOW"))
    {
      Window::SetExStyle(WS_EX_TOOLWINDOW, 200);
      Window::SetWindowPosFake();
      GetSys->HideTaskButton(Window::real_window);
    }
  }

  HWND owner = GetWindow(real_window, GW_OWNER);
  window_icon = owner ? owner : real_window;

  bool ismediaMonkeyShit=false;

  filename=NULL;
  filename=GetSys->GetWindowFileName(real_window);
  if (filename!=NULL)
  {
    int last_backslash=filename->find_last_of('\\');
    if (last_backslash!=wstring::npos)
    {
      wstring exe_name=filename->substr(last_backslash+1);
      to_lower(exe_name);

      trace("exe name: <%s>\n", exe_name);

      if (exe_name.find(L"vshost.exe")!=wstring::npos)
      {
        runningInsideVisualStudio=true;
      }

      if (CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, exe_name.c_str(),
        -1, L"mediamonkey.exe", -1)==CSTR_EQUAL)
      {
        ismediaMonkeyShit=true;
      }

      if (GetPager->CheckIfOnIgnoreList(exe_name, *class_string))
      {
        tracew(L"new window: %s on ignore list-> destroy\n", filename->c_str());
        SetProp(real_window, "PAGER_IGNORE", (void*) 1);
        dead=true;
        return;
      }
    }
  }

  if (filename!=NULL)
  {
    // trace("got filename: %s\n", filename->c_str());
    wstring tmp=(L"c:\\windows\\explorer.exe");
    if (CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE,
      filename->c_str(), -1, tmp.c_str(), -1)==CSTR_EQUAL)
    {
      explorer=true;
    }

    UINT piconid;
    UINT res= PrivateExtractIconsW(filename->c_str(), explorer?13:0, 48, 48,
      &smallicon, &piconid, 1, LR_COLOR);

    if (res==0 || res==0xFFFFFFFF)
    {
      smallicon = GetSys->GetIconFromWindowHWND(window_icon);
    }
  } else
  {
    smallicon = GetSys->GetIconFromWindowHWND(window_icon);
  }


  //no relation to layered stuff
  if (style!=-1 && (style & WS_EX_TRANSPARENT))
  {
    trace("set wx_translskdf style to: %x\n", real_window);
    wm_ex_transparent=true;
  }
 
  if (!explorer && !HasFlag("ignore") &&!ismediaMonkeyShit)
  {
    SendNotifyMessageA(real_window, MSG_SUBCLASS, (WPARAM) GetPager->GetHookAtom(), 0);
  }

  SetProp(real_window, "PAGER_CURRENT_DESKTOP", (void*) desktop->GetDesktopNumber());

  if (Window::HasFlag("ignore"))
  {
    SetProp(real_window, "PAGER_IGNORE", (void*) 1);
  }

  if (Window::HasFlag("OnSecondaryMonitor"))
  {
    SetProp(real_window, "PAGER_2MONITOR", (void*) 1);
  }

  Window::InjectHookDll();

  UINT_PTR  menu_timer=GetPager->CreateUniqueTimerID();
  GetPager->Key2ValueInsert(menu_timer, (UINT_PTR) real_window);
  SetTimer(GetPager->GetPagerWindowHWND(), menu_timer, 500, (TIMERPROC) MenuTimer);
}

void Window::InjectHookDll(void)
{
  if (!IsWindow(real_window))
  {
    return;
  }

  DWORD pId;
  if (!GetWindowThreadProcessId(real_window, &pId))
  {
    return;
  }

  if (pId == GetCurrentProcessId())
  {
    return;
  }
 
  trace("\ninjecthook dll. pid: %d, window: %x\n", pId, real_window);

  if (GetPager->PIDKnown(pId))
  {
    trace("pid known\n");
    return;
  }

  HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |  PROCESS_VM_READ, FALSE, pId);
  if(hProcess)
  {
    bool is64BitPros= GetSys->Is64BitProcess(hProcess);
    if (is64BitPros)
    {
      HWND hook_starter64 = FindWindow("PagerHookStarter64", NULL);
      if (hook_starter64)
      {
        PostMessage(hook_starter64, MSG_INJECT64_DLL, (WPARAM) real_window, 0);
      }
    } else
    {
      HWND hook_starter32 = FindWindow("PagerHookStarter32", NULL);
      if (hook_starter32)
      {
        PostMessage(hook_starter32, MSG_INJECT32_DLL, (WPARAM) real_window, 0);
      }
    }

    CloseHandle(hProcess);
  }
}

Window::~Window()
{
  LogMethod;

  trace("~Window: %x\n", real_window);

  if (flash_timer_id !=0)
  {
    KillTimer(GetPager->GetPagerWindowHWND(), flash_timer_id);
    GetPager->Key2ValueDelete(flash_timer_id);
  }

  if (observation_timer !=0)
  {
    KillTimer(GetPager->GetPagerWindowHWND(), observation_timer);
    GetPager->Key2ValueDelete(observation_timer);
  }

  if (smallicon)
  {
    DestroyIcon(smallicon);
  }

  if (filename)
  {
    delete filename;
  }

  if (class_string)
  {
    delete class_string;
  }

  if (mini_window_bitmap)
  {
    DeleteObject(mini_window_bitmap);
  }

  if (Window::HasFlag("button") && !console_window)
  {
    RemoveProp(real_window, "PAGER_WINDOW_BUTTON");
  }

  if (IsWindow(real_window) && !console_window)
  {
    // Window::SystemMenuReset();
    RemoveProp(real_window, "PAGER_WINDOW");
    RemoveProp(real_window, "PAGER_CURRENT_DESKTOP");
    RemoveProp(real_window, "PAGER_STICKY");
    RemoveProp(real_window, "PAGER_2MONITOR");
    RemoveProp(real_window, "PAGER_TOPMOST");
  }

  flags.clear();


}

int CALLBACK Window::StopCounting(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
  LogMethod;

  KillTimer(hwnd, idTimer);

  HWND win = (HWND) GetPager->Key2ValueGetValue(idTimer);
  GetPager->Key2ValueDelete(idTimer);

  Window *window=GetPager->GetWindowByHWNDAllDesks(win); 
  if (!window)
  {
    return 0;
  }

  window->SetFlashTimerID(0);

  if (window->GetNcEvents() >=4)
  {
    window->SetFlag("flashing");
    window->GetDesktop()->ManageNotification();
  }

  window->SetNcEvents(0);
  window->SetCountingFalse();

  return 0;
}

void Window::ResetFlashing(void)
{
  LogMethod;

  if (flash_timer_id!=0)
  {
    KillTimer(GetPager->GetPagerWindowHWND(), flash_timer_id);
    GetPager->Key2ValueDelete(flash_timer_id);
    flash_timer_id=0;
  }

  DeleteFlag("flashing");
  nc_events=0;
  counting_events=false;
}

void Window::ProcedeNcActivateEvent(void)
{
  LogMethod;

  if (counting_events)
  {
    nc_events++;
    return;
  }

  flash_timer_id=GetPager->CreateUniqueTimerID();
  GetPager->Key2ValueInsert(flash_timer_id, (UINT_PTR) real_window);

  nc_events=0;
  counting_events=true;  
  SetTimer(GetPager->GetPagerWindowHWND(), flash_timer_id, 10000, (TIMERPROC) Window::StopCounting);
}

void Window::SetHiddenStyles(bool withfakepos /*=true*/)
{
  LogMethod;

  SetExStyle(WS_EX_NOACTIVATE | (useToolWindowFlag?WS_EX_TOOLWINDOW:0), 200);
  SetDisabled(200);
  SetFlag("hidden");
  SetProp(real_window, "PAGER_HIDDEN", (void*)1);

  if (withfakepos)
  {
    SetWindowPosFake(false);
  }
}

void Window::RemoveHiddenStyles(bool withfakepos /*true*/)
{
  LogMethod;

  RemoveNoactivateStyle();
  RemoveDisabled(200);

  DeleteFlag("hidden");
  RemoveProp(real_window, "PAGER_HIDDEN");

  if (withfakepos)
  {
    SetWindowPosFake(false);
  }
}

void Window::SetHidden(void)
{
  LogMethod;

  SetFlag("hidden");
  SetProp(real_window, "PAGER_HIDDEN", (void*)1);
}

void Window::DeleteHidden(void)
{
  LogMethod;

  DeleteFlag("hidden");
  RemoveProp(real_window, "PAGER_HIDDEN");
}

bool Window::SetExStyleEx(LONG_PTR old_style_ex, LONG_PTR new_style, int timeout)
{
  LogMethod;

  if (old_style_ex==-1)
  {
    return FALSE;
  }

  return GetSys->SetWindowLongPtrTimeout(real_window, GWL_EXSTYLE, 
    old_style_ex | new_style, timeout);
}

bool Window::SetExStyle(LONG_PTR new_style, int timeout)
{
  LogMethod;

  bool ret=FALSE;

  LONG_PTR old_style_ex = GetSys->GetWindowLongPTR(real_window, GWL_EXSTYLE);
  if (old_style_ex==-1)
  {
    return FALSE;
  }
  ret=GetSys->SetWindowLongPtrTimeout(real_window, GWL_EXSTYLE, 
    old_style_ex | new_style, timeout);
  return ret;
}

bool Window::RemoveExStyle(LONG_PTR remove_style, int timeout)
{
  LogMethod;

  bool ret=FALSE;

  LONG_PTR old_style_ex = GetSys->GetWindowLongPTR(real_window, GWL_EXSTYLE);
  if (old_style_ex==-1)
  {
    return FALSE;
  }

  ret=GetSys->SetWindowLongPtrTimeout(real_window, GWL_EXSTYLE, 
    old_style_ex  &~ remove_style, timeout);

  return ret;

}

bool Window::SetDisabled(int timeout)
{
  LogMethod;

  LONG_PTR old_style = GetSys->GetWindowLongPTR(real_window, GWL_STYLE);
  if (old_style==-1)
  {
    return FALSE;
  }
  return GetSys->SetWindowLongPtrTimeout(real_window, GWL_STYLE, 
    old_style | WS_DISABLED, timeout);
}

bool Window::RemoveDisabled(int timeout)
{
  LogMethod;

  LONG_PTR old_style = GetSys->GetWindowLongPTR(real_window, GWL_STYLE);
  if (old_style==-1)
  {
    return FALSE;
  }

  return GetSys->SetWindowLongPtrTimeout(real_window, GWL_STYLE, 
    old_style  &~ WS_DISABLED, timeout);
}

void Window::SetWindowPosFake(bool set_flag)
{
  LogMethod;

  if (set_flag)
  {
    Window::SetFlag("resize_known");
  }

  SetWindowPos(real_window, NULL,
    0,
    0,
    0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE |
    SWP_NOSENDCHANGING | SWP_ASYNCWINDOWPOS | SWP_FRAMECHANGED);
}

HWND Window::GetRealWindowHWND()
{
  return real_window;
}

RECT Window::RealRectToMini(RECT rect)
{
  LogMethod;

  RECT mini_rect;

  double dmini_x =
    ((double) (GetPager->GetDesktopWidth() * rect.left)) /
    (double) GetSys->GetScreenWidth();
  double dmini_y =
    ((double) (GetPager->GetDesktopHeight() * rect.top)) /
    (double) GetSys->GetScreenHeight();
  int mini_x = GetSys->Round(dmini_x);
  int mini_y = GetSys->Round(dmini_y);

  double dmini_w =
    ((double) (GetPager->GetDesktopWidth() * (rect.right - rect.left))) /
    (double) GetSys->GetScreenWidth();
  double dmini_h =
    ((double) (GetPager->GetDesktopHeight() * (rect.bottom - rect.top))) /
    (double) GetSys->GetScreenHeight();

  int mini_w = GetSys->Round(dmini_w);
  int mini_h = GetSys->Round(dmini_h);

  if (mini_w % 2 == 0)
    mini_w--;

  if (mini_h % 2 == 1)
    mini_h++;

  mini_rect.left = mini_x;
  mini_rect.top = mini_y;
  mini_rect.right = mini_rect.left + mini_w;
  mini_rect.bottom = mini_rect.top + mini_h;

  return mini_rect;
}

void Window::SetDesktop(Desktop * _desktop)
{
  LogMethod;

  desktop = _desktop;
}

RECT Window::GetMiniWindowRect()
{
  LogMethod;

  return mini_window_rect;
}

void Window::SetMiniWindowRect(RECT & rect)
{
  LogMethod;

  mini_window_rect = rect;
}

POINT Window::GetPadding()
{
  LogMethod;

  return padding;
}

void Window::SetPadding(POINT pt)
{
  LogMethod;

  POINT pad;
  pad.x = pt.x - mini_window_rect.left;
  pad.y = pt.y - mini_window_rect.top;
  padding = pad;
}

Desktop *Window::GetDesktop()
{
  LogMethod;

  return desktop;
}

bool Window::WasDragged()
{
  LogMethod;

  return dragged;
}

void Window::DragOn()
{
  LogMethod;

  dragged = TRUE;
}

void Window::DragOff()
{
  LogMethod;

  dragged = FALSE;
}

void Window::SetFlag(char *key)
{
  LogMethod;

  string mystring(key);
  flags.insert(make_pair(mystring, 1));

  //DEVMODE
  //SetProp (real_window, key, (void *) 1);
}

bool Window::HasFlag(char *key)
{
  try
  {
    LogMethod;

    string mystring(key);

    map < string, int >::iterator iter = flags.find(mystring);
    if (iter != flags.end())
    {
      return TRUE;
    } else
    {
      return FALSE;
    }
  } catch (...)
  {
    return false;
  }
}

void Window::DeleteFlag(char *key)
{
  LogMethod;

  string mystring(key);
  flags.erase(mystring);

  //DEVMODE
  // RemoveProp (real_window, key);

}

void Window::CreateMiniWindowBitmap(int mini_width , int mini_height)
{
  LogMethod;

  //trace("create bitmap width: %d, height: %d\n", mini_width, mini_height);

  HDC hdc = CreateDC("DISPLAY", NULL, NULL, NULL);

  RECT bitmap_rect;
  SetRect(&bitmap_rect, 0, 0, mini_width, mini_height);

  mini_window_bitmap = CreateCompatibleBitmap(hdc, mini_width, mini_height);
  HDC mini_window_hdc = CreateCompatibleDC(hdc);
  SelectObject(mini_window_hdc, mini_window_bitmap);

  /*background */
  RECT white_rect;
  SetRect(&white_rect, 1, 1, mini_width + 1, mini_height + 1);
  FillRect(mini_window_hdc, &white_rect, GetSys->GetBrushWhite());

  /*frame */
  FrameRect(mini_window_hdc, &bitmap_rect, GetSys->GetBrushFrameColor());
  SelectObject(mini_window_hdc, GetSys->GetPenFrameColor());
  MoveToEx(mini_window_hdc, 1, 1, NULL);
  LineTo(mini_window_hdc, mini_width - 1, 1);

  //trace("mini_widht: %d, mini_height: %d\n", mini_width, mini_height);

  int small_size = min(mini_width-4 , mini_height-5);
  if (small_size % 2 != 0)
  {
    small_size--;
  }
  //trace("small size: %d\n", small_size);

  int icon_x =2+ (mini_width-4) / 2 - small_size / 2;
  int icon_y = 3+ (mini_height-5) / 2 - small_size / 2;

  //trace("icon_x: %d, icon_y: %d\n", icon_x, icon_y);

  HBITMAP bigbitmap = CreateCompatibleBitmap(hdc, 48, 48);

  HDC hdc_big = CreateCompatibleDC(hdc);
  SelectObject(hdc_big, bigbitmap);

  DrawIconEx(hdc_big, 0, 0, smallicon,
    48, 48, 0, GetSys->GetBrushWhite(), DI_NORMAL);

  SetStretchBltMode(mini_window_hdc, HALFTONE);
  SetBrushOrgEx(mini_window_hdc, 0, 0, NULL);

  StretchBlt(mini_window_hdc, icon_x, icon_y, small_size, small_size,
    hdc_big, 0, 0, 48, 48, SRCCOPY);

  DeleteObject(bigbitmap);
  DeleteDC(hdc_big);
  DeleteDC(mini_window_hdc);
  DeleteDC(hdc);
}

void Window::UpdateMiniWindow(void)
{
  LogMethod;

  RECT rect_real;

  if (HasFlag("hidden"))
  {
    trace("updated mini window is hidden\n");
    CopyRect(&rect_real, &backup_rect);
  } else
  {
    if (!GetWindowRect(real_window, &rect_real))
    {
      return;
    }

    if (wm_ex_transparent)
    {
      trace("window has transparent inupdate: %x\n", real_window);
      return;
    }

    RECT inter;
    if (IntersectRect(&inter, &GetSys->GetHiddenRect(), &rect_real))
    {
      trace("backup rect is in hidden rect\n");
      return;
    }


    if (rect_real.left == -32000 && rect_real.top == -32000)
    {
      //trace("normal to iconic: %x\n", real_window);
      SetFlag("iconic");
      return;
    } else
    {
      if (HasFlag("iconic"))
      {
        // trace("iconic to normal: %x\n", real_window);
        DeleteFlag("iconic");
        RemoveProp(real_window, "PAGER_ICONIC");
      }
    }
  }

  CopyRect(&backup_rect, &rect_real);


  if (GetSys->GetMultiMonitorMode()==ONE_BIG_SCREEN)
  {
    RECT rect_virtual=GetSys->RealToVirtualScreenRect(rect_real);
    // trace("rect virtual left: %d, rect virtual top: %d\n", 
    //  rect_virtual.left, rect_virtual.top);
    mini_window_rect = Window::RealRectToMini(rect_virtual);
  } else
  {  
    mini_window_rect = Window::RealRectToMini(rect_real);
  }

  GetPager->DesktopToPager(desktop, mini_window_rect);

  mini_window_rect.bottom -= 1;
  mini_window_rect.right -= 1;
  mini_window_rect.left += 1;
  mini_window_rect.top += 1;

  if ((mini_window_rect.bottom - mini_window_rect.top) < 6)
  {
    mini_window_rect.bottom = mini_window_rect.top + 6;
  }

  if ((mini_window_rect.right - mini_window_rect.left) < 8)
  {
    mini_window_rect.right = mini_window_rect.left + 8;
  }

  int new_bitmap_width=mini_window_rect.right - mini_window_rect.left;
  int new_bitmap_height=mini_window_rect.bottom - mini_window_rect.top;


  if (new_bitmap_height % 2 == 0)
  {
    new_bitmap_height--;
    mini_window_rect.bottom--;
  }

  if (mini_window_bitmap==NULL)
  {
    Window::CreateMiniWindowBitmap(new_bitmap_width, new_bitmap_height);
    return;
  }

  BITMAP old_bitmap;
  if (!GetObject(mini_window_bitmap, sizeof(BITMAP), (LPSTR) &old_bitmap))
  {
    DeleteObject(mini_window_bitmap);
    Window::CreateMiniWindowBitmap(new_bitmap_width, new_bitmap_height);
    return;
  }

  if (old_bitmap.bmWidth==new_bitmap_width && old_bitmap.bmHeight==new_bitmap_height)
  {
    DeleteObject(&old_bitmap);
    return;
  }

  DeleteObject(&old_bitmap);
  DeleteObject(mini_window_bitmap);
  Window::CreateMiniWindowBitmap(new_bitmap_width,
    new_bitmap_height);

}

void Window::DrawMiniWindow(Gdiplus::Graphics *g, int x, int y, bool sticky)
{
  LogMethod;

  if (DeathCheckPositiv()) return;

  if (!smallicon || !mini_window_bitmap)
  {
    trace("mini no smallicon\n");
    return;
  }

  RECT mini_rect;
  CopyRect(&mini_rect, &mini_window_rect);
  int mini_width = mini_rect.right - mini_rect.left;
  int mini_height = mini_rect.bottom - mini_rect.top;

  if (sticky)
  {
    mini_rect.left = x;
    mini_rect.top = y;
    mini_rect.right = mini_rect.left + mini_width;
    mini_rect.bottom = mini_rect.top + mini_height;
  }

  RECT pager_tmp;
  SetRect(&pager_tmp, 0, 0, GetPager->GetPagerWidth(), GetPager->GetPagerHeight());
  RECT inter;
  if (!IntersectRect(&inter, &mini_rect, &pager_tmp))
  {
    trace("mini not intersect\n");
    return;
  }

  vector < Desktop * >desklist = GetPager->GetDesktopList();

  vector < Desktop * >::iterator desk_iter = desklist.begin();
  while (desk_iter != desklist.end())
  {
    RECT desk_rect = (*desk_iter)->GetDesktopRect();
    desk_rect.bottom++;
    desk_rect.right++;

    RECT intersect;
    if (IntersectRect(&intersect, &desk_rect, &mini_rect))
    {
      RECT intersect_relative;
      SetRect(&intersect_relative,
        intersect.left - mini_rect.left,
        intersect.top - mini_rect.top,
        intersect.right - mini_rect.right, intersect.bottom - mini_rect.bottom);

      Bitmap bitmap(mini_window_bitmap, NULL);
      g->DrawImage(      
        &bitmap,
        intersect.left,
        intersect.top,
        intersect_relative.left,
        intersect_relative.top,
        intersect.right - intersect.left,
        intersect.bottom - intersect.top,
        UnitPixel);

    }

    desk_iter++;
  }
}

bool Window::MiniWindowStartDragging( RECT &main_window_rect, Desktop* current_desktop, int desktop_padding_horizontal, int desktop_padding_vertical)
{
  LogMethod;

  if (HasFlag("hangup"))
  {
    trace("ignore hang up on mouse down\n");
    return false;;
  }

  if (GetSys->CheckIfWindowDoRespond(real_window, 50))
  {
    return false;
  }

  LONG_PTR res=GetSys->GetWindowLongPTR(real_window, GWL_EXSTYLE);
  if (res==-1)
  {
    return false;
  }

  if (res & WS_EX_TOPMOST)
  {
    GetSys->ShowMessageBoxModless("Sorry but you can not move a window which is on top of other windows.",
      MB_ICONINFORMATION);
    return false;
  }

  if (!GetWindowRect(real_window, &mini_rect_start))
  {
    return false;
  }

  SetProp(real_window, "PAGER_MINIWINDOW", (void*) 1);

  trace("screen points: x1: %d, y1: %d, x2: %d, y2: %d\n", main_window_rect.left, main_window_rect.top, main_window_rect.right, main_window_rect.bottom);

  if (desktop->GetDesktopNumber()!=current_desktop->GetDesktopNumber())
  {
    trace("set outer space on mouse click\n");
    SetFlag("outer_space");
    if (HasFlag("flashing"))
    {
      desktop->StopFlash();
      ResetFlashing();
    }

    RemoveHiddenStyles();

    HWND owner = GetWindow(real_window, GW_OWNER);
    if (owner && owner != real_window)
    {
      Window *owni=desktop->GetWindowByHWND(owner);
      if (owni && owni->HasFlag("ignore"))
      {
        trace("clicked mini window has owner\n");
        trace("remove disabled/noativte to owner on click\n");
        owni->RemoveDisabled(200);
        owni->RemoveExStyle(WS_EX_NOACTIVATE, 200);
        owni->SetWindowPosFake();
      }
    }

  } else
  {
    DeleteFlag("outer_space");
    trace("set backup rect\n");
  }

  main_window_rect.top += (padding.y + desktop_padding_vertical);
  main_window_rect.left += (padding.x + desktop_padding_horizontal);

  int mini_width=mini_window_rect.right - mini_window_rect.left;
  main_window_rect.right-= (mini_width-padding.x + desktop_padding_horizontal);

  int mini_height=mini_window_rect.bottom - mini_window_rect.top;

  main_window_rect.bottom-= 2; //((mini_height-clicked_mini_window->GetPadding().y) + GetPagerPaddingVertical());
  hover_desktop=NULL;
  return true;

}

void Window::MiniWindowMoveOnCurrentDesktop(int new_x, int new_y )
{
  LogMethod;

  bool from_outer_space=false;

  if (HasFlag("outer_space"))
  {
    trace("outer space to current.....................\n");
    DeleteFlag("outer_space");
    from_outer_space=TRUE;
  }

  SetWindowPos(real_window, NULL, new_x,
    new_y >= 0 ? new_y : 0, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE |
    SWP_FRAMECHANGED);


  int width=mini_rect_start.right-mini_rect_start.left;
  int height=mini_rect_start.bottom-mini_rect_start.top;

  SetRect(&backup_rect, new_x, new_y >= 0 ? new_y : 0, new_x+width, (new_y >= 0 ? new_y : 0)+height);

  if (from_outer_space)
  {
    HWND owner = GetWindow(real_window, GW_OWNER);
    if (owner && owner !=real_window)
    {
      Window *owni=desktop->GetWindowByHWND(owner);
      if (owni && owni->HasFlag("ignore"))
      {
        GetSys->ShowTaskButton(owner);
      }
      else
      {
        GetSys->ShowTaskButton(real_window);
      }
    } else
    {
      GetSys->ShowTaskButton(real_window);
    }
  }
}

void Window::MiniWindowMoveOnForeignDesktop( int new_x, int new_y )
{
  LogMethod;

  bool to_outer_space=FALSE;
  if (!HasFlag("outer_space"))
  {
    trace("current..to outer space ........\n");
    SetFlag("outer_space");
    to_outer_space=TRUE;
  }

  SetWindowPos(real_window, NULL, new_x + 15000,
    new_y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

  int width=mini_rect_start.right-mini_rect_start.left;
  int height=mini_rect_start.bottom-mini_rect_start.top;

  trace("outerspace backup width: %d, height: %d\n", width, height);
  SetRect(&backup_rect, new_x, new_y >= 0 ? new_y : 0, new_x+width, (new_y >= 0 ? new_y : 0)+height);
  trace("backuprect %d,%d,%d,%d\n", backup_rect.left, backup_rect.top, backup_rect.right,backup_rect.bottom);

  if (to_outer_space)
  {
    HWND owner = GetWindow(real_window, GW_OWNER);
    if (owner && owner != real_window)
    {
      Window *owni=desktop->GetWindowByHWND(owner);
      if (owni && owni->HasFlag("ignore"))
      {
        GetSys->HideTaskButton(owner);
      } else
      {
        GetSys->HideTaskButton(real_window);
      }
    } else
    {
      GetSys->HideTaskButton(real_window);
    }
  }
}

void Window::MiniWindowMoving( Desktop *current_desktop, Desktop * _hover_desktop, POINT &pt )
{
  LogMethod;

  hover_desktop=_hover_desktop;

  if (!dragged)
  {
    dragged=true;
  }

  //trace("movemove desktop: %d\n", hover_desktop->GetDesktopNumber());

  RECT rect_mini = mini_window_rect;

  int width = rect_mini.right - rect_mini.left;
  int height = rect_mini.bottom - rect_mini.top;

  RECT mini_new;
  CopyRect(&mini_new, &rect_mini);

  mini_new.left = pt.x - padding.x;
  mini_new.top = pt.y - padding.y;
  mini_new.right = mini_new.left + width;
  mini_new.bottom = mini_new.top + height;
  CopyRect(&mini_window_rect, &mini_new);


  RECT mini_desk;
  CopyRect(&mini_desk, &mini_new);
  GetPager->PagerToDesktop(hover_desktop, mini_desk);

  int new_x = (GetSys->GetScreenWidth() * mini_desk.left) / GetPager->GetDesktopWidth();
  int new_y = (GetSys->GetScreenHeight() * (mini_desk.top)) / GetPager->GetDesktopHeight();

  trace("new_x: %d, new_y: %d\n", new_x, new_y);


  if (GetSys->GetMultiMonitorMode()==ONE_BIG_SCREEN)
  {
    GetSys->VirtualToRealScreenXY(new_x, new_y);
  } 

  int hover_left = hover_desktop->GetDesktopRect().left;
  int hover_right = hover_desktop->GetDesktopRect().right;
  int mini_left = mini_new.left;
  int mini_right = mini_new.right;

  bool setpos = FALSE;

  if (_hover_desktop->GetDesktopNumber() == current_desktop->GetDesktopNumber())
  {
    MiniWindowMoveOnCurrentDesktop(new_x, new_y);
  } else
  {
    MiniWindowMoveOnForeignDesktop(new_x, new_y);
  }	
}

void Window::MiniWindowDragStop(Desktop* current_desktop)
{
  LogMethod;

  RemoveProp(real_window, "PAGER_MINIWINDOW");

  Desktop *released_desk = hover_desktop;
  if (released_desk == NULL)
  {
    dragged=false;
    return;
  }

  bool on_current=FALSE;
  if (released_desk->GetDesktopNumber()==current_desktop->GetDesktopNumber())
  {
    on_current=true;
  }

  if (dragged && (desktop->GetDesktopNumber() == released_desk->GetDesktopNumber()))
  {
    if (!on_current)
    {
      trace("gedrückt und released auf gleichem hidden desktop\n");
      SetHiddenStyles();
    }

    dragged=false;
    hover_desktop=NULL;
    return;
  }

  desktop->RemoveWindow(this, FALSE); 

  HWND owner = GetWindow(real_window, GW_OWNER);
  if (owner && owner != real_window)
  {
    Window *owni=desktop->GetWindowByHWND(owner);
    if (owni && owni->HasFlag("ignore"))
    {
      trace("mini window has owner\n");
      desktop->RemoveButton(owner);
      desktop->RemoveWindow(owni, FALSE);
      released_desk->AddButton(owner);
      released_desk->AddWindow(owni);
      if (!on_current)
      {
        trace("set disabled/noativte to owner on click relase\n");
        owni->SetDisabled(200);
        owni->SetExStyle(WS_EX_NOACTIVATE, 200);
        owni->SetWindowPosFake();
      }
    }
  } else
  {
    desktop->RemoveButton(real_window);
    released_desk->AddButton(real_window);
  }

  if (!on_current)
  {
    trace("mouse up nicht auf current\n");
    SetHiddenStyles();
  }

  released_desk->AddWindow(this);
  released_desk->SetActiveWindow(this);
  SystemMenuUpdate();
  
  dragged=false;
  hover_desktop=NULL;
}

bool Window::IsConsoleWindow(void)
{
  LogMethod;

  return console_window;
}

bool Window::GetWmExTransparent(void)
{
  LogMethod;
  return wm_ex_transparent;
}

std::wstring *Window::GetClassString(void)
{
  LogMethod;

  return class_string;
}

std::wstring *Window::GetFileName(void)
{
  LogMethod;

  return filename;
}

void Window::ResetObservationTimer(void)
{
  LogMethod;

  observation_timer=0;
}

Desktop *Window::GetHoverDesktop(void)
{

  LogMethod;
  return hover_desktop;
}

void Window::SetHoverDesktop(Desktop *hover)
{
  LogMethod;

  hover_desktop=hover;
}

void Window::SetDummyWindowNULL(void)
{
  LogMethod;

  dummy_window=NULL;
}

HangUpInfoWindow* Window::GetDummyWindow(void)
{
  LogMethod;

  return dummy_window;
}

bool Window::WasDuringHide(void)
{
  LogMethod;

  return duringhide;
}

void Window::IncreaseHangupCount(void)
{
  LogMethod;
  hangup_count++;
}

int Window::GetHangupCount(void)
{
  LogMethod;
  return hangup_count;
}

void Window::ResetHangupCount(void)
{
  LogMethod;
  hangup_count=0;
} 

bool Window::DeathCheckPositiv(void)
{
  LogMethod;

  if (dead)
  {
    trace("dead: %x\n", real_window);
    return true;
  }

  if (!IsWindow(real_window))
  {
    trace("is not window\n");
    dead=true;
    return true;
  } else
  {
    return false;
  }
}

bool Window::IsDead(void)
{
  LogMethod;
  return dead;
}

void Window::SetDeath(void)
{
  LogMethod;
  dead=true;
}

HICON Window::GetSmallIcon(void)
{
  LogMethod;
  return smallicon;
}

void Window::SetSmallIcon(HICON icon)
{
  LogMethod;
  smallicon = icon;
}

void Window::SetBackupRect(RECT new_one)
{
  LogMethod;
  CopyRect(&backup_rect, &new_one);
}

void Window::SetFlashTimerID(UINT id)
{
  LogMethod;
  flash_timer_id=id;
}

int Window::GetNcEvents(void)  
{
  LogMethod;
  return nc_events;
}

void Window::SetCountingFalse(void)
{ 
  LogMethod;
  counting_events=false;
}

void Window::SetNcEvents(int i)
{
  LogMethod;
  nc_events=i;
}

bool Window::RunningInsideVisualStudio(void)
{
  return runningInsideVisualStudio;
}
