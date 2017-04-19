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

#include "..\common\stdafx.h"
#include <commctrl.h>

#include "XWinVer.h"
#include "System.h"
#include "Desktop.h"
#include "Window.h"
#include "Pager.h"
#include "Ini.h"
#include "WindowSelector.h"

using namespace std;
using namespace boost;
using namespace Gdiplus;

#define BUTTON_SIZE 64
#define PIE 14
#define MOVE_BOX 40
#define SPACE 20
#define BAR_WIDTH (4*BUTTON_SIZE+2*SPACE+20)
#define BAR_HEIGHT (BUTTON_SIZE+PIE+5)

void WindowSelector::AddToInIWin(void)
{
  LogMethod;

  vector<HWND> zorder;
  zorder.clear();

  trace("add to ini win\n");

  std::vector<std::pair<HWND, RECT>>::iterator iter = windows.begin();
  while (iter != windows.end())
  {
    RECT rect= (*iter).second;
    if (RectInRect(selection, rect))
    {
      zorder.push_back((*iter).first);
    }
    ++iter;
  }

  if (!zorder.empty())
  {
    did_something=true;
  } else
  {
    MessageBox(main_win, "No window(s) selected!", "WindowsPager: Window-Selector", MB_OK);
    return;
  }

  int last_value=GetSys->GetIniFile()->GetIgnoreWindowsCount();
  if (last_value==-1)
  {
    MessageBoxW(main_win, L"The section 'IgnoreWindows' in the 'windowspager.ini' file seems to be broken.\nFix it and restart WindowsPager.", L"WindowsPager: Window-Selector", MB_OK);
    return;
  }

  wstring message_text(L"Successfully added the following entries to the 'windowspager.ini':\n\n   ");

  set<wstring> added_stuff;
  added_stuff.clear();

  BOOST_FOREACH(HWND win, zorder)
  {
    WCHAR wndClassName[256];
    if (!GetClassNameW(win, wndClassName, 256))
    {
      continue;
    }

    shared_ptr<wstring> filename(GetSys->GetWindowFileName(win));
    if (!filename.get())
    {
      continue;
    }

    int last_backslash=filename->find_last_of('\\');
    if (last_backslash==wstring::npos)
    {
      continue;
    }

    wstring exe_name=filename->substr(last_backslash+1);
    to_lower(exe_name);
    
    wstring value=exe_name+wstring(L":")+wndClassName;

    if (added_stuff.count(value) ==1)
    {
      tracew(L"found exe %s, added_stuff-> skip\n", value);
      continue;
    }
    
    last_value++;
    if (GetSys->GetIniFile()->AddToIgnoreWindowList(last_value, value))
    {
      message_text=message_text+L"- "+exe_name+L" (class: "+wndClassName+L")\n   ";
      tracew(L"value added: <%s>\n", value.c_str());
      added_stuff.insert(value);
    }
  }

  message_text+=L"\n\n(Duplicated values are removed.)";

  MessageBoxW(main_win, message_text.c_str(), L"WindowsPager: Window-Selector", MB_OK);
  zorder.clear();
  added_stuff.clear();
}

