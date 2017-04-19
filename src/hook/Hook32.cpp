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

#define _WIN32_WINNT 0x0501 //subclass functions need this
#define WINVER 0x0501
#define _WIN32_WINDOWS 0x0501
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <cstdio>
#include <commctrl.h> //Subclassing

#include "Hook.h"
#include "..\common\Messages.h"
#include "..\common\smartfix.h"

using namespace std;

void trace(const char *str, ...)
{
#ifdef _DEBUG
  char buf[2048];
  va_list ptr;
  va_start(ptr, str);
  vsprintf_s(buf, 2048, str, ptr);
  OutputDebugStringA(buf);
#endif
}

#pragma data_seg(".SHARDATA")
HHOOK trayHook=NULL;
HHOOK menuHook=NULL;
HWND mainWindow = NULL;
HWND trayWindow = NULL;
HWND taskWindow = NULL;
HWND listWindow = NULL;
UINT message = 0;
int numDesktops=0;
int moveAndSwitchModifier=0;
#pragma data_seg()
#pragma comment(linker, "/section:.SHARDATA,rws")

void SubclassOnWindowPosChanging( HWND hWnd, LPARAM lParam );
LRESULT __stdcall MenuProc(int nCode, WPARAM wParam, LPARAM lParam);

static HMODULE ModuleFromAddress(PVOID pv) 
{
  MEMORY_BASIC_INFORMATION mbi;
  return ((::VirtualQuery(pv, &mbi, sizeof(mbi)) != 0) 
    ? (HMODULE) mbi.AllocationBase : NULL);
}

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) 
{
  switch (ul_reason_for_call) 
  {
  case DLL_PROCESS_ATTACH:
    {
      DisableThreadLibraryCalls((HINSTANCE)hModule);
    }
    break;

  }
  return TRUE;
}

#ifdef _M_X64
EXPORT bool UnInstallFilters64()
#else
EXPORT bool UnInstallFilters32()

#endif
{
  UnhookWindowsHookEx(trayHook);
  UnhookWindowsHookEx(menuHook);
  return TRUE;
}

void SubclassOnWindowPosChanging( HWND hWnd, LPARAM lParam )
{
  if (GetProp(hWnd, "PAGER_HIDDEN"))
  {
    if (!(((WINDOWPOS*) lParam)->flags & SWP_NOMOVE))
    {
      if (GetProp(hWnd, "PAGER_ICONIC"))
      {
        SetProp(hWnd, "PAGER_ICONIC_X", (void*) ((WINDOWPOS*) lParam)->x);
        SetProp(hWnd, "PAGER_ICONIC_Y", (void*) ((WINDOWPOS*) lParam)->y);
        SetProp(hWnd, "PAGER_ICONIC_W", (void*) ((WINDOWPOS*) lParam)->cx);
        SetProp(hWnd, "PAGER_ICONIC_H", (void*) ((WINDOWPOS*) lParam)->cy);
        SendNotifyMessage(mainWindow, MSG_ADOBE_SHIT, (LPARAM) hWnd, (LPARAM) 0);
        ((WINDOWPOS*) lParam)->x+=15000;
      } else
      {
        trace("not iconic but move detected\n");
        ((WINDOWPOS*) lParam)->flags=(((WINDOWPOS*) lParam)->flags | SWP_NOMOVE);
      }
    }
  }
}

LONG_PTR GetWindowLongPTR(HWND hWnd, int nIndex)
{
  SetLastError(0);
  LONG_PTR res=GetWindowLongPtr(hWnd, nIndex);
  if (res==0)
  {
    if (GetLastError()!=ERROR_SUCCESS)
    {
      return -1;
    }
  }
  return res;
}

void ToggleOnTop(HWND window)
{
  static SYSTEMTIME last_time;

  SYSTEMTIME current_time;
  GetSystemTime(&current_time);

  if (current_time.wYear ==last_time.wYear &&
    current_time.wMonth ==last_time.wMonth &&
    current_time.wDay==last_time.wDay  &&
    current_time.wDayOfWeek==last_time.wDayOfWeek &&
    current_time.wHour==last_time.wHour &&
    current_time.wMinute ==last_time.wMinute &&
    current_time.wSecond == last_time.wSecond)
  {
    int span= abs(((int)current_time.wMilliseconds-(int) last_time.wMilliseconds));
    if (span < 100)
    {
      last_time=current_time;
      return;
    }
  }

  last_time=current_time;

  LONG_PTR res=GetWindowLongPTR(window, GWL_EXSTYLE);
  if (res==-1)
  {
    return;
  }

  SetWindowPos(window,
    (res & WS_EX_TOPMOST)?HWND_NOTOPMOST:HWND_TOPMOST, 0, 0, 0, 0, 
    SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED);

  if (res & WS_EX_TOPMOST)
  {
    RemoveProp(window, "PAGER_TOPMOST");
  } else
  {
    SetProp(window, "PAGER_TOPMOST", (void*) 1);
  }

  //trigger api call
  HMENU system_menu = GetSystemMenu(window, FALSE);
}

