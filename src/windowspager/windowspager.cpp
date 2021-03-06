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
*    Jochen Baier, 2007, 2008, 2009, 2010 (email@Jochen-Baier.de)
*
*/

#include "..\common\stdafx.h"

using namespace std; 
#include <shobjidl.h>
#include <shellapi.h>
#include <sys/stat.h> 

ITaskbarList  *pTaskbar;

using namespace boost;

void RestoreAllWindows();
void StartPagerThread(void);

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

struct set_window_long_data
{
  HWND hWnd;
  int nIndex;
  LONG_PTR dwNewLong;
};

static DWORD WINAPI SetWindowLongPtrTimeoutThread(LPVOID lpParam)
{
  set_window_long_data  *my_data = (set_window_long_data *) lpParam;

  HWND hWnd = my_data->hWnd;
  int nIndex=my_data->nIndex;
  LONG dwNewLong = (LONG) my_data->dwNewLong;

  delete my_data;

#pragma warning( disable: 4244)
  SetWindowLongPtr(hWnd, nIndex, (LONG_PTR) dwNewLong);
#pragma warning(default: 4244)
  return 0;
}

bool SetWindowLongPtrTimeout(HWND hWnd, int nIndex, 
                             LONG_PTR dwNewLong, int timeout)
{
  set_window_long_data  *my_data = (set_window_long_data *) new set_window_long_data;
  my_data->hWnd = hWnd;
  my_data->nIndex = nIndex;
  my_data->dwNewLong = dwNewLong;

  shared_ptr<void> threadHandle (CreateThread(0, 0, SetWindowLongPtrTimeoutThread, 
    my_data, 0, NULL), CloseHandle);

  if (WaitForSingleObject(threadHandle.get(), timeout) == WAIT_TIMEOUT)
  {
    return false;
  }

  return true;
}

bool CheckIfFileExists(std::string strFilename) 
{
  struct stat stFileInfo;
  bool blnReturn;
  int intStat;

  intStat = stat(strFilename.c_str(), &stFileInfo);
  if(intStat == 0)
  {
    blnReturn = true;
  } else
  {
    blnReturn = false;
  }

  return(blnReturn);
}

void RestoreWindow(HWND window)
{
  if (GetProp(window, "PAGER_WINDOW")) 
  {
    RemoveProp(window, "PAGER_WINDOW");
 
    LRESULT res=SendMessageTimeout(window, MSG_RESET_SUBCLASS_CRASH, 
        (WPARAM) 0, 0, SMTO_ABORTIFHUNG|SMTO_BLOCK, 200, NULL);

    LONG_PTR old_style_ex = GetWindowLongPtr(window, GWL_EXSTYLE);
    SetWindowLongPtrTimeout(window, GWL_EXSTYLE, old_style_ex 
      & ~(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE), 200);

	EnableWindow(window, true);

    if (GetProp(window, "PAGER_WINDOW_BUTTON"))
    {
      RemoveProp(window, "PAGER_WINDOW_BUTTON");
      pTaskbar->AddTab(window);
    }

    if (!IsIconic(window))
    {
      HMONITOR monitor=MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
      if (monitor==NULL)
      {
        SetWindowPos(window, NULL,
          10, 10,
          0, 0, SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|
          SWP_ASYNCWINDOWPOS | SWP_FRAMECHANGED);
      }
    }

  }
}

void RestoreAllWindows()
{
  vector <HWND> zorder2;
  HWND iter_win2 = GetTopWindow(NULL);
  if(!iter_win2)
  {
    return;
  }

  while(iter_win2)
  {
    zorder2.push_back(iter_win2);
    iter_win2= GetNextWindow(iter_win2, GW_HWNDNEXT); 
  }

  vector <HWND> ::reverse_iterator riter2 = zorder2.rbegin();
  while (riter2 != zorder2.rend())
  {
    RestoreWindow(*riter2);
    riter2++;
  }
}