void WindowSelector::AddToInIProcess(void)
{
  LogMethod;

  vector<HWND> zorder;
  zorder.clear();

  trace("add to ini process\n");

  std::vector<std::pair<HWND, RECT>>::iterator iter = windows.begin();
  while (iter != windows.end())
  {
    RECT rect= (*iter).second;
    if (RectInRect(selection, rect))
    {
      zorder.push_back((*iter).first);
    }
    ++iter;
  }

  if (!zorder.empty())
  {
    did_something=true;
  } else
  {
    MessageBox(main_win, "No window(s) selected!", "WindowsPager: Window-Selector", MB_OK);
    return;
  }

  int last_value=GetSys->GetIniFile()->GetIgnoreWindowsCount();
  if (last_value==-1)
  {
    MessageBoxW(main_win, L"The section 'IgnoreWindows' in the 'windowspager.ini' file seems to be broken.\nFix it and restart WindowsPager.", L"WindowsPager: Window-Selector", MB_OK);
    return;
  }

  wstring message_text(L"Successfully added the following Application(s) to the 'windowspager.ini':\n\n   ");

  set<wstring> added_stuff;
  added_stuff.clear();

  BOOST_FOREACH(HWND win, zorder)
  {
    shared_ptr<wstring> filename(GetSys->GetWindowFileName(win));
    if (!filename.get())
    {
      continue;
    }
    int last_backslash=filename->find_last_of('\\');
    if (last_backslash==wstring::npos)
    {
      continue;
    }

    wstring exe_name=filename->substr(last_backslash+1);
    to_lower(exe_name);

    if (added_stuff.count(exe_name) ==1)
    {
      tracew(L"found exe %s, added_stuff-> skip\n", exe_name);
      continue;
    }
    
    wstring value=exe_name+wstring(L":*");
    
    last_value++;
    if (GetSys->GetIniFile()->AddToIgnoreWindowList(last_value, value))
    {
      message_text=message_text+L"- "+exe_name+L"\n   ";
      tracew(L"value added: <%s>\n", value.c_str());
      added_stuff.insert(exe_name);
    }
  }

  MessageBoxW(main_win, message_text.c_str(), L"WindowsPager: Window-Selector", MB_OK);
  zorder.clear();
  added_stuff.clear();
}

bool WindowSelector::RectInRect(const RECT& big_rect, const RECT& small_rect)
{
  LogMethod;

  POINT upper_left; 
  upper_left.x=small_rect.left;
  upper_left.y=small_rect.top;

  POINT bottom_right; 
  bottom_right.x=small_rect.right;
  bottom_right.y=small_rect.bottom;

  if (PtInRect(&big_rect, upper_left) &&  PtInRect(&big_rect, bottom_right))
  {
    return true;
  } else
  {
    return false;
  }
}

void WindowSelector::GetAllWindows(void)
{
  LogMethod;

  windows.clear();
  HWND hwnd = GetTopWindow(NULL);
  if (!hwnd)
  {
    return;
  }

  while (hwnd)
  {
    if (!IsWindowVisible(hwnd))
    {
      hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
      continue;
    }

    RECT win_rect;
    if (!GetWindowRect(hwnd, &win_rect))
    {
      hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
      continue;
    }

    if (GetProp(hwnd, "PAGER_HIDDEN"))
    {
      trace("found window is hidden .> skip\n");
      hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
    }

    LONG_PTR normal_style = GetSys->GetWindowLongPTR(hwnd, GWL_STYLE);
    if (normal_style==-1)
    {
      hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
      continue;
    }

    WCHAR wndClassName[256];
    if (!GetClassNameW(hwnd, wndClassName, 256))
    {
      hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
      continue;
    }
   
    if (lstrcmpiW(L"ConsoleWindowClass", wndClassName) == 0)
    {
      hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
      continue;
    }

    windows.push_back(std::pair<HWND, RECT>(hwnd, win_rect));
    hwnd = GetNextWindow(hwnd, GW_HWNDNEXT);
  }
}

