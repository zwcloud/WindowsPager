// XWinVer.cpp  Version 1.2
//
// Original Author:  Hans Dietrich
//          hdietrich@gmail.com
//
// Modifications: Jochen Baier, 2010
//
// License:
//     This software is released into the public domain.  You are free to use
//     it in any way you like, except that you may not sell this source code.
//
//     This software is provided "as is" with no expressed or implied warranty.
//     I accept no liability for any damage or loss of business that this
//     software may cause.
//
///////////////////////////////////////////////////////////////////////////////

#include "..\common\stdafx.h"
#include "XWinVer.h"

#include "System.h"

using namespace std;

XWinVersion *XWinVersion::_instance = NULL;

XWinVersion *XWinVersion::Instance()
{
  LogMethod;

  if (_instance == NULL)
  {
    _instance = new XWinVersion;
  }
  return _instance;
}

void XWinVersion::Init()
{
  LogMethod;

  OSVERSIONINFOEX os_info;
  ZeroMemory(&os_info, sizeof(os_info));
  os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  if (GetVersionEx((LPOSVERSIONINFO) &os_info))
  {
    version=ParseInfo(os_info);
  }
}

int XWinVersion::ParseInfo(OSVERSIONINFOEX& m_osinfo)
{
  LogMethod;

  int nVersion = WUNKNOWN;

  DWORD dwPlatformId = m_osinfo.dwPlatformId;
  DWORD dwMinorVersion = m_osinfo.dwMinorVersion;
  DWORD dwMajorVersion = m_osinfo.dwMajorVersion;
  DWORD dwBuildNumber = m_osinfo.dwBuildNumber & 0xFFFF;

  if ((dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) && (dwMajorVersion == 4))
  {
    if ((dwMinorVersion < 10) && (dwBuildNumber == 950))
    {
      nVersion = W95;
    } else if ((dwMinorVersion < 10) &&
      ((dwBuildNumber > 950) && (dwBuildNumber <= 1080)))
    {
      nVersion = W95SP1;
    } else if ((dwMinorVersion < 10) && (dwBuildNumber > 1080))
    {
      nVersion = W95OSR2;
    } else if ((dwMinorVersion == 10) && (dwBuildNumber == 1998))
    {
      nVersion = W98;
    } else if ((dwMinorVersion == 10) &&
      ((dwBuildNumber > 1998) && (dwBuildNumber < 2183)))
    {
      nVersion = W98SP1;
    } else if ((dwMinorVersion == 10) && (dwBuildNumber >= 2183))
    {
      nVersion = W98SE;
    } else if (dwMinorVersion == 90)
    {
      nVersion = WME;
    }
  } else if (dwPlatformId == VER_PLATFORM_WIN32_NT)
  {
    if ((dwMajorVersion == 3) && (dwMinorVersion == 51))
    {
      nVersion = WNT351;
    } else if ((dwMajorVersion == 4) && (dwMinorVersion == 0))
    {
      nVersion = WNT4;
    } else if ((dwMajorVersion == 5) && (dwMinorVersion == 0))
    {
      nVersion = W2K;
    } else if ((dwMajorVersion == 5) && (dwMinorVersion == 1))
    {
      nVersion = WXP;
    } else if ((dwMajorVersion == 5) && (dwMinorVersion == 2))
    {
      nVersion = W2003;
    } else if ((dwMajorVersion == 6) && (dwMinorVersion == 0))
    {
      nVersion = WVISTA;
    }
    else if ((dwMajorVersion == 6) && (dwMinorVersion == 1))
    {
      nVersion = WSEVEN;
    }
  } else if (dwPlatformId == VER_PLATFORM_WIN32_CE)
  {
    nVersion = WCE;
  }

  return nVersion;
}

BOOL XWinVersion::IsSeven()
{
  LogMethod;

  if (version==WSEVEN)
  {
    return TRUE;
  }
  return FALSE;
}

BOOL XWinVersion::IsXP()
{
  LogMethod;

  if (version==WXP)
  {
    if (GetSys->GetXPTheme() != 4)
    {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL XWinVersion::IsXPClassic()
{
  LogMethod;

  if (version==WXP)
  {
    if (GetSys->GetXPTheme()==4)
    {
      return TRUE;
    }
  }
  return FALSE;
}

BOOL XWinVersion::IsWin2000()
{
  LogMethod;

  if (version==W2K)
  {
    return TRUE;
  }
  return FALSE;
}

BOOL XWinVersion::IsVista()
{
  LogMethod;

  if (version==WVISTA || version==WSEVEN)
  {
    return TRUE;
  }
  return FALSE;
}