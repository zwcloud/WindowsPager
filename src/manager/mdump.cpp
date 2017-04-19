/*
Copyright:  Andy Pennell, Modifications: Jochen Baier
http://www.codeproject.com/KB/debug/postmortemdebug_standalone1.aspx
*/


#include "..\common\stdafx.h"
#include "mdump.h"
#include "System.h"
#include "Pager.h"
#include <fstream>
#include <iostream>

using namespace std;

LONG MiniDumper::SeriousCrash( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
  MiniDumper::CrashReporting(pExceptionInfo, true);
  return EXCEPTION_EXECUTE_HANDLER;
}

LONG MiniDumper::NonSeriousCrash( struct _EXCEPTION_POINTERS *pExceptionInfo )
{
  if (GetPager->CrashReportingOption()==2)
  {
    return EXCEPTION_EXECUTE_HANDLER;
  }

  if (GetPager->CrashReportingOption()==1)
  {
    MiniDumper::CrashReporting(pExceptionInfo, false);
  } else
  {
    MiniDumper::CrashReporting(pExceptionInfo, true);
  }
  return EXCEPTION_EXECUTE_HANDLER;
}


LONG MiniDumper::CrashReporting( struct _EXCEPTION_POINTERS *pExceptionInfo, bool informUser)
{
  //FIXME eigen ex werfen !!
  if (pExceptionInfo==NULL)
  {
    return EXCEPTION_CONTINUE_SEARCH;
  }

  LONG retval = EXCEPTION_CONTINUE_SEARCH;
  LPCSTR m_szAppName="WindowsPager";

  HMODULE hDll = NULL;

  char libFolder[_MAX_PATH];
  char szDbgHelpPath32Bit[_MAX_PATH];
  char szDbgHelpPath64bit[_MAX_PATH];

  if (GetModuleFileName( NULL, libFolder, _MAX_PATH ))
  {
    char *pSlash = _tcsrchr(libFolder, '\\' );
    pSlash++;
    *pSlash=0;

    sprintf_s(szDbgHelpPath32Bit, _countof(szDbgHelpPath32Bit), "%s\\DBGHELP32.DLL",  libFolder);
    sprintf_s(szDbgHelpPath64bit, _countof(szDbgHelpPath64bit), "%s\\DBGHELP64.DLL", libFolder);

    hDll = ::LoadLibrary(szDbgHelpPath32Bit);
    if (hDll==NULL)
    {
      hDll = ::LoadLibrary(szDbgHelpPath64bit);
    }
  }

  if (hDll==NULL)
  {
    MessageBox(NULL, "WindowsPager has trapped an error.\n\nBut the DLLs DBGHELP32.DLL and DBGHELP64.DLL\n"\
      "are not in the lib folder. Please unzip WindowsPager again.", "WindowsPager Crash Report", MB_OK);
    return retval;
  }

  LPCTSTR szResult = NULL;

  if (hDll)
  {
    MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress(hDll, "MiniDumpWriteDump" );
    if (pDump)
    {
      char szDumpPath[_MAX_PATH];
      sprintf_s(szDumpPath, _countof(szDumpPath), "%s..\\CrashReports", libFolder);
      CreateDirectory(szDumpPath, NULL);  

      char readMeFile[_MAX_PATH];
      sprintf_s(readMeFile, _countof(readMeFile), "%s..\\CrashReports\\Readme.txt", libFolder);
      ofstream a_file (readMeFile, ios::trunc );
      a_file << "WindowsPager\n\nPlease send all *.dmp files to email@Jochen-Baier.de .\nWith the help of these crash reports I can analyze the crash.\nThank you.";
      a_file.close();

      SYSTEMTIME stLocalTime;
      GetLocalTime( &stLocalTime );
      char szDumpFile[_MAX_PATH];

      #ifdef _WIN64
      sprintf_s(szDumpFile, _countof(szDumpFile), "%s\\WindowsPager64_%s_CrashReport-%04d-%02d-%02d-%02d-%02d-%02d-%02d.dmp", 
        szDumpPath, WPVERSION, stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, stLocalTime.wMilliseconds);
      #else
      sprintf_s(szDumpFile, _countof(szDumpFile), "%s\\WindowsPager32_%s_CrashReport-%04d-%02d-%02d-%02d-%02d-%02d-%02d.dmp", 
        szDumpPath, WPVERSION, stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
        stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, stLocalTime.wMilliseconds);
      #endif

      char szScratch [_MAX_PATH];

      // create the file
      HANDLE hFile = ::CreateFile(szDumpFile, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL );

      if (hFile!=INVALID_HANDLE_VALUE)
      {
        _MINIDUMP_EXCEPTION_INFORMATION ExInfo;

        ExInfo.ThreadId = ::GetCurrentThreadId();
        ExInfo.ExceptionPointers = pExceptionInfo;
        ExInfo.ClientPointers = NULL;

        // write the dump
        BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ExInfo, NULL, NULL );
        if (bOK)
        {
          sprintf_s( szScratch, _countof(szScratch), "WindowsPager has trapped an error. A crash report was generated.\n\nIf you want to help to improve WindowsPager\n"\
            "please send all crash report files (*.dmp) in the WindowsPager\\CrashReports folder to email@Jochen-Baier.de.\n\nThank you.");
          szResult = szScratch;
          retval = EXCEPTION_EXECUTE_HANDLER;
        }
        else
        {
          sprintf_s( szScratch, _countof(szScratch), "Failed to save dump file to '%s' (error %d)", szDumpPath, GetLastError() );
          szResult = szScratch;
        }
        ::CloseHandle(hFile);
      }
      else
      {
        sprintf_s( szScratch, _countof(szScratch), "Failed to create dump file '%s' (error %d)", szDumpPath, GetLastError() );
        szResult = szScratch;
      }
    }
    else
    {
      szResult = "DBGHELP.DLL too old. Please make sure the newest DBGHELP32/64.DLL is in the lib folder.";
    }
  }
  else
  {
    szResult = "DBGHELP.DLL not found. Please make sure DBGHELP32/64.DLL is in the lib folder.";
  }

  if (szResult && informUser)
  {
    ::MessageBox( NULL, szResult, "WindowsPager Crash Report", MB_OK );
  }

  return retval;
}