WindowSelector::WindowSelector()
{
  LogMethod;

  char szName[] = "WindowsPagerWindowSelector";

  add_pressed_proc=false;
  add_pressed_win=false;
  help_pressed=false;
  exit_pressed=false;
  did_something=false;
  moving=false;
  resizing1=false;
  resizing2=false;
  in_move_box=false;
  in_resizing1_box=false;
  in_resizing2_box=false;

#ifdef _DEBUG
  add_button_win= new Image(L"../../windowspager/lib/selector_add_win.png");
  add_button_proc= new Image(L"../../windowspager/lib/selector_add_proc.png");
  exit_button= new Image(L"../../windowspager/lib/selector_exit.png");
  help_button= new Image(L"../../windowspager/lib/selector_help.png");
#else
  add_button_win= new Image(L"lib/selector_add_win.png");
  add_button_proc= new Image(L"lib/selector_add_proc.png");
  exit_button= new Image(L"lib/selector_exit.png");
  help_button= new Image(L"lib/selector_help.png");
#endif
  
  bmp = new Bitmap(GetSys->GetPrimaryWidth(), GetSys->GetPrimaryHeight());

  hand_cursor= LoadCursor(NULL, IDC_SIZEALL); 

  wc.style         = CS_HREDRAW | CS_VREDRAW; 
  wc.lpfnWndProc   = WindowSelector::WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hI;
  wc.hIcon         = LoadIcon (NULL, IDI_WINLOGO);
  wc.hCursor       = LoadCursor (NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(1, 2, 3));
  wc.lpszMenuName  = NULL;
  wc.lpszClassName = szName;
  RegisterClass (&wc);

  HWND hwnd =
    CreateWindowEx(WS_EX_LAYERED|WS_EX_TOOLWINDOW|WS_EX_TOPMOST, szName, 
    "WindowsPager: Window Selector",
    WS_POPUP, 0, 0, GetSys->GetPrimaryWidth(), GetSys->GetPrimaryHeight(), NULL, NULL, hI, NULL);

  if (GetWinVersion->IsSeven() || GetWinVersion->IsVista())
  {
    SetLayeredWindowAttributes(hwnd , RGB(1, 2, 3) , 200, LWA_COLORKEY | LWA_ALPHA);
  } else
  {
    SetLayeredWindowAttributes(hwnd , RGB(1, 2, 3) , 150, LWA_COLORKEY);
  }

  int start_width=GetSys->GetPrimaryWidth()/3;
  int start_height=GetSys->GetPrimaryHeight()/2;
  int start_x=GetSys->GetPrimaryWidth()/2-start_width/2;
  int start_y=GetSys->GetPrimaryHeight()/2-start_height/2;

  SetRect(&selection, start_x, start_y, start_x+start_width, start_y+start_height);
  WindowSelector::GetAllWindows();

  main_win=hwnd;

#pragma warning( disable : 4244)
  SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast <LONG_PTR> (this));
#pragma warning(default:4244)


  ShowWindow   (hwnd, SW_NORMAL);
  UpdateWindow (hwnd);
}

WindowSelector::~WindowSelector()
{
  LogMethod;

  delete add_button_proc;
  delete add_button_win;
  delete exit_button;
  delete help_button;
  delete bmp;
  
  DestroyCursor(hand_cursor);

  UnregisterClass("WindowsPagerWindowSelector", hI);
  windows.clear();
  DestroyWindow(main_win);

  if (did_something)
  {
    PostMessage(GetPager->GetPagerWindowHWND(), GetPager->GetHookMessage(), 
      MSG_WINDOW_SELECTOR, 1);
  }
}

void WindowSelector::InvalidateTheShit(HWND hwnd, RECT rect)
{
  LogMethod;

  RECT out(rect);

  out.left-=6;
  out.top-=6;
  out.right+=6;
  out.bottom+=6;
  InvalidateRect(hwnd, &out, false);

  RECT bar;
  SetRect(&bar, GetSys->GetPrimaryWidth()/2-BAR_WIDTH/2-PIE, 
    GetSys->GetPrimaryHeight()-BAR_HEIGHT-PIE,
    GetSys->GetPrimaryWidth()/2+BAR_WIDTH/2+PIE, GetSys->GetPrimaryHeight());
  ValidateRect(hwnd, &bar);
}

