#pragma once

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define _WIN32_WINDOWS 0x0500
#define NTDDI_VERSION NTDDI_WIN2KSP4

#include <windows.h>
#include <Windowsx.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <string>
#include <iostream>
#include <vector>
#include <cstdio>
#include <algorithm>
#include <map>
#include <set>
#include <utility>
#include <gdiplus.h>
#include <cctype>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "..\common\smartfix.h"
#include "..\common\\Messages.h"

#if !defined (WS_EX_TOOLWINDOW)
#define WS_EX_TOOLWINDOW 0x00000080L
#endif

#if !defined (WS_EX_NOACTIVATE)
#define WS_EX_NOACTIVATE 0x08000000L
#endif

#if !defined (WS_EX_NOANIMATION)
#define WS_EX_NOANIMATION 0x04000000L
#endif

//#define LOGGING_ENABLED
extern void trace(const char *str, ...);
extern void tracew(const WCHAR *str, ...);

extern void _Log(const char *file, const int lineNr, const char *function);
extern void LogExtra(const char *str, ...);
extern void LogExtraW(const WCHAR *str, ...);

#define LogMethod  _Log(__FILE__, __LINE__,  __FUNCTION__)

#define WPVERSION "1.02"

#define SHADOW 1
#define FOCUS 1

extern HINSTANCE hI;

#define TIMER_ID_OSD 200000001
#define TIMER_ID_DND 200000002
#define TIMER_ID_DIE_SLOWLY 200000004

#define SINGLE_MONITOR 200000006
#define ONE_BIG_SCREEN 200000007
#define PAGER_ON_PRIMARY 200000008

#define PRIMARY_MONITOR   200000009
#define SECONDARY_MONITOR 200000010

#define MENUQUIT 1001
#define MENUHELP 1002
#define MENUABOUT 1003
#define MENUSELECTOR 1004
#define MENUDOWNLOAD 1005
#define MENUSHUTDOWN 1006
#define MENUREBOOT 1007
#define MENULOGOFF 1008

#define MENUSLEEP 1010
#define MENUHYPERNATE 1011


#define MENUSWITCHTO 2000
