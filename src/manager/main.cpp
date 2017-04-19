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
#include "Pager.h"
#include "XWinver.h"
#include "mdump.h"

#include <fstream>
#include <iostream>


#include "HangUpInfoWindow.h"

using namespace std;
using namespace Gdiplus;

HINSTANCE hI;

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

void _Log(const char *file, const int lineNr, const char *function)
{
#ifdef LOGGING_ENABLED
  // trace("%s(%d): %s\n", file, lineNr, function);

  char buf[512];
  sprintf(buf, "%s(%d): %s\n", file, lineNr, function);

  ofstream a_file ( "wpLogFile.txt", ios::app );
  a_file << buf;
  a_file.close();
#endif
}

void LogExtra(const char *str, ...)
{
#ifdef LOGGING_ENABLED
  char buf_extra[256];
  va_list ptr;
  va_start(ptr, str);
  vsprintf_s(buf_extra, 256, str, ptr);

  //trace("Extra: %s\n", buf_extra);

  ofstream a_file ( "wpLogFile.txt", ios::app );
  a_file << buf_extra;
  a_file.close();
#endif
}

void LogExtraW(const WCHAR *str, ...)
{
#ifdef LOGGING_ENABLED
  WCHAR buf[1024];
  va_list ptr;
  va_start(ptr, str);
  vswprintf_s(buf, 1024, str, ptr);

  //trace("Extra: %s\n", buf_extra);

  wofstream a_file ( "wpLogFile.txt", ios::app );
  a_file << buf;
  a_file.close();
#endif
}

void tracew(const WCHAR *str, ...)
{
#ifdef _DEBUG
  WCHAR buf[2048];
  va_list ptr;
  va_start(ptr, str);
  vswprintf_s(buf, 2048, str, ptr);
  OutputDebugStringW(buf);
#endif
}

BOOL WINAPI myRoutine(DWORD dwCtrlType)
{
  GetPager->RestoreWindowsBeforeExit();
  delete GetPager;
  return FALSE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  SetUnhandledExceptionFilter(MiniDumper:: SeriousCrash);

  __try
  {

#ifdef LOGGING_ENABLED
    ofstream a_file ( "wpLogFile.txt", ios::trunc );
    a_file.close();
#endif

    LogMethod;

    CoInitialize(0);
    if (NOERROR != OleInitialize(NULL))
    {
      return FALSE; 
    }

    hI = hInstance;

    SetLastError(0);
    HANDLE hMutex = CreateMutex(NULL, TRUE, "windowspager");
    bool found = false;
    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
      found = true;
    }

    if (hMutex)
    {
      ReleaseMutex(hMutex);
    }

#ifndef _DEBUG
    if (!found && !IsDebuggerPresent())
    {
      MessageBox(0, "Please do not start 'manager.exe' by hand.\nUse 'windowspager.exe'",
        "WindowsPager", MB_ICONHAND);

      OleUninitialize();
      CoUninitialize();          
      return FALSE;
    }
#endif


    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    GetWinVersion->Init();
    GetSys->Init();

    //Test if running the wrong Windowspager version...
    if (GetSys->Is64BitWindows())
    {
      #ifndef _WIN64 //Windowspager is not 64 bit

      MessageBox(NULL, "You started the 32 bit version of WindowsPager on a 64 bit Windows operation system.\nPlease start "\
                        "the 64 bit version of Windowspager. Will quit now.", "WindowsPager", MB_OK);
      exit(1);
      #endif
    }

    GetPager->Init();

    SetConsoleCtrlHandler((PHANDLER_ROUTINE) myRoutine, TRUE);


    BOOL bRet;
    MSG msg;
    while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
    {
      if (bRet == -1)
      {
        exit(1);
      } else
      {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    OleUninitialize();
    CoUninitialize();
    GdiplusShutdown(gdiplusToken);

    return (int) msg.wParam;

  } __except(MiniDumper::SeriousCrash(GetExceptionInformation()))
  {
    return 1;
  }

}