void WindowSelector::onPaint(HWND hwnd, Graphics& graphics)
{
  LogMethod;

  if (GetWinVersion->IsSeven() || GetWinVersion->IsVista())
  {
    SolidBrush blackBrush(Color(255, 0, 0, 0));
    graphics.FillRectangle(&blackBrush, 0, 0, GetSys->GetPrimaryWidth(), 
      GetSys->GetPrimaryHeight());

    SolidBrush whiteBrush(Color(255, 1, 2, 3));
    graphics.FillRectangle(&whiteBrush, selection.left, selection.top,
      selection.right-selection.left, selection.bottom-selection.top);
  } else
  {
    SolidBrush blackBrush(Color(255, 1, 2, 3));
      graphics.FillRectangle(&blackBrush, 0, 0, 1680, 1050 );
  }
  

  Pen pen_frame(Color(255, 0, 0, 255), 6);
  int selection_w=selection.right-selection.left;
  int selection_h=selection.bottom-selection.top;
  graphics.DrawRectangle(&pen_frame, selection.left, selection.top, 
    selection.right-selection.left, 
    selection.bottom-selection.top);

  Pen pen_red_small(Color(255, 255, 0, 0), 1);
  Pen pen_red_big(Color(255, 255, 0, 0), 4);

  std::vector<std::pair<HWND, RECT>>::iterator iter = windows.begin();
  while (iter != windows.end())
  {
    RECT rect= (*iter).second;
    if (RectInRect(selection, rect))
    {
      graphics.DrawRectangle(&pen_red_big, rect.left, rect.top,
        rect.right-rect.left, rect.bottom-rect.top);
    }
    ++iter;
  }

  SolidBrush greenBrush(Color(255, 0, 255, 0));

  graphics.FillRectangle(&greenBrush, selection.left+selection_w/2-MOVE_BOX/2 ,
    selection.top+selection_h/2-MOVE_BOX/2, MOVE_BOX, MOVE_BOX);

  graphics.FillRectangle(&greenBrush, selection.left-5,
    selection.top-5, 10, 10);

  graphics.FillRectangle(&greenBrush, selection.right-5,
    selection.bottom-5, 10, 10);

  SolidBrush behindButtonBrush(Color(255, 255, 213, 0));
  graphics.FillRectangle(&behindButtonBrush, GetSys->GetPrimaryWidth()/2-BAR_WIDTH/2,
    GetSys->GetPrimaryHeight()-BAR_HEIGHT-PIE, BAR_WIDTH, BAR_HEIGHT+PIE);

  graphics.FillPie(&behindButtonBrush, GetSys->GetPrimaryWidth()/2-BAR_WIDTH/2-PIE, 
    GetSys->GetPrimaryHeight()-BAR_HEIGHT-PIE, PIE*2, PIE*2, 180.0f, 90.0f);

  graphics.FillPie(&behindButtonBrush, GetSys->GetPrimaryWidth()/2+BAR_WIDTH/2-PIE, 
    GetSys->GetPrimaryHeight()-BAR_HEIGHT-PIE, PIE*2, PIE*2, 270.0f, 90.0f);

  graphics.FillRectangle(&behindButtonBrush, GetSys->GetPrimaryWidth()/2-BAR_WIDTH/2-PIE,
    GetSys->GetPrimaryHeight()-BAR_HEIGHT, PIE, BAR_HEIGHT);

  graphics.FillRectangle(&behindButtonBrush, GetSys->GetPrimaryWidth()/2+BAR_WIDTH/2,
    GetSys->GetPrimaryHeight()-BAR_HEIGHT, PIE, BAR_HEIGHT);

  if (!add_pressed_proc)
  {
    graphics.DrawImage(add_button_proc, GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE-SPACE-BUTTON_SIZE,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,BUTTON_SIZE, BUTTON_SIZE );
  } else
  {
    graphics.DrawImage(add_button_proc, GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE-SPACE-BUTTON_SIZE+2,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+2,BUTTON_SIZE, BUTTON_SIZE );
  }

  if (!add_pressed_win)
  {
    graphics.DrawImage(add_button_win, GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,BUTTON_SIZE, BUTTON_SIZE );
  } else
  {
    graphics.DrawImage(add_button_win, GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE+2,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+2,BUTTON_SIZE, BUTTON_SIZE );
  }

  if (!exit_pressed)
  {
    graphics.DrawImage(exit_button, GetSys->GetPrimaryWidth()/2+SPACE, 
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5, BUTTON_SIZE, BUTTON_SIZE);
  } else
  {
    graphics.DrawImage(exit_button, GetSys->GetPrimaryWidth()/2+SPACE+2, 
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+2, BUTTON_SIZE, BUTTON_SIZE);
  }

  if (!help_pressed)
  {
    graphics.DrawImage(help_button, GetSys->GetPrimaryWidth()/2+SPACE+BUTTON_SIZE+SPACE,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,BUTTON_SIZE, BUTTON_SIZE);
  } else
  {
    graphics.DrawImage(help_button, GetSys->GetPrimaryWidth()/2+SPACE+BUTTON_SIZE+SPACE+2,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+2,BUTTON_SIZE, BUTTON_SIZE);
  }

}

