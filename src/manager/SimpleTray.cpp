/////////////////////////////////////////////////////
// CSimpleTray - Simple tray icon implementation
//
// Copyright (c) 2003 - 3827 T1TAN 
// Copyright (c) 2003 - 3827 SprdSoft Inc.
// Modification by Jochen Baier, (c) 2009, 2011 
//
// This source code is FREE.
//

#include "SimpleTray.h"
#include "pager.h"
#include <shellapi.h>

using namespace std;

CSimpleTray::~CSimpleTray()
{
  LogMethod;

  m_nid.uFlags = 0;
  DestroyMenu(tray_menu);
  Shell_NotifyIcon(NIM_DELETE, &m_nid ); 
}

HMENU CSimpleTray::GetTrayMenu(void)
{
  LogMethod;

  return tray_menu;
} 

CSimpleTray::CSimpleTray(HICON icon, HWND message_window)
{
  LogMethod;

  if (icon==NULL)
  {
    trace("no valid icon\n");
    return;
  }

  tray_menu = CreatePopupMenu();
  InsertMenu(tray_menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUQUIT), "Quit");
  InsertMenu(tray_menu, 0, MF_BYPOSITION | MF_SEPARATOR, 2, NULL);
  InsertMenu(tray_menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUSELECTOR), 
    "Add window(s) to the ignore list");
  InsertMenu(tray_menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUHELP), 
    "Open help web site");
  InsertMenu(tray_menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUDOWNLOAD), 
    "Open download web site");
  InsertMenu(tray_menu, 0, MF_BYPOSITION, static_cast<UINT_PTR>(MENUABOUT), "About");

  AppendMenu(tray_menu, MF_SEPARATOR, static_cast<UINT_PTR>(0), "");

  int i;
  for (i = 1; i <= GetPager->GetNumberOfDesktops(); i++)
  {
    char buf[256];
    sprintf_s(buf, 256, "Desktop %d", i);
    if (i==1)
    {
      AppendMenu(tray_menu, MF_STRING|MF_CHECKED, static_cast<UINT_PTR>(MENUSWITCHTO+i), buf);
    } else
    {
      AppendMenu(tray_menu, MF_STRING, static_cast<UINT_PTR>(MENUSWITCHTO+i), buf);
    }
  }

  char temp[10]="Desktop 1";
  strcpy_s(m_nid.szTip, _countof(m_nid.szTip), temp); 

  m_nid.cbSize	= sizeof(NOTIFYICONDATA);
  m_nid.hWnd		=message_window; 
  m_nid.uID		= 100;
  m_nid.uCallbackMessage = WM_TRAYNOTIFY;	
  m_nid.szTip[ strlen(m_nid.szTip) ] = '\0';
  m_nid.hIcon = icon;
  m_nid.uFlags =	NIF_MESSAGE |	NIF_ICON |	NIF_TIP;	

  Shell_NotifyIcon( NIM_ADD, &m_nid );
}

void CSimpleTray::SetIcon(int desk_number, HICON hIcon)
{
  LogMethod;

  if ( hIcon == NULL ) return;

  int i;
  for (i = 1; i <= GetPager->GetNumberOfDesktops(); i++)
  {
    char buf[256];
    sprintf_s(buf, 256, "Desktop %d", i);
    if (i==desk_number)
    {
      ModifyMenu(tray_menu, MENUSWITCHTO+i, MF_STRING|MF_CHECKED, 
        static_cast<UINT_PTR>(MENUSWITCHTO+i), buf);
    } else
    {
      ModifyMenu(tray_menu, MENUSWITCHTO+i, MF_STRING, 
        static_cast<UINT_PTR>(MENUSWITCHTO+i), buf);
    }
  }

  char temp[256];
  sprintf_s(temp, 256, "Desktop %d", desk_number); 
  strcpy_s(m_nid.szTip,_countof(m_nid.szTip), temp);
  m_nid.szTip[ strlen(m_nid.szTip) ] = '\0';

  m_nid.hIcon =hIcon;
  m_nid.uFlags = NIF_ICON | NIF_TIP;	
  Shell_NotifyIcon( NIM_MODIFY, &m_nid );
}

void CSimpleTray::SetTooltip(std::string lpTooltip)
{
  LogMethod;

  strcpy_s(m_nid.szTip, _countof(m_nid.szTip), lpTooltip.c_str());
  m_nid.uFlags = NIF_TIP;
  Shell_NotifyIcon( NIM_MODIFY, &m_nid );
}

