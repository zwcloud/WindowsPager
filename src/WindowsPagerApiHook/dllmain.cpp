// dllmain.cpp : Defines the entry point for the DLL application.

/* 
*  MinHook - Minimalistic API Hook Library	
*  Copyright (C) 2009 Tsuda Kageyu. All rights reserved.
*  Modification for WindowsPager by Jochen Baier (C) 2011
*  
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*  
*  1. Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*  3. The name of the author may not be used to endorse or promote products
*     derived from this software without specific prior written permission.
*  
*  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
*  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
*  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
*  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
*  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
*  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
*  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "stdafx.h"
#include "MinHook.h"

#define MSG_PROCESS_DETACH 35920
#define MSG_PROCESS_ATTACH 35921

#define MENU_ITEM (61700)
#define MENU_SEP1 (MENU_ITEM-1)
#define MENU_SEP2 (MENU_ITEM-2)

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

RECT hidden_rect;
int num_desktops =4;

typedef BOOL (WINAPI *GETWINDOWRECT) ( __in HWND hWnd,  __out LPRECT lpRect);
typedef BOOL (WINAPI *GETWINDOINFO)  (__in HWND hwnd,  __inout  PWINDOWINFO pwi);
typedef BOOL (WINAPI *SETFOREGROUNDWINDOW) (__in  HWND hWnd);
typedef BOOL (WINAPI *GETWINDOWPLACEMENT) (__in     HWND hWnd,  __inout  WINDOWPLACEMENT *lpwndpl);
typedef HMENU (WINAPI *GETSYSTEMMENU) (__in HWND hWnd, __in BOOL bRevert);
typedef HWND  (WINAPI *SETACTIVEWINDOW) (__in  HWND hWnd);
typedef BOOL (WINAPI *BRINGWINDOWTOTOP) (__in  HWND hWnd);
typedef BOOL (WINAPI *SHOWWINDOW) (__in  HWND hWnd, __in  int nCmdShow);
typedef BOOL (WINAPI *SETWINDOWPOS) ( __in      HWND hWnd,  __in_opt  HWND hWndInsertAfter, __in      int X,
                                     __in      int Y,  __in      int cx,  __in      int cy,  __in      UINT uFlags);


GETWINDOWRECT fpGetWindowRect= NULL;
GETWINDOINFO fpGetWindowInfo= NULL;
SETFOREGROUNDWINDOW fpSetForegroundWindow= NULL;
GETWINDOWPLACEMENT fpGetWindowPlacement = NULL;
GETSYSTEMMENU fpGetSystemMenu = NULL;
SETACTIVEWINDOW fpSetActiveWindow =NULL;
BRINGWINDOWTOTOP fpBringWindowToTop= NULL;
SHOWWINDOW fpShowWindow = NULL;
SETWINDOWPOS fpSetWindowPos = NULL;

BOOL WINAPI DetourGetWindowRect( __in HWND hWnd,  __out LPRECT lpRect)
{
  try
  {
    RECT rect;
    if (!fpGetWindowRect(hWnd, &rect))
    {
      return false;
    }

    RECT inter;
    if (IntersectRect(&inter, &hidden_rect, &rect))
    {
      lpRect->left= rect.left -15001; //one More is important !!!!!!
      lpRect->right= rect.right -15001;
      lpRect->top= rect.top;
      lpRect->bottom = rect.bottom;

    } else
    {
      lpRect->left= rect.left;
      lpRect->right= rect.right;
      lpRect->top= rect.top;
      lpRect->bottom = rect.bottom;
    }
  }
  catch (...)
  {
  }
  return true;
}

BOOL WINAPI DetourGetWindowInfo(__in HWND hwnd,  __inout  PWINDOWINFO pwi)
{
  BOOL retVal=false;

  try
  {
    retVal =fpGetWindowInfo(hwnd, pwi);
    if (retVal)
    {
      RECT inter;
      if (IntersectRect(&inter, &hidden_rect, &pwi->rcWindow))
      {
        pwi->rcWindow.left-=15001;
        pwi->rcWindow.right-=15001;
        pwi->rcClient.left-=15001;
        pwi->rcClient.right-=15001;
      }
    }
  }
  catch (...)
  {
  }
  return retVal;
}

BOOL WINAPI DetourGetWindowPlacement(__in HWND hwnd,  __inout  WINDOWPLACEMENT *lpwndpl)
{
  BOOL retVal=false;

  try
  {
    retVal=fpGetWindowPlacement(hwnd, lpwndpl);
    if (retVal)
    {
      RECT inter;
      if (IntersectRect(&inter, &hidden_rect, &lpwndpl->rcNormalPosition))
      {
        lpwndpl->rcNormalPosition.left-=15001;
        lpwndpl->rcNormalPosition.right-=15001;
      }
    }
  }
  catch (...)
  {
  }
  return retVal;
}

//BOOL WINAPI DetourSetForegroundWindow(__in HWND hwnd)
//{
//  try
//  {
//    trace("detour set foreground window\n");
//
//    //MessageBox(NULL, L"DetourSetForegroundWindow", L"kdfkd", MB_OK);
//
//    RECT rect;
//    if (fpGetWindowRect(hwnd, &rect))
//    {
//      RECT inter;
//      if (IntersectRect(&inter, &hidden_rect, &rect))
//      {
//        trace("set forgeound call but in hidden window -> skip");
//        return true;
//      } 
//    }
//
//    trace("normale forgrond call -> ok\n");
//    return fpSetForegroundWindow(hwnd);
//  }
//  catch (...)
//  {
//    MessageBox(NULL, L"Crash in DetourSetForegroundWindow", L"WindowsPager", MB_OK);
//    return false;
//  }
//}

void ResetSystemMenu(HMENU system_menu)
{
  RemoveMenu(system_menu, MENU_SEP1, MF_BYCOMMAND);
  RemoveMenu(system_menu, MENU_SEP2, MF_BYCOMMAND);

  int i;
  for (i = 1; i <= num_desktops; i++)
  {
    RemoveMenu(system_menu, MENU_ITEM + i, MF_BYCOMMAND);
  }

  RemoveMenu(system_menu,
    MENU_ITEM +  num_desktops+ 1, MF_BYCOMMAND);
  RemoveMenu(system_menu,
    MENU_ITEM +  num_desktops+ 2, MF_BYCOMMAND);
}


HMENU WINAPI DetourGetSystemMenu(__in HWND hWnd, __in BOOL bRevert)
{
  HMENU system_menu=NULL;

  try
  {
    trace("DetourGetSystemMenu: %x\n", hWnd);

    system_menu =fpGetSystemMenu(hWnd , bRevert);
    if (system_menu==NULL)
    {
      trace("DetourGetSystemMenu ist null %x\n", hWnd);
      return NULL;
    }

    HWND panel_window = FindWindow(L"Shell_TrayWnd", NULL);
    if (panel_window==0)
    {
      return system_menu;
    }

    HWND pagerWindow = FindWindowEx(panel_window, NULL, L"pagermain", NULL);
    if (pagerWindow==0)
    {
      return system_menu;
    }


    trace("rset system menu %x\n", hWnd);
    ResetSystemMenu(system_menu);

    int ontop=0;
    if (GetProp(hWnd, L"PAGER_TOPMOST"))
    {
      ontop=1;
    }

    int on_desk=0;
    on_desk=(int) GetProp(hWnd, L"PAGER_CURRENT_DESKTOP");

    trace("on deskt: %d\n", on_desk);

    int ignore=0;
    if (GetProp(hWnd, L"PAGER_IGNORE"))
    {
      ignore=1;
    }

    int monitor2=0;
    if (GetProp(hWnd, L"PAGER_2MONITOR"))
    {
      monitor2=1;
    }

    bool sticky=false;
    if (GetProp(hWnd, L"PAGER_STICKY"))
    {
      sticky=true;
    }


    AppendMenuA(system_menu, MF_SEPARATOR, static_cast<UINT_PTR>(MENU_SEP1), NULL);

    for (int i = 1; i <= num_desktops; i++)
    {
      char buf[256];
      sprintf_s(buf, 256, "Move to Desktop %d", i);
      AppendMenuA(system_menu,((i == (on_desk + 1)) || ignore || monitor2 || sticky)?MF_GRAYED:0|MF_STRING|MF_BYCOMMAND, 
        static_cast<UINT_PTR> (MENU_ITEM + i), buf);
    }

    AppendMenuA(system_menu,
      MF_BYCOMMAND | MF_STRING |(ignore|monitor2)?MF_GRAYED:0 |(sticky?MF_CHECKED:0),
      static_cast<UINT_PTR>(MENU_ITEM + num_desktops + 2), 
      "Always visible");

    AppendMenuA(system_menu, MF_SEPARATOR, static_cast<UINT_PTR>(MENU_SEP2), NULL);

    AppendMenuA(system_menu, (!ignore && ontop)?MF_CHECKED:0 | MF_BYCOMMAND | MF_STRING,
      static_cast<UINT_PTR>(MENU_ITEM + num_desktops + 1), 
      "Keep on top");
  }
  catch (...)
  {
    return NULL;
  }
  return system_menu;

}

//HWND WINAPI DetourSetActiveWindow(__in HWND hWnd)
//{
//  //MessageBox(NULL, L"SetatvieWidnow", L"kdjfkfj", MB_OK);
//
//  trace("detourset active widnow");
//
//   RECT rect;
//    if (fpGetWindowRect(hWnd, &rect))
//    {
//      RECT inter;
//      if (IntersectRect(&inter, &hidden_rect, &rect))
//      {
//        trace("SetActiveWindow call but in hidden window -> skip");
//
//        HWND activeWindow= GetActiveWindow();
//        return activeWindow;
//      } 
//    }
//
//    trace("SetActiveWindow call normal");
//   
//  return fpSetActiveWindow(hWnd);
//}

//BOOL WINAPI DetourBringWindowToTop(__in HWND hWnd)
//{
//  //MessageBox(NULL, L"DetourBringWindowToTop", L"kdjfkfj", MB_OK);
//
//   RECT rect;
//    if (fpGetWindowRect(hWnd, &rect))
//    {
//      RECT inter;
//      if (IntersectRect(&inter, &hidden_rect, &rect))
//      {
//        trace("Brintotop call but in hidden window -> skip");
//       return true;
//      } 
//    }
//
//
//  trace("detour bring window to top normal");
//  return fpBringWindowToTop(hWnd);
//}

//BOOL WINAPI DetourShowWindow(__in  HWND hWnd, __in  int nCmdShow)
//{
//  trace("detour show Window");
//
//
//  RECT rect;
//    if (fpGetWindowRect(hWnd, &rect))
//    {
//      RECT inter;
//      if (IntersectRect(&inter, &hidden_rect, &rect))
//      {
//        trace("DetourShowWindow skip");
//
//        if (nCmdShow == SW_SHOW || nCmdShow == SW_SHOWDEFAULT || nCmdShow == SW_SHOWNORMAL)
//        {
//          return ShowWindow(hWnd, SW_SHOWNOACTIVATE);
//        }
//     } 
//    }
//
//  return ShowWindow(hWnd, nCmdShow);
//  
//}
//
//
//BOOL WINAPI DetourSetWindowPos( __in      HWND hWnd,  __in_opt  HWND hWndInsertAfter, __in      int X,
//                                  __in      int Y,  __in      int cx,  __in      int cy,  __in      UINT uFlags)
//{
//  trace("DetourSetWindowPos");
//
//  //FIXME nicht alles skippen
//
//   return true;
//
//}


BOOL APIENTRY DllMain( HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
  switch (ul_reason_for_call)
  {
  case DLL_PROCESS_ATTACH:
    {
      try
      {
        trace("***api hook dll prozess attach***\n");

        HWND panel_window = FindWindow(L"Shell_TrayWnd", NULL);
        if (panel_window==0)
        {
          MessageBox(NULL, L"DllMain Lpanel_window==0", L"WindowsPager", MB_OK);
          return 1;
        }

        if (panel_window)
        {
          HWND pagerWindow = FindWindowEx(panel_window, NULL, L"pagermain", NULL);
          if (pagerWindow)
          {
            trace("sending attach mesage\n");
            PostMessage(pagerWindow, MSG_PROCESS_ATTACH, (WPARAM) GetCurrentProcessId(), 0);
          }
        }

        int virtual_left=(int) GetProp(panel_window, L"PAGER_HIDDEN_RECT_VIRTUAL_LEFT");
        int screen_width=(int) GetProp(panel_window, L"PAGER_HIDDEN_RECT_SCREEN_WIDTH");
        int screen_height=(int) GetProp(panel_window, L"PAGER_HIDDEN_RECT_SCREEN_HEIGHT");

        num_desktops=(int) GetProp(panel_window, L"PAGER_NUM_DESKTOPS");

        SetRect(&hidden_rect, virtual_left+15000, 0, virtual_left+15000+screen_width,
          screen_height);

        // trace("hidden rect in dll: %d, %d, %d, %d", hidden_rect.left, hidden_rect.right, hidden_rect.top, hidden_rect.bottom);

        if (MH_Initialize() != MH_OK)
        {
          //FIXME
          MessageBox(NULL, L"MH_Initialize() != MH_OK", L"WindowsPager", MB_OK);
          return 1;
        }

        //GetWindowRect
        if (MH_CreateHook(&GetWindowRect, &DetourGetWindowRect, reinterpret_cast<void**>(&fpGetWindowRect)) != MH_OK)
        {
          MessageBox(NULL, L"create hook failed", L"DetourGetWindowRect", MB_OK);
          return 1;
        }

        if (MH_EnableHook(&GetWindowRect) != MH_OK)
        {
          MessageBox(NULL, L"enable hook failed", L"kdfkd", MB_OK);
          return 1;
        }


        //GetWindowInfo
        if (MH_CreateHook(&GetWindowInfo, &DetourGetWindowInfo, reinterpret_cast<void**>(&fpGetWindowInfo)) != MH_OK)
        {
          MessageBox(NULL, L"create hook failed for GetWindowInfo", L"kdfkd", MB_OK);
          return 1;
        }

        if (MH_EnableHook(&GetWindowInfo) != MH_OK)
        {
          MessageBox(NULL, L"enable hook failed for GetWindowInfo", L"kdfkd", MB_OK);
          return 1;
        }


        //GetWindowPlacement
        if (MH_CreateHook(&GetWindowPlacement, &DetourGetWindowPlacement, reinterpret_cast<void**>(&fpGetWindowPlacement)) != MH_OK)
        {
          MessageBox(NULL, L"create hook failed for Windowplacment", L"kdfkd", MB_OK);
          return 1;
        }

        if (MH_EnableHook(&GetWindowPlacement) != MH_OK)
        {
          MessageBox(NULL, L"enable hook failed for window placent", L"kdfkd", MB_OK);
          return 1;
        }

        ////SetForegroundWindow
        //if (MH_CreateHook(&SetForegroundWindow, &DetourSetForegroundWindow, reinterpret_cast<void**>(&fpSetForegroundWindow)) != MH_OK)
        //{
        //  MessageBox(NULL, L"create hook failed for set foreground window", L"kdfkd", MB_OK);
        //  return 1;
        //}

        //if (MH_EnableHook(&SetForegroundWindow) != MH_OK)
        //{
        //  MessageBox(NULL, L"enable hook failed for set forgegournd", L"kdfkd", MB_OK);
        //  return 1;
        //}


        // //GetSystemMenu
        if (MH_CreateHook(&GetSystemMenu, &DetourGetSystemMenu, reinterpret_cast<void**>(&fpGetSystemMenu)) != MH_OK)
        {
          MessageBox(NULL, L"create hook failed for Getysemmenu", L"kdfkd", MB_OK);
          return 1;
        }

        if (MH_EnableHook(&GetSystemMenu) != MH_OK)
        {
          MessageBox(NULL, L"enable hook failed for  Getysemmenu", L"kdfkd", MB_OK);
          return 1;
        }


        ////SetActiveWindow
        //if (MH_CreateHook(&SetActiveWindow, &DetourSetActiveWindow, reinterpret_cast<void**>(&fpSetActiveWindow)) != MH_OK)
        //{
        //  MessageBox(NULL, L"create hook failed for SetActiveWindow", L"kdfkd", MB_OK);
        //  return 1;
        //}

        //if (MH_EnableHook(&SetActiveWindow) != MH_OK)
        //{
        //  MessageBox(NULL, L"enable hook failed for  SetActiveWindow", L"kdfkd", MB_OK);
        //  return 1;
        //}
        ////BringWindowToTop
        //if (MH_CreateHook(&BringWindowToTop, &DetourBringWindowToTop, reinterpret_cast<void**>(&fpBringWindowToTop)) != MH_OK)
        //{
        //  MessageBox(NULL, L"create hook failed for BringWindowToTop", L"kdfkd", MB_OK);
        //  return 1;
        //}

        /* if (MH_EnableHook(&BringWindowToTop) != MH_OK)
        {
        MessageBox(NULL, L"enable hook failed for  BringWindowToTop", L"kdfkd", MB_OK);
        return 1;
        }*/

        //   //ShowWindow
        //if (MH_CreateHook(&ShowWindow, &DetourShowWindow, reinterpret_cast<void**>(&fpShowWindow)) != MH_OK)
        //{
        //  MessageBox(NULL, L"create hook failed for ShowWindow", L"kdfkd", MB_OK);
        //  return 1;
        //}

        //if (MH_EnableHook(&ShowWindow) != MH_OK)
        //{
        //  MessageBox(NULL, L"enable hook failed for  ShowWindow", L"kdfkd", MB_OK);
        //  return 1;
        //}


        //   //SetWindowPos
        //if (MH_CreateHook(&SetWindowPos, &DetourSetWindowPos, reinterpret_cast<void**>(&fpSetWindowPos)) != MH_OK)
        //{
        //  MessageBox(NULL, L"create hook failed for SetWindowPos", L"kdfkd", MB_OK);
        //  return 1;
        //}

        //if (MH_EnableHook(&SetWindowPos) != MH_OK)
        //{
        //  MessageBox(NULL, L"enable hook failed for  SetWindowPos", L"kdfkd", MB_OK);
        //  return 1;
        //}

      } catch(...)
      {
        return false;
      }
    }

    break;
  case DLL_THREAD_ATTACH:
    break;

  case DLL_THREAD_DETACH:
    break;

  case DLL_PROCESS_DETACH:
    {
      //Achtung hooks nicht disablen!

      HWND panel_window = FindWindow(L"Shell_TrayWnd", NULL);
      if (panel_window)
      {
        HWND pagerWindow = FindWindowEx(panel_window, NULL, L"pagermain", NULL);
        if (pagerWindow)
        {
          trace("hook_starter64: %x\n", pagerWindow);
          PostMessage(pagerWindow, MSG_PROCESS_DETACH, (WPARAM) GetCurrentProcessId(), 0);
        }
      }
    }
    break;
  }
  return TRUE;
}