int WindowSelector::LButtonDown(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LogMethod;

  POINT pt;
  pt.x = (LONG) LOWORD(lParam);
  pt.y = (LONG) HIWORD(lParam);

  int selection_w=selection.right-selection.left;
  int selection_h=selection.bottom-selection.top;

  RECT move_button; 
  SetRect(&move_button,  selection.left+selection_w/2-MOVE_BOX/2,
    selection.top+selection_h/2-MOVE_BOX/2, 
    selection.left+selection_w/2+MOVE_BOX/2,
    selection.top+selection_h/2+MOVE_BOX/2);
  if (PtInRect(&move_button, pt))
  {
    trace("move button clicked\n");
    move_startx=pt.x;
    move_starty=pt.y;
    CopyRect(&move_start_rect, &selection);
    RECT clip_cursor; 
    SetRect(&clip_cursor, MOVE_BOX, MOVE_BOX, GetSys->GetPrimaryWidth()-MOVE_BOX, 
      GetSys->GetPrimaryHeight()-MOVE_BOX);
    ClipCursor(&clip_cursor);
    SetCapture(hwnd);
    moving=true;
  }

  RECT resize1_button; 
  SetRect(&resize1_button,  selection.left-5 ,
    selection.top-5, 
    selection.left+5,
    selection.top+5);
  if (PtInRect(&resize1_button, pt))
  {
    trace("resize1_button clicked\n");
    RECT clip_cursor; 
    SetRect(&clip_cursor, 5, 5,          
      selection.right-MOVE_BOX-5-5, 
      selection.bottom-MOVE_BOX-5-5);
    ClipCursor(&clip_cursor);
    SetCapture(hwnd);
    resizing1=true;
  }

  RECT resize2_button; 
  SetRect(&resize2_button,  selection.right-5,
    selection.bottom-5, 
    selection.right+5,
    selection.bottom+5);
  if (PtInRect(&resize2_button, pt))
  {
    trace("resize2_button clicked\n");
    RECT clip_cursor;
    SetRect(&clip_cursor, selection.left+MOVE_BOX+5+5+10, 
      selection.top+MOVE_BOX+5+5+10,
      GetSys->GetPrimaryWidth()-5, GetSys->GetPrimaryHeight()-5);
    ClipCursor(&clip_cursor);
    SetCapture(hwnd);
    resizing2=true;
  }

  RECT add_button_proc; 
  SetRect(&add_button_proc,
    GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE-SPACE-BUTTON_SIZE,
    GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,
    GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE-SPACE-BUTTON_SIZE+BUTTON_SIZE,
    GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+BUTTON_SIZE);
  if (PtInRect(&add_button_proc, pt))
  {
    trace("ok button clicked\n");
    add_pressed_proc=true;
    InvalidateRect(hwnd, NULL, false);
    UpdateWindow (hwnd);
    return 0;
  }

  RECT add_button_win; 
  SetRect(&add_button_win,
    GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE,
    GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,
    GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE+BUTTON_SIZE,
    GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+BUTTON_SIZE);
  if (PtInRect(&add_button_win, pt))
  {
    trace("ok button clicked\n");
    add_pressed_win=true;
    InvalidateRect(hwnd, NULL, false);
    UpdateWindow (hwnd);
    return 0;
  }

  RECT exit_button; 
  SetRect(&exit_button,
    GetSys->GetPrimaryWidth()/2+SPACE, 
    GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,
    GetSys->GetPrimaryWidth()/2+SPACE+BUTTON_SIZE, 
    GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+BUTTON_SIZE);
  if (PtInRect(&exit_button, pt))
  {
    trace("exit button clicked\n");
    exit_pressed=true;
    InvalidateRect(hwnd, NULL, false);
    UpdateWindow (hwnd);
    return 0;
  }

  RECT help_button; 
  SetRect(&help_button,
    GetSys->GetPrimaryWidth()/2+SPACE+BUTTON_SIZE+SPACE, 
    GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,
    GetSys->GetPrimaryWidth()/2+SPACE+BUTTON_SIZE+SPACE+BUTTON_SIZE, 
    GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+BUTTON_SIZE);
  if (PtInRect(&help_button, pt))
  {
    trace("help button clicked\n");
    help_pressed=true;
    InvalidateRect(hwnd, NULL, false);
    UpdateWindow (hwnd);
    return 0;
  }

  return 0;
}