void ToggleSticky(HWND window)
{
  static SYSTEMTIME last_time;

  SYSTEMTIME current_time;
  GetSystemTime(&current_time);

  if (current_time.wYear ==last_time.wYear &&
    current_time.wMonth ==last_time.wMonth &&
    current_time.wDay==last_time.wDay  &&
    current_time.wDayOfWeek==last_time.wDayOfWeek &&
    current_time.wHour==last_time.wHour &&
    current_time.wMinute ==last_time.wMinute &&
    current_time.wSecond == last_time.wSecond)
  {
    int span= abs(((int)current_time.wMilliseconds-(int) last_time.wMilliseconds));

    if (span < 100)
    {
      last_time=current_time;
      return;
    }
  }

  last_time=current_time;
  PostMessage(mainWindow, message, MSG_TOGGLESTICKY, (LPARAM) window);
}

void MoveWorkspace(HWND window, int id )
{
  static SYSTEMTIME last_time;

  SYSTEMTIME current_time;
  GetSystemTime(&current_time);

  if (current_time.wYear ==last_time.wYear &&
    current_time.wMonth ==last_time.wMonth &&
    current_time.wDay==last_time.wDay  &&
    current_time.wDayOfWeek==last_time.wDayOfWeek &&
    current_time.wHour==last_time.wHour &&
    current_time.wMinute ==last_time.wMinute &&
    current_time.wSecond == last_time.wSecond)
  {
    int span= abs(((int)current_time.wMilliseconds-(int) last_time.wMilliseconds));
    if (span < 100)
    {
      last_time=current_time;
      return;
    }
  }

  last_time=current_time;

  int pint= id -MENU_ITEM;
  short state=GetKeyState(moveAndSwitchModifier);
  if (state & 0x80)
  {
    SendNotifyMessage(mainWindow, MSG_MOVE_TO_WORKSPACE_SHIFTED, (WPARAM) window, (LPARAM) pint);
  } else
  {
    SendNotifyMessage(mainWindow, MSG_MOVE_TO_WORKSPACE, (WPARAM) window, (LPARAM) pint);
  }
}

int SubclassOnSysCommand(HWND hWnd, WPARAM wParam)
{
  int id= LOWORD(wParam);

  if (id == MENU_ITEM+numDesktops+1)
  {
    ToggleOnTop(hWnd);
    return 1;
  }

  if (id == MENU_ITEM+numDesktops+2) 
  {
    ToggleSticky(hWnd);
    return 1;
  }

  if (id >= MENU_ITEM+1 && id <= MENU_ITEM+numDesktops)
  {
    MoveWorkspace(hWnd, id);
    return 1;
  }

  return 0;
}

void WndProcSystemMenuInitTrigger( CWPSTRUCT * pData )
{
  //only trigger the api call
  HMENU system_menu = GetSystemMenu(pData->hwnd, FALSE);
}