bool GetCurrentDirectory(std::string& currentDirectory)
{
 char buf[MAX_PATH];
 int bytes = GetModuleFileNameA(NULL, buf, MAX_PATH);
 if (bytes==0)
 {
   return false;
 }

 std::string str(buf);
 
 size_t found;
 found=str.find_last_of("\\");
 if (found==0 || found ==-1)
 {
   return false;
 }
 currentDirectory=str.substr(0,found);
 return true;
}

static DWORD WINAPI PagerThread(LPVOID lpParam)
{
  trace("pager thread\n");

  PROCESS_INFORMATION pif;
  STARTUPINFO si; 
  ZeroMemory(&si,sizeof(si));
  si.cb = sizeof(si);

  std::string currentDirectory;
  if (!GetCurrentDirectory(currentDirectory))
  {
    MessageBox(NULL, "OHOH", "kdfjk", MB_OK);
    return 0;
  }
  trace("currentdirectory: %s\n", currentDirectory.c_str());


  string path=currentDirectory+"\\lib\\manager.exe";
  trace("path: %s\n", path.c_str());

  if (!CheckIfFileExists(path.c_str()))
  {
    MessageBox(0, "Missing manager.exe in the 'lib' folder! Will quit now.",
      "WindowsPager", MB_ICONHAND);
    CoUninitialize();
    exit(1);
  }

  if (CreateProcess(NULL,
    (LPSTR) path.c_str(), NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pif))
  {
    WaitForSingleObject( pif.hProcess, INFINITE ); 
    CloseHandle(pif.hProcess); 
    CloseHandle(pif.hThread); 
  }

  CoInitialize(0);
  CoCreateInstance(CLSID_TaskbarList, 0,
    CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void **) &pTaskbar);
  pTaskbar->HrInit();

  RestoreAllWindows();

  pTaskbar->Release();
  CoUninitialize();


  HWND hook_starter64 = FindWindow("PagerHookStarter64", NULL);
  if (hook_starter64)
  {
    PostMessage(hook_starter64, WM_CLOSE, 0, 0);
  }

  HWND hook_starter32 = FindWindow("PagerHookStarter32", NULL);
  if (hook_starter32)
  {
    PostMessage(hook_starter32, WM_CLOSE, 0, 0);
  }

  exit(0);    
  return 0;
}

bool AllOk(void)
{
  HWND panel_window=NULL;
  panel_window = FindWindow("Shell_TrayWnd", NULL);
  if (panel_window==NULL)
  {
    return FALSE;
  }

  HWND list_window=NULL;
  list_window = FindWindowEx((HWND) panel_window, NULL, "ReBarWindow32", NULL);
  if (list_window==NULL)
  {
    return FALSE;
  }

  HWND tray_window=NULL;
  tray_window = FindWindowEx((HWND) panel_window, NULL, "TrayNotifyWnd", NULL);
  if (tray_window==NULL)
  {
    return FALSE;
  }

  return TRUE;
}

int CALLBACK AutostartTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
  if(!AllOk())
  {
    return 0;
  }

  KillTimer (NULL, idTimer);
  StartPagerThread(); 
  return 0;
}

void StartPagerThread(void)
{
  shared_ptr<void> threadHandle (CreateThread(0, 0, PagerThread, 
    NULL, 0, NULL), CloseHandle);
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);
  UNREFERENCED_PARAMETER(lpCmdLine);

  SetLastError(0);
  HANDLE hMutex =  CreateMutexA   (NULL, TRUE, "windowspager");
  bool found=false;
  if(GetLastError() == ERROR_ALREADY_EXISTS)
  {
    found = true;
  }

  if(hMutex) 
  {
    ReleaseMutex (hMutex);
  }

  if (found)
  {
    MessageBox(0, "WindowsPager is already running.", "WindowsPager", MB_ICONHAND);
    return FALSE;
  }

  if (AllOk())
  {
    StartPagerThread(); 
  } else
  {
    SetTimer(NULL, 20, 1000, (TIMERPROC) AutostartTimer);   
  }

  BOOL bRet;
  MSG msg; 
  while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
  { 
    if (bRet == -1)
    {
      exit (1);
    }
    else
    {
      TranslateMessage(&msg); 
      DispatchMessage(&msg); 
    }
  }

  return (int) msg.wParam;
}
