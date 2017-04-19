/*
 Copyright:  Andy Pennell
 http://www.codeproject.com/KB/debug/postmortemdebug_standalone1.aspx
*/
#include "dbghelp.h"


// based on dbghelp.h
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
									CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
									CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
									CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
									);

class MiniDumper
{
  private:
    static LONG WINAPI CrashReporting( struct _EXCEPTION_POINTERS *pExceptionInfo, bool informUser);


 public:

   static LONG WINAPI SeriousCrash( struct _EXCEPTION_POINTERS *pExceptionInfo );
   static LONG WINAPI NonSeriousCrash( struct _EXCEPTION_POINTERS *pExceptionInfo );
};
