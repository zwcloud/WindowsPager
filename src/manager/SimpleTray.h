/////////////////////////////////////////////////////
// CSimpleTray - Simple tray icon implementation
//
// Copyright (c) 2003 - 3827 T1TAN 
// Copyright (c) 2003 - 3827 SprdSoft Inc.
// Modification by Jochen Baier, (c) 2009
//
// This source code is FREE.
//

#pragma once


#include "..\common\stdafx.h"
#include <shellapi.h>

// custom window message
#define WM_TRAYNOTIFY WM_APP+1

class CSimpleTray  
{
public:
  CSimpleTray(HICON icon, HWND message_window);
  ~CSimpleTray();

  HMENU GetTrayMenu(void);
  void SetIcon(int desk_number, HICON hIcon );
  void SetTooltip(std::string lpTooltip);
  static HICON CreateIconFromNumber(int number);

private:
  HMENU tray_menu;
  NOTIFYICONDATA m_nid;
};
