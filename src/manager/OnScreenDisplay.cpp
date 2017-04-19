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
*    Jochen Baier, 2007, 2008, 2009 (email@Jochen-Baier.de)
*
*/


#include "..\common\stdafx.h"
#include "System.h"
//#include "OnScreenDisplay.h"
#include "XWinVer.h"
#include "Pager.h"


#define circle_radius 8
#define donat_width 2
#define offset 14

OnScreenDisplay::OnScreenDisplay()
{
  timer_count=0;
  switch_number=0;

  WNDCLASSEX wcl = { NULL };
  wcl.cbSize = sizeof(WNDCLASSEX);
  wcl.hInstance = hI;
  wcl.lpfnWndProc = (WNDPROC) WndProc;
  wcl.style = CS_HREDRAW | CS_VREDRAW;
  wcl.hIcon = LoadIcon(hI, IDC_ARROW);
  wcl.hCursor = LoadCursor(NULL, IDC_ARROW);
  wcl.hbrBackground = (HBRUSH) GetStockObject(BLACK_BRUSH);
  wcl.lpszClassName = "PagerOSD";
  RegisterClassEx(&wcl);

  m_hWnd =
    CreateWindowEx(WS_EX_NOACTIVATE | WS_EX_LAYERED |WS_EX_TOOLWINDOW | WS_EX_TOPMOST|WS_EX_TRANSPARENT,
    "PagerOSD", NULL, WS_POPUP,
    0,0, circle_radius*2, circle_radius*2, NULL, NULL, hI, this);

  SetLayeredWindowAttributes(m_hWnd, 0, 99, LWA_COLORKEY | LWA_ALPHA);
}

OnScreenDisplay::~OnScreenDisplay()
{
  UnregisterClass("PagerOSD", hI);
  KillTimer(m_hWnd, TIMER_ID_OSD);
  DestroyWindow(m_hWnd);
}

void OnScreenDisplay::HideOSD(void)
{
  KillTimer(m_hWnd , TIMER_ID_OSD);
  ShowWindow(m_hWnd , SW_HIDE);
}

LRESULT CALLBACK OnScreenDisplay::WndProc(HWND hWnd, UINT uMessage, WPARAM wParam,
                                          LPARAM lParam)
{
  static OnScreenDisplay *pThis = NULL;
  if (uMessage == WM_CREATE)
  {
    pThis = (OnScreenDisplay *) ((CREATESTRUCT *) (lParam))->lpCreateParams;
  }

  if (pThis!=NULL)
  {
    return pThis->WindowProc(hWnd, uMessage, wParam, lParam);
  } else
  {
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
  }
  
}

LRESULT OnScreenDisplay::WindowProc(HWND hWnd, UINT uMessage, WPARAM wParam,
                                    LPARAM lParam)
{
  switch (uMessage)
  {
  case WM_DESTROY:

    UnregisterClass("PagerOSD", hI);
    PostQuitMessage(0);
    return 0;

  case WM_PAINT:
    OnScreenDisplay::OnPaint();
    return 0;

  case WM_TIMER:
    if (wParam == TIMER_ID_OSD)
    {
      timer_count++;

      int x,y;
      GetSys->GetMousePos(x,y);
      SetWindowPos(m_hWnd, 0, x+offset, y+offset, 0, 0, SWP_NOSIZE);

      if (timer_count > 40)
      {
        KillTimer(hWnd, TIMER_ID_OSD);
        ShowWindow(m_hWnd, SW_HIDE);
        timer_count=0;
      }
    }
    return 0;

  default:
    return DefWindowProc(hWnd, uMessage, wParam, lParam);
  }

  return 0;
}

void OnScreenDisplay::OnPaint(void)
{
  PAINTSTRUCT ps = { NULL };
  HDC hDC = BeginPaint(m_hWnd, &ps);

  SelectObject(hDC, GetSys->GetPenNull());
  SelectObject(hDC, GetSys->GetBrushRed());
  Ellipse(hDC, 0, 0, circle_radius*2, circle_radius*2);

  SelectObject(hDC, GetSys->GetPenNull());
  SelectObject(hDC, GetSys->GetBrushBlack());
  Ellipse(hDC, circle_radius -(circle_radius-donat_width) , circle_radius -(circle_radius-donat_width),
    circle_radius +(circle_radius-donat_width), circle_radius +(circle_radius-donat_width));

  EndPaint(m_hWnd, &ps);

}

void OnScreenDisplay::ShowDesktopNumber(int number)
{
  KillTimer(m_hWnd, TIMER_ID_OSD);

  /*HWND forground=GetForegroundWindow();
  if (forground)
  {
  trace("have forground\n");
  RECT for_rect;
  if (GetWindowRect(forground, &for_rect))
  {

  trace("for rect: %d, %d, %d, %d\n", for_rect.left, for_rect.top,
  for_rect.right, for_rect.bottom);

  if (EqualRect(&GetSys->GetFullScreenRect(), &for_rect))
  {
  trace("recect osd window\n");
  return;
  } else
  {
  trace("not equal\n");
  }
  }
  }*/

  int x,y;
  GetSys->GetMousePos(x,y);
  SetWindowPos(m_hWnd, 0, x+offset, y+offset, 0, 0, SWP_NOSIZE);

  if (!IsWindowVisible(m_hWnd))
  {
    ShowWindow(m_hWnd, SW_SHOW);
  }

  InvalidateRect(m_hWnd, NULL, true);
  UpdateWindow(m_hWnd);

  SetTimer(m_hWnd, TIMER_ID_OSD, 50, NULL);
}