int WindowSelector::LButtonUp(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LogMethod;

  POINT pt;
  pt.x = (LONG) LOWORD(lParam);
  pt.y = (LONG) HIWORD(lParam);

  ClipCursor(NULL);
  ReleaseCapture();
  moving=false;
  resizing1=false;
  resizing2=false;

  if (add_pressed_proc)
  {
    RECT add_button_proc; 
    SetRect(&add_button_proc,
      GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE-SPACE-BUTTON_SIZE,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,
      GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE-SPACE-BUTTON_SIZE+BUTTON_SIZE,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+BUTTON_SIZE);

    if (PtInRect(&add_button_proc, pt))
    {
      trace("add proc up\n");
      AddToInIProcess();
    }
    add_pressed_proc=false;
  } else if (add_pressed_win)
  {
    RECT add_button_win; 
    SetRect(&add_button_win,
      GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,
      GetSys->GetPrimaryWidth()/2-SPACE-BUTTON_SIZE+BUTTON_SIZE,
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+BUTTON_SIZE);

    if (PtInRect(&add_button_win, pt))
    {
      trace("add win up\n");
      AddToInIWin();
    }
    add_pressed_win=false;
  } else if (exit_pressed)
  {
    RECT exit_button; 
    SetRect(&exit_button,
      GetSys->GetPrimaryWidth()/2+SPACE, 
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,
      GetSys->GetPrimaryWidth()/2+SPACE+BUTTON_SIZE, 
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+BUTTON_SIZE);

    if (PtInRect(&exit_button, pt))
    {
      trace("exit button clicked\n");
      delete this;
      return 0;
    }
    exit_pressed=false;
  } else if (help_pressed)
  {
    RECT help_button; 
    SetRect(&help_button,
      GetSys->GetPrimaryWidth()/2+SPACE+BUTTON_SIZE+SPACE, 
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5,
     GetSys->GetPrimaryWidth()/2+SPACE+BUTTON_SIZE+SPACE+BUTTON_SIZE, 
      GetSys->GetPrimaryHeight()-BAR_HEIGHT+5+BUTTON_SIZE);

    if (PtInRect(&help_button, pt))
    {
      trace("help button clicked\n");
      MessageBoxW(main_win, L"To add an application or window to the ignore list follow the following steps.\n"\
                            L"To avoid problems you should not change the desktop before.\n\n"\
                            L"1. Select the window(s) WindowsPager should ignore with the blue rectangle.\n"\
                            L"You can move and resize it with the green rectangles.\n"\
                            L"If a window is selected it will be framed with a red rectangle.\n\n"\
                     L"2a. If WindowsPager should ignore all windows from this application click on the green "\
                     L"'plus'-button to add the selected application to the 'windowspager.ini'.\nThis is recommended.\n\n"\
                            L"2b. If WindowsPager should only ignore the selected window(s) click on the "\
                            L"black 'plus'-button.\n\n\n"\
                            L"3. Exit the Windows-Selector with the red 'exit'-button." ,
                            L"WindowsPager: Window-Selector", MB_OK);
    }
    help_pressed=false;
  }

  InvalidateRect(hwnd, NULL, false);
  UpdateWindow (hwnd);
  return 0;
}