LRESULT CALLBACK SubClassWndProc(HWND hWnd, UINT uMsg, WPARAM wParam,
                                 LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
{
  try
  {
    switch(uMsg)
    {
    case WM_MOVE:
      {
        if (GetProp(hWnd, "PAGER_MINIWINDOW"))
        {
          return 1;
        }
      }
      break;

    case WM_WINDOWPOSCHANGING:
      {
        SubclassOnWindowPosChanging(hWnd, lParam);
      }
      break;

    case WM_DESTROY:
      {
        RemoveWindowSubclass(hWnd, SubClassWndProc, 0);
      }
      break;

    case WM_WINDOWPOSCHANGED: 
      {
        WINDOWPOS *pos=(WINDOWPOS*) lParam;
        if (pos->flags & SWP_HIDEWINDOW)
        {
          RemoveWindowSubclass(hWnd, SubClassWndProc, 0);
        }
      }
      break;

    case WM_SYSCOMMAND:
      {
        int res=SubclassOnSysCommand(hWnd, wParam);
        if (res==1)
        {
          return 1;
        }

      } break;
    }

    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
  } catch (...)
  {
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
  }
}

LRESULT __stdcall CallWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  try
  {
    if(nCode == HC_ACTION ) 
    {
      CWPSTRUCT *pData=(CWPSTRUCT *) lParam;

      switch (pData->message)
      {
      case MSG_SUBCLASS:
        {
          char buf[20];
          if (!GlobalGetAtomNameA((ATOM) pData->wParam, buf, sizeof(buf)) || 
            strncmp(buf, "WindowsPagerMessage", 20)!=0) 
          {
            break;
          }

          SetWindowSubclass(pData->hwnd, SubClassWndProc, 0, 0);
        }
        break;

      case MSG_RESET_SUBCLASS:
        {
          char buf[20];
          if (!GlobalGetAtomNameA((ATOM) pData->wParam, buf, sizeof(buf)) || 
            strncmp(buf, "WindowsPagerMessage", 20)!=0) 
          {
            break;
          }

          RemoveWindowSubclass(pData->hwnd, SubClassWndProc, 0);
        }
        break;

      case MSG_RESET_SUBCLASS_CRASH:
        {
          RemoveWindowSubclass(pData->hwnd, SubClassWndProc, 0);
        }
        break;

      case MSG_SYSTEM_MENU_INIT_TRIGGER:
        {

          char buf[20];
          if (!GlobalGetAtomNameA((ATOM) pData->wParam, buf, sizeof(buf)) || 
            strncmp(buf, "WindowsPagerMessage", 20)!=0) 
          {
            break;
          }

          WndProcSystemMenuInitTrigger(pData);
        }
        return 0;

      case WM_SIZE: 
        {
          if (pData->hwnd == trayWindow ||
            pData->hwnd == taskWindow ||
            pData->hwnd == listWindow)
          {
            PostMessage(mainWindow, message, MSG_TRAYSIZED, (LPARAM) pData->hwnd);
            break;
          }
        }
        break;

      case WM_DESTROY:
        {
          PostMessage(mainWindow, message, MSG_DESTROYED, (LPARAM) pData->hwnd);
        }
        break;

      case WM_WINDOWPOSCHANGED: 
        {
          WINDOWPOS *pos=(WINDOWPOS*) pData->lParam;

          if (pos->flags & SWP_SHOWWINDOW)
          {
            PostMessage(mainWindow, message, MSG_SHOW, (LPARAM) pData->hwnd);
          } else if (pos->flags & SWP_HIDEWINDOW)
          {
            PostMessage(mainWindow, message, MSG_HIDE, (LPARAM) pData->hwnd);
          } else 
          {
            PostMessage(mainWindow, message, MSG_SIZED, (LPARAM) pData->hwnd);
          }
        }
        break;

      case WM_NCACTIVATE:
        {
          if (pData->wParam == TRUE)
          {
            PostMessage(mainWindow, message, MSG_NCACTIVATE, (LPARAM) pData->hwnd);
          }
        }
        break;

      case WM_ACTIVATE: 
        {
          if (pData->wParam==WA_CLICKACTIVE || pData->wParam==WA_ACTIVE)
          {
            PostMessage(mainWindow, message, MSG_ACTIVATE, (LPARAM) pData->hwnd);
          }
        }
        break;  

      case WM_SYSCOMMAND:
        {
          int res=SubclassOnSysCommand(pData->hwnd, pData->wParam);
          if (res==1)
          {
            return 1;
          }
        }
        break;

      default:
        break;  
      }
    }

    return(CallNextHookEx(trayHook, nCode, wParam, lParam));
  } catch (...)
  {
    return(CallNextHookEx(trayHook, nCode, wParam, lParam));
  }
}

LRESULT __stdcall MenuProc(int nCode, WPARAM wParam, LPARAM lParam)
{
  try
  {
    static DWORD last_time=0;
    MSG *msg = (MSG *) lParam;

    if(nCode < 0)
    {
      return CallNextHookEx(menuHook, nCode, wParam, lParam);
    }

    if ((msg->message == WM_SYSCOMMAND))
    {
      if (msg->time == last_time)
      {
        return CallNextHookEx(menuHook, nCode, wParam, lParam);
      }

      last_time=msg->time;
      int id= LOWORD(msg->wParam);

      if (id == MENU_ITEM+numDesktops+1)
      {
        ToggleOnTop(msg->hwnd);
      } else  if (id == MENU_ITEM+numDesktops+2) 
      {
        ToggleSticky(msg->hwnd);
      } else  if (id >= MENU_ITEM+1 && id <= MENU_ITEM+numDesktops)
      {
        MoveWorkspace(msg->hwnd, id);
      }
    }

    return CallNextHookEx(menuHook, nCode, wParam, lParam);

  } catch (...)
  {
    return CallNextHookEx(menuHook, nCode, wParam, lParam);
  }
}

#ifdef _M_X64
EXPORT bool InitHooksDll64(HWND window, HWND tray_window,
                           HWND task_window, HWND list_window, UINT msg, int _numDesktops, 
                           int _moveAndSwitchModifier)
#else
EXPORT bool InitHooksDll32(HWND window, HWND tray_window,
                           HWND task_window, HWND list_window, UINT msg, int _numDesktops, 
                           int _moveAndSwitchModifier)
#endif
{
  mainWindow = window;
  trayWindow = tray_window;
  taskWindow = task_window;
  listWindow = list_window;
  message = msg;
  numDesktops = _numDesktops;
  moveAndSwitchModifier=_moveAndSwitchModifier;

  trayHook = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC) CallWndProc, ModuleFromAddress(CallWndProc), 0);
  menuHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC) MenuProc,  ModuleFromAddress(MenuProc), 0);

  return TRUE;
}