//thanks to http://www.wolfgang-rolke.de/dev_tips/dev_12.htm
HICON CSimpleTray::CreateIconFromNumber(int number)
{
  LogMethod;

  int cxIcon = 16;
  int cyIcon = 16;

  HDC hdcScreen = GetDC(NULL);
  if (hdcScreen == NULL)
    return NULL;

  HDC hdcMemColor = CreateCompatibleDC(hdcScreen);
  if (hdcMemColor == NULL)
  {
    ReleaseDC(NULL, hdcScreen);
    return NULL;
  }
  HDC hdcMemMask = CreateCompatibleDC(hdcMemColor);
  if (hdcMemMask == NULL)
  {
    DeleteDC(hdcMemColor);
    ReleaseDC(NULL, hdcScreen);
    return NULL;
  }

  ICONINFO iconinfo;
  iconinfo.fIcon = TRUE;
  iconinfo.xHotspot = 0;
  iconinfo.yHotspot = 0;
  iconinfo.hbmMask = CreateBitmap(cxIcon, cyIcon, 1, 1, NULL);
  if (iconinfo.hbmMask == NULL)
  {
    DeleteDC(hdcMemMask);
    DeleteDC(hdcMemColor);
    ReleaseDC(NULL, hdcScreen);
    return NULL;
  }
  iconinfo.hbmColor = CreateCompatibleBitmap(hdcScreen, cxIcon, cyIcon);
  if (iconinfo.hbmColor == NULL)
  {
    DeleteObject(iconinfo.hbmMask);
    DeleteDC(hdcMemMask);
    DeleteDC(hdcMemColor);
    ReleaseDC(NULL, hdcScreen);
    return NULL;
  }
  ReleaseDC(NULL, hdcScreen);

  HBITMAP hbmMaskOld = (HBITMAP)SelectObject(hdcMemMask, iconinfo.hbmMask);
  PatBlt(hdcMemMask, 0, 0, cxIcon, cyIcon, WHITENESS);
  HBITMAP hbmColorOld = (HBITMAP)SelectObject(hdcMemColor, iconinfo.hbmColor);
  PatBlt(hdcMemColor, 0, 0, cxIcon, cyIcon, BLACKNESS);

  HPEN pen_black = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
  HPEN pen_white = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

  SelectObject(hdcMemColor, pen_white);
  MoveToEx(hdcMemColor, 0, 0, NULL);
  LineTo(hdcMemColor, 0, 15); 
  LineTo(hdcMemColor, 15, 15); 
  LineTo(hdcMemColor, 15, 0); 
  LineTo(hdcMemColor, 0, 0);

  SelectObject(hdcMemMask, pen_black);
  MoveToEx(hdcMemMask, 0, 0, NULL);
  LineTo(hdcMemMask, 0, 15); 
  LineTo(hdcMemMask, 15, 15); 
  LineTo(hdcMemMask, 15, 0); 
  LineTo(hdcMemMask, 0, 0);

  LOGFONT lf;
  memset((void *)&lf, 0, sizeof(lf));
  lf.lfHeight = cyIcon;
  lf.lfWeight = FW_BOLD;
  strcpy_s(lf.lfFaceName, _countof(lf.lfFaceName), "Arial");

  HFONT hfont = CreateFontIndirect(&lf);
  if (hfont != NULL)
  {
    SelectObject(hdcMemMask, hfont);
    SelectObject(hdcMemColor, hfont);
  }

  SetTextColor(hdcMemColor, RGB(255, 255, 255));
  SetBkMode(hdcMemMask, TRANSPARENT);
  SetBkMode(hdcMemColor, TRANSPARENT);

  char buf[10];
  sprintf_s(buf, _countof(buf), "%d", number);
  RECT rc;
  if (number==1)
  {
    rc.left = 0; rc.top = 0; rc.right = cxIcon; rc.bottom = cyIcon;
  } else
  {
    rc.left = 1; rc.top = 0; rc.right = cxIcon; rc.bottom = cyIcon;
  }
  
  DrawText(hdcMemMask, buf, (int) strlen(buf), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
  DrawText(hdcMemColor, buf, (int) strlen(buf), &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);

  hfont?DeleteObject(hfont):0;
  pen_black?DeleteObject(pen_black):0;
  pen_white?DeleteObject(pen_white):0;
  hbmMaskOld?SelectObject(hdcMemMask, hbmMaskOld):0;
  hbmColorOld?SelectObject(hdcMemColor, hbmColorOld):0;

  DeleteDC(hdcMemMask);
  DeleteDC(hdcMemColor);

  HICON hIcon = CreateIconIndirect(&iconinfo);

  DeleteObject(iconinfo.hbmColor);
  DeleteObject(iconinfo.hbmMask);

  return hIcon;
}