int WindowSelector::MouseMove(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LogMethod;

  POINT pt;
  pt.x = (LONG) LOWORD(lParam);
  pt.y = (LONG) HIWORD(lParam);
  RECT old_selection(selection);

  int selection_w=selection.right-selection.left;
  int selection_h=selection.bottom-selection.top;
  RECT move_button; 

  SetRect(&move_button,  selection.left+selection_w/2-MOVE_BOX/2,
    selection.top+selection_h/2-MOVE_BOX/2, 
    selection.left+selection_w/2+MOVE_BOX/2,
    selection.top+selection_h/2+MOVE_BOX/2);

  if (PtInRect(&move_button, pt))
  {
    in_move_box=true;
  } else
  {
    in_move_box=false;
  }

  RECT resize1_button; 
  SetRect(&resize1_button,  selection.left-5 ,
    selection.top-5, 
    selection.left+5,
    selection.top+5);

  if (PtInRect(&resize1_button, pt))
  {
    in_resizing1_box=true;
  } else
  {
    in_resizing1_box=false;
  }

  RECT resize2_button; 
  SetRect(&resize2_button,  selection.right-5,
    selection.bottom-5, 
    selection.right+5,
    selection.bottom+5);

  if (PtInRect(&resize2_button, pt))
  {
    in_resizing2_box=true;
  } else
  {
    in_resizing2_box=false;
  }

  if (moving)
  {
    int diff_x=pt.x-move_startx;
    int diff_y=pt.y-move_starty;
    selection.left=move_start_rect.left+diff_x;
    selection.top=move_start_rect.top+diff_y;
    selection.right=move_start_rect.right+diff_x;
    selection.bottom=move_start_rect.bottom+diff_y;

    if (GetWinVersion->IsSeven() || GetWinVersion->IsVista())
    {
      InvalidateRect(hwnd, NULL, false);
    } else
    {
      InvalidateTheShit(hwnd, old_selection);
      InvalidateTheShit(hwnd, selection);
    }

    UpdateWindow (hwnd);
    return 0;
  } else  if (resizing1)
  {
    selection.left=pt.x;
    selection.top=pt.y;
    
    if (GetWinVersion->IsSeven() || GetWinVersion->IsVista())
    {
      InvalidateRect(hwnd, NULL, false);
    } else
    {
      InvalidateTheShit(hwnd, old_selection);
      InvalidateTheShit(hwnd, selection);
    }
    UpdateWindow (hwnd);
    return 0;
  } else if (resizing2)
  {
    selection.right=pt.x;
    selection.bottom=pt.y;
    
    if (GetWinVersion->IsSeven() || GetWinVersion->IsVista())
    {
      InvalidateRect(hwnd, NULL, false);
    } else
    {
      InvalidateTheShit(hwnd, old_selection);
      InvalidateTheShit(hwnd, selection);
    }
    UpdateWindow (hwnd);
    return 0;
  }
  return 0;
}

LRESULT WindowSelector::WindowCalls (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  LogMethod;

  switch (message)
  {
  case WM_CLOSE:
  case WM_SYSKEYDOWN:
  case WM_KEYDOWN:
    {
      ClipCursor(NULL);
      ReleaseCapture();
      delete this;
      return 0;
    }
    break;

  case WM_LBUTTONDOWN:
    {
      return WindowSelector::LButtonDown(hwnd, message, wParam, lParam);
    }
    break;

  case WM_MOUSEMOVE:
    {
      return WindowSelector::MouseMove(hwnd, message, wParam, lParam);

    }
    break;

  case WM_CANCELMODE:
  case WM_LBUTTONUP:
    {
      return WindowSelector::LButtonUp(hwnd, message, wParam, lParam);
    }
    return 0;

  case WM_PAINT:
    {
      HDC hdc;
      PAINTSTRUCT ps;
      hdc = BeginPaint(hwnd, &ps);
      Graphics graphics(hdc);

      if (GetWinVersion->IsSeven() || GetWinVersion->IsVista())
      {
        Graphics* buffer_graph = Graphics::FromImage(bmp);
        onPaint(hwnd, *buffer_graph);
        graphics.DrawImage(bmp, 0,0);
        delete buffer_graph;
      } else
      {
         onPaint(hwnd, graphics);
      }

      EndPaint(hwnd, &ps);
    }
    return 0;

  case WM_SETCURSOR: 
    {
      if (in_move_box || in_resizing1_box || in_resizing2_box || moving || resizing1 ||
        resizing2)
      {
        SetCursor(hand_cursor);
        return TRUE;
      }
    }
    break;

  }
  return DefWindowProc (hwnd, message, wParam, lParam);
} 

LRESULT CALLBACK WindowSelector::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  LogMethod;

  WindowSelector *pThis = reinterpret_cast<WindowSelector*>(GetWindowLongPtr(hWnd,
    GWLP_USERDATA));
  if (pThis!=NULL)
  {
    return pThis->WindowCalls(hWnd, uMessage, wParam, lParam);
  } else
  {
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
  }
}