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

#include "HangUpInfoWindow.h"

#include "System.h"
#include "XWinVer.h"
#include "Pager.h"

using namespace std;


HangUpInfoWindow::HangUpInfoWindow(HWND bad_win, HICON &smallicon, bool duringhide)
{
  LogMethod;

  _seconds=0;
  _bad_win=bad_win;
  _isdeath=false;
  visible=false;
  _duringhide=duringhide;
  unicode=IsWindowUnicode(_bad_win);
  title=NULL;
  title_w=NULL;

  _icon=CopyIcon(smallicon);
 
  if (unicode)
  {
    trace("bad win is unicode\n");

    wstring title_tmp_w;

    wchar_t old[256];
    if (!GetWindowTextW(_bad_win, old, 256))
    {
      title_tmp_w =L"(no title)";
    } else
    {
      title_tmp_w  =old;
    }

    if (title_tmp_w.length() >= 20)
    {
      title_w=  new wstring (title_tmp_w.substr(0, 20)+L"...");
    } else
    {
      title_w= new wstring(title_tmp_w);
    }

  } else
  {
    string title_tmp;

    char old[256];
    if (!GetWindowTextA(_bad_win, old, 256))
    {
      title_tmp ="(no title)";
    } else
    {
      title_tmp  =old;
    }

    if (title_tmp.length() >= 20)
    {
      title=  new string (title_tmp.substr(0, 20)+"...");
    } else
    {
      title= new string(title_tmp);
    }

  }

  _close_icon=LoadIcon (NULL, IDI_ERROR);
  SetRect(&_close_rect, 285, 2, 285+20, 2+20);

  m_hFont =
    CreateFont(-12, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
    OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
    DEFAULT_PITCH, "Tahoma");

  trace("create dummy window\n");
 
  m_hWnd =
    CreateWindowEx(WS_EX_TOOLWINDOW|WS_EX_TOPMOST, "PagerHangUpWindow", "",
    WS_POPUP|WS_BORDER  ,
    0,
    0, dummy_width, dummy_height, NULL, NULL, hI, NULL);

#pragma warning( disable : 4244)
  SetWindowLongPtr(m_hWnd, GWLP_USERDATA, reinterpret_cast <LONG_PTR> (this));
#pragma warning(default:4244)
}

HangUpInfoWindow::~HangUpInfoWindow()
{
  LogMethod;

  trace("~dummywindow");

  if (unicode)
  {
    delete title_w;
  } else
  {
    delete title;
  }
  
  DestroyIcon(_icon);
  DestroyIcon(_close_icon);
  DeleteObject(m_hFont);
  DestroyWindow(m_hWnd);
}

LRESULT CALLBACK HangUpInfoWindow::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam,
                                      LPARAM lParam)
{
  LogMethod;

  HangUpInfoWindow *pThis = reinterpret_cast<HangUpInfoWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
  if (pThis!=NULL)
  {
    return pThis->WindowProc(hWnd, uMessage, wParam, lParam);
  } else
  {
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
  }
}

void HangUpInfoWindow::DieSlowly(void)
{
  LogMethod;

  trace("start slow die timer\n");
  _isdeath=true;

  if (visible)
  {
    InvalidateRect(m_hWnd, NULL, true);
    UpdateWindow(m_hWnd);
    SetTimer(m_hWnd, TIMER_ID_DIE_SLOWLY, 2000, NULL);
  } else
  {
    delete this;
  }
}

void HangUpInfoWindow::Show(void)
{
  LogMethod;

  int screen_width= GetSys->GetPrimaryWidth();

  bool free_place=false;
  POINT p;
  p.x=screen_width-2;
  p.y=1;

  while (free_place==false)
  {
    p.x-=(dummy_width+2);
    if (p.x < 0)
    {
      p.x=screen_width- dummy_width-2;
      break;
    }

    HWND win =WindowFromPoint(p);
    if (!win)
    {
      trace("found free place on: %d\n", p.x);
      break;
    } else
    {
      char wndClassName[256];
      if (GetClassName(win, wndClassName, 256))
      {
        trace("clasenam on point: %d is: %s\n", p.x, wndClassName);
        if (strcmp(wndClassName, "PagerHangUpWindow") != 0)
        {
          trace("found free place on: %d\n", p.x);
          break;
        }
      }
    }
  }

  SetWindowPos( m_hWnd, HWND_TOPMOST,  p.x, p.y, 0, 0, SWP_NOSIZE|
    SWP_NOACTIVATE);
  AnimateWindow(m_hWnd, 300, AW_BLEND);

  visible=true;
}

void HangUpInfoWindow::Update(void)
{
  LogMethod;

  _seconds++;
  trace("seconds: %d\n", _seconds);

  if (visible)
  {
    InvalidateRect(m_hWnd,NULL, true);
    UpdateWindow(m_hWnd);
  }
}

LRESULT HangUpInfoWindow::WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam,
                                LPARAM lParam)
{
  LogMethod;

  switch (uMessage)
  {
  case WM_CLOSE:
    {
      trace("wm close\n");
    }
    return 0;

  case WM_PAINT:
    HangUpInfoWindow::OnPaint();
    return 0;

  case WM_TIMER:
    {
      if (wParam == TIMER_ID_DIE_SLOWLY)
      {
        KillTimer(m_hWnd, TIMER_ID_DIE_SLOWLY);
        trace("timer die slowly over\n");
        AnimateWindow(m_hWnd, 200, AW_BLEND|AW_HIDE);
        delete this;
        return 0;
      }  
    }
    return 0;

  case WM_GETMINMAXINFO:
    {
      MINMAXINFO *pmmi = (MINMAXINFO *) lParam;
      pmmi->ptMinTrackSize.x = dummy_width;
      pmmi->ptMinTrackSize.y = dummy_height;
      pmmi->ptMaxTrackSize.x = dummy_width;
      pmmi->ptMaxTrackSize.y = dummy_height;
    }
    return 0;

  case WM_LBUTTONUP:
    {
      POINT pt;
      pt.x = (LONG) LOWORD(lParam);
      pt.y = (LONG) HIWORD(lParam);

      if (GetSys->PointIsInsideRect(_close_rect, pt))
      {
        trace("close cliced\n");

        if (!_isdeath)
        {
          trace("wm close nicht death\n");
          DWORD h;
          GetWindowThreadProcessId(_bad_win, &h);
          DestroyWindow(_bad_win);
          HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, h);
          if (hProcess!=NULL)
          {
            TerminateProcess(hProcess, 0);
            CloseHandle( hProcess );
          }
        }
      }

    }
    return 0;

  default:
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
  }

  return 0;
}

void HangUpInfoWindow::OnPaint(void)
{
  LogMethod;

  PAINTSTRUCT ps = { NULL };
  HDC hDC = BeginPaint(m_hWnd, &ps);

  DrawIconEx (hDC, 3, 8, _icon, 48, 48, 0, (HBRUSH) GetStockObject(WHITE_BRUSH), DI_NORMAL); 
  DrawIconEx (hDC, _close_rect.left, _close_rect.top, _close_icon, 20, 20, 0,
    (HBRUSH) GetStockObject(WHITE_BRUSH), DI_NORMAL); 


  SelectObject(hDC, m_hFont);
  RECT rc;
  SetRect(&rc, 57, 10, 270, 57);

  if (unicode)
  {
    wchar_t buf[256];
    if (_isdeath==false)
    {
      swprintf_s(buf, 256,
        L"Window '%s'\nhas not responded for %d seconds.\nClosing this window will kill the process.",
        title_w->c_str(), _seconds);

    } else
    {
      swprintf_s(buf, 256, L"Window '%s'\nis now responding!", title_w->c_str());
    }

    DrawTextW(hDC, buf, (int) wcslen(buf), &rc,DT_LEFT | DT_VCENTER);
  } else
  {
    char buf[256];
    if (_isdeath==false)
    {
      sprintf_s(buf, 256,
        "Window '%s'\nhas not responded for %d seconds.\nClosing this window will kill the process.",
        title->c_str(), _seconds);

    } else
    {
      sprintf_s(buf, 256, "Window '%s'\nis now responding!", title->c_str());
    }

    DrawText(hDC, buf, (int) strlen(buf), &rc,DT_LEFT | DT_VCENTER);
  }

  EndPaint(m_hWnd, &ps);

}
