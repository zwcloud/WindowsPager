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

#define _WIN32_WINNT 0x0500
#define WINVER 0x0500
#define _WIN32_WINDOWS 0x0500
#define NTDDI_VERSION NTDDI_WIN2KSP4
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tchar.h>
#include <iostream>
#include <cstdio>

#include "..\hook\Hook.h"
#include "..\common\Messages.h"
#include "..\common\smartfix.h"

using namespace std; 

std::wstring currentFolder;


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

#ifdef _M_X64
void Inject64Dll(HWND window)
#else
void Inject32Dll(HWND window)
#endif
{
  try
  {
    if (!IsWindow(window))
    {
      return;
    }

    DWORD pId;
    if (GetWindowThreadProcessId(window, &pId)!=0)
    {
      HANDLE hProcess = OpenProcess(
        PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
        PROCESS_VM_OPERATION |PROCESS_VM_WRITE | PROCESS_VM_READ |
        PROCESS_TERMINATE , FALSE, pId);
      if(hProcess)
      {
        DWORD dwThreadId;
        LPVOID pvProcMem = VirtualAllocEx(hProcess, 0, currentFolder.length() *sizeof(WCHAR),
          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        if (pvProcMem != NULL)
        {
          SIZE_T written;
          if (WriteProcessMemory(hProcess, pvProcMem, currentFolder.c_str(), currentFolder.length() *sizeof(WCHAR), &written))
          {
            HANDLE hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryW,
              pvProcMem, 0, &dwThreadId);
            if (hThread!=NULL)
            {
              DWORD wtso = WAIT_FAILED;
              wtso = WaitForSingleObject(hThread, 1000);
              CloseHandle(hThread);
            }
          }

          VirtualFreeEx(hProcess, pvProcMem, currentFolder.length() *sizeof(WCHAR), MEM_RELEASE);
        }
        CloseHandle(hProcess);
      }
    }

  } catch (...)
  {
  }
}

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
  if (uMessage == WM_DESTROY)
  {
    PostQuitMessage(0);
    return 0;
  }

  if (uMessage == WM_CLOSE)
  {
#ifdef _M_X64
    UnInstallFilters64();
#else
     UnInstallFilters32();
#endif

    DestroyWindow(hWnd);
    return 0;
  }

#ifdef _M_X64
  if (uMessage == MSG_INJECT64_DLL)
  {
    Inject64Dll((HWND) wParam);
    return 0;
  }
#else
 if (uMessage == MSG_INJECT32_DLL)
  {
    Inject32Dll((HWND) wParam);
    return 0;
  }
#endif
 
  return DefWindowProc(hWnd, uMessage, wParam, lParam);
}


bool GetDirectoryFromExe(std::wstring& currentDirectory)
{
  WCHAR buf[MAX_PATH];
  int bytes = GetModuleFileNameW(NULL, buf, MAX_PATH);
  if (bytes==0)
  {
    return false;
  }

  std::wstring str(buf);

  size_t found;
  found=str.find_last_of(L"\\");
  if (found==0 || found ==-1)
  {
    return false;
  }

  currentDirectory=str.substr(0,found);

  return true;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                       HINSTANCE hPrevInstance,
                       LPTSTR    lpCmdLine,
                       int       nCmdShow)
{
  UNREFERENCED_PARAMETER(hPrevInstance);

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

  if (!found)
  {
    MessageBox(0, "Please do not start 'hookstarter32/64.exe' by hand.\nUse 'windowspager.exe'",
      "WindowsPager", MB_ICONHAND);

    return 0;
  }

  if (lpCmdLine==NULL || strlen(lpCmdLine)==0)
  {
    trace("command line 0\n");
    MessageBox(0, "Bad bug ;(. hookstarter32/64.cpp line 112",
      "WindowsPager", MB_ICONHAND);
    exit(1);
  }

  long win_id= strtol(lpCmdLine, NULL, 16); 
  if (win_id==0 || win_id==LONG_MAX || win_id==LONG_MIN)
  {
    MessageBox(0, "Bad bug ;(. hookstarter32/64.cpp line 120",
      "WindowsPager", MB_ICONHAND);
    exit(1);
  }

  HWND hwnd_main=(HWND) win_id;
  trace("main window: %x\n", hwnd_main);


  WNDCLASS wc;
  ZeroMemory(&wc, sizeof(wc));
  wc.style = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc = (WNDPROC) WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = NULL;
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)NULL;
  wc.lpszMenuName = NULL;

#ifdef _M_X64
  wc.lpszClassName ="PagerHookStarter64";
#else
  wc.lpszClassName ="PagerHookStarter32";
#endif


  RegisterClass(&wc);

  UINT m_hookMessage = RegisterWindowMessage("PagerHookMessages");

  HWND panel_window = FindWindow("Shell_TrayWnd", NULL);
  HWND list_window = FindWindowEx((HWND) panel_window, NULL, "ReBarWindow32", NULL);
  HWND tray_window = FindWindowEx((HWND) panel_window, NULL, "TrayNotifyWnd", NULL);
  HWND tmp = FindWindowEx(list_window, NULL, "MSTaskSwWClass", NULL);
  HWND taskbuttonwindow = FindWindowEx(tmp, NULL, "ToolbarWindow32", NULL);
  if (taskbuttonwindow==NULL)
  {
    taskbuttonwindow = FindWindowEx(tmp, NULL, "MSTaskListWClass", NULL);
  }
  if (taskbuttonwindow==NULL)
  {
    taskbuttonwindow = FindWindowEx(tmp, NULL, "SysTabControl32", NULL);
  }

  int num_desktops= (int)  GetProp (hwnd_main, "PAGER_NUM_DESKTOPS");
  if (num_desktops==0)
  {
    num_desktops=4;
  }

  int modifier= (int) GetProp(panel_window, "PagerMoveAndSwitchModifier");

#ifdef _M_X64
  InitHooksDll64(hwnd_main, tray_window,
    taskbuttonwindow, list_window, m_hookMessage, num_desktops, modifier);

  HWND hookstarter_window =
    CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE, L"PagerHookStarter64", L"WindowsPager Hookstarter",
    WS_POPUP, -1, -1,
    1, 1, NULL, NULL, hInstance, (LPVOID) NULL);
#else
   InitHooksDll32(hwnd_main, tray_window,
    taskbuttonwindow, list_window, m_hookMessage, num_desktops, modifier);

  HWND hookstarter_window =
    CreateWindowExW(WS_EX_TOOLWINDOW|WS_EX_NOACTIVATE, L"PagerHookStarter32", L"WindowsPager Hookstarter",
    WS_POPUP, -1, -1,
    1, 1, NULL, NULL, hInstance, (LPVOID) NULL);
#endif


  //FIXME nötig ???
  ShowWindow(hookstarter_window, SW_SHOWNOACTIVATE);

  if (GetDirectoryFromExe(currentFolder))
  {
#ifdef _M_X64
    currentFolder.append(L"\\WindowsPagerApiHook64bit.dll");
#else
    currentFolder.append(L"\\WindowsPagerApiHook32bit.dll");
#endif

    //MessageBoxW(NULL, currentFolder.c_str(), L"dll path", MB_OK);
  }

#ifdef _M_X64
  PostMessage(hwnd_main, m_hookMessage, MSG_HOOK_INSTALLED_64, (LPARAM) 1);
#else
  PostMessage(hwnd_main, m_hookMessage, MSG_HOOK_INSTALLED_32, (LPARAM) 1);
#endif

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