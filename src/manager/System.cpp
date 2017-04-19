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
#include "XWinVer.h"

#include <shobjidl.h>
#include <objbase.h>
#include <oleauto.h>
#include <olectl.h>
#include <ocidl.h>
#include <shlobj.h>
#include <Psapi.h>
#include <Sddl.h>

#include "Ini.h"

using namespace std;
using namespace boost;

System *System::_instance = NULL;

System *System::Instance()
{
  if (_instance == NULL)
  {
    _instance = new System;
  }
  return _instance;
}

bool System::WpagerApiHookAlreadInjected ( HANDLE hProcess, bool is64Bit)
{
  HMODULE hMods[1024];
  DWORD cbNeeded;

  std::wstring exeFolder=GetSys->GetExeFolder();

  if (is64Bit)
  {
    exeFolder.append(L"\\WindowsPagerApiHook64bit.dll");

  } else
  {
    exeFolder.append(L"\\WindowsPagerApiHook32bit.dll");
  }

#ifdef _M_X64
  if( EnumProcessModulesEx(hProcess, hMods, sizeof(hMods), &cbNeeded, is64Bit?LIST_MODULES_64BIT:LIST_MODULES_32BIT))
#else
  if( EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
#endif

  {
    for (UINT i = 0; i < (cbNeeded / sizeof(HMODULE)); i++ )
    {
      WCHAR szModName[MAX_PATH];
      if (GetModuleFileNameExW( hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(WCHAR)))
      {
        //tracew(L"\t%s (0x%08X)\n", szModName, hMods[i] );
        if (lstrcmpiW(szModName, exeFolder.c_str()) ==0)
        {
          trace("dll schon drin!!!!");
          return true;
        }
      }
    }
  }
  return false;
}


LONG_PTR System::GetWindowLongPTR(HWND hWnd, int nIndex)
{
  LogMethod;

  SetLastError(0);
  LONG_PTR res=GetWindowLongPtr(hWnd, nIndex);
  if (res==0)
  {
    if (GetLastError()!=ERROR_SUCCESS)
    {
      return -1;
    }
  }
  return res;
}

UINT System::GetVirtualKeyCodebyString(std::wstring key_value)
{
  LogMethod;

  if (key_value.length()==1 && key_value.compare(L"a") >= 0 && key_value.compare(L"z") <=0)
  {
    SHORT  code=VkKeyScanW(key_value.c_str()[0]);
    return LOBYTE(code);
  }

  if (key_value.length()==1 && key_value.compare(L"0") >= 0 && key_value.compare(L"9") <=0)
  {
    SHORT  code=VkKeyScanW(key_value.c_str()[0]);
    return LOBYTE(code);
  }

  if (key_value.compare(L"F1")==0) return VK_F1;
  if (key_value.compare(L"F2")==0) return VK_F2;
  if (key_value.compare(L"F3")==0) return VK_F3;
  if (key_value.compare(L"F4")==0) return VK_F4;
  if (key_value.compare(L"F5")==0) return VK_F5;
  if (key_value.compare(L"F6")==0) return VK_F6;
  if (key_value.compare(L"F7")==0) return VK_F7;
  if (key_value.compare(L"F8")==0) return VK_F8;
  if (key_value.compare(L"F9")==0) return VK_F9;
  if (key_value.compare(L"F10")==0) return VK_F10;
  if (key_value.compare(L"F11")==0) return VK_F11;
  if (key_value.compare(L"F12")==0) return VK_F12;

  if (key_value.compare(L"numpad0")==0) return VK_NUMPAD0;
  if (key_value.compare(L"numpad1")==0) return VK_NUMPAD1;
  if (key_value.compare(L"numpad2")==0) return VK_NUMPAD2;
  if (key_value.compare(L"numpad3")==0) return VK_NUMPAD3;
  if (key_value.compare(L"numpad4")==0) return VK_NUMPAD4;
  if (key_value.compare(L"numpad5")==0) return VK_NUMPAD5;
  if (key_value.compare(L"numpad6")==0) return VK_NUMPAD6;
  if (key_value.compare(L"numpad7")==0) return VK_NUMPAD7;
  if (key_value.compare(L"numpad8")==0) return VK_NUMPAD8;
  if (key_value.compare(L"numpad9")==0) return VK_NUMPAD9;

  if (key_value.compare(0, 2, L"0x")!=0)
  {
    WCHAR buf[256];
    swprintf_s(buf, 256, L"Invalid value '%s' in 'windowspager.ini'."\
      L"\nPlease repair the 'windowspager.ini' file. Will quit now.", 
      key_value.c_str());
    MessageBoxW(0, buf, L"WindowsPager", MB_ICONHAND);
    GetPager->ResetListwindow();
    exit(1);
  }

  UINT v =(UINT) wcstol(key_value.c_str(), NULL, 16); 
  if (v==0)
  {
      WCHAR buf[256];
      swprintf_s(buf, 256, L"Invalid value '%s' in 'windowspager.ini'."\
        L"\nPlease repair the 'windowspager.ini' file. Will quit now.", 
        key_value.c_str());
      MessageBoxW(0, buf, L"WindowsPager", MB_ICONHAND);
      GetPager->ResetListwindow();
      exit(1);
  }

  return v;
}

bool System::GotoURL(string url)
{
  LogMethod;

  if (ShellExecutePointer==NULL)
  {
    return false;
  }

  int result=(int) ShellExecutePointer(NULL, "open", url.c_str(), NULL,NULL, SW_SHOWNORMAL);
  if (result<=32)
  {
    return FALSE;
  } else
  {
    return TRUE;
  }
}

DWORD WINAPI System::SetWindowLongPtrTimeoutThread(LPVOID lpParam)
{
  LogMethod;

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

bool System::SetWindowLongPtrTimeout(HWND hWnd, int nIndex, 
                                     LONG_PTR dwNewLong, int timeout)
{
  LogMethod;

  set_window_long_data  *my_data = (set_window_long_data *) new set_window_long_data;
  my_data->hWnd = hWnd;
  my_data->nIndex = nIndex;
  my_data->dwNewLong = dwNewLong;
  
  shared_ptr<void> threadHandle (CreateThread(0, 0, System::SetWindowLongPtrTimeoutThread, 
    my_data, 0, NULL), CloseHandle);
  
  if (WaitForSingleObject(threadHandle.get(), timeout) == WAIT_TIMEOUT)
  {
    return false;
  }

  return true;
}

DWORD WINAPI System::DialogThread(LPVOID lpParam)
{
  LogMethod;

  dialog_data *data = (dialog_data *) lpParam;

  MessageBox(0, (LPCSTR) data->text->c_str(), "WindowsPager", data->icon);
  delete data->text;
  delete data;
  return 0;
}

void System::ShowMessageBoxModless(string text, UINT icon)
{
  LogMethod;

 
  dialog_data *data = (dialog_data *) new dialog_data;

  if (text.empty())
  {
    data->text = new string("No Text to display!");
  } else
  {
    data->text = new string(text);
  }

  data->icon=icon;
 
  shared_ptr<void> threadHandle(CreateThread(0, 0, System::DialogThread, (LPVOID) data,
    0, NULL), CloseHandle);
}

HWND System::GetTaskButtonWindow(void)
{
  LogMethod;

  return taskbutton_window;
}

bool System::GetDirectoryFromExe(std::wstring& currentDirectory)
{
 LogMethod;

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

std::wstring System::GetExeFolder(void)
{
  LogMethod;

  return currentFolder;
}

BOOL System::InternalIs64BitWindows(void)
{
  #if defined(_WIN64)
   return TRUE;  // 64-bit programs run only on Win64
  #elif defined(_WIN32)
   // 32-bit programs run on both 32-bit and 64-bit Windows
   // so must sniff
   BOOL f64 = FALSE;

   if (IsWow64ProcessPointer==NULL)
   {
     return false;
   }

   return IsWow64ProcessPointer(GetCurrentProcess(), &f64) && f64;
  #else
   return FALSE; // Win64 does not support Win16
  #endif
}


void System::Init(void)
{
  LogMethod;

  debugmode = IsDebuggerPresent();

  if (!GetDirectoryFromExe(currentFolder))
  {
    MessageBox(NULL, "WindowsPager", "System::Init() GetDirectoryFromExe(currentFolder) failed", MB_OK);
    return;
  }
  
  IsWow64ProcessPointer=NULL;
  HMODULE hLibKernel32 = ::LoadLibrary("kernel32");
  if (hLibKernel32 != NULL)
  {
    IsWow64ProcessPointer= (pIsWow64Process)::GetProcAddress(hLibKernel32, "IsWow64Process");
    FreeLibrary(hLibKernel32);
  }


  //trace("currentdirectory: %s\n", currentFolder.c_str());

  panel_window = FindWindow("Shell_TrayWnd", NULL);

  taskbarEdge=ABE_BOTTOM;
  taskbarHorizontal=true;
    
  APPBARDATA abd;
  abd.cbSize=sizeof(APPBARDATA);
  abd.hWnd=panel_window;
    
  bool res=(bool) SHAppBarMessage(ABM_GETTASKBARPOS, &abd); 
  if (res==true)
  {
    taskbarEdge=abd.uEdge;
    if (taskbarEdge==ABE_LEFT || taskbarEdge==ABE_RIGHT)
    {
      taskbarHorizontal=false;
    }
  }
 
  IniFile = new CIniReader();
  desktop_window=GetDesktopWindow();

#ifdef _DEBUG
  IniFile->SetINIFileName (L"C:\\ini\\windowspager.ini"); 
#else
  std::wstring path=currentFolder+L"\\..\\windowspager.ini";
  IniFile->SetINIFileName (path.c_str()); 
#endif
  
  multiMonitorMode=SINGLE_MONITOR;
  int m = GetSystemMetrics(SM_CMONITORS);
  if (m>=2)
  {
    wstring tmp_string=GetSys->GetIniFile()->GetKeyValue(L"MultiMonitor", L"MultiMonitorMode");
    if (tmp_string.compare(L"CombineToOneBigScreen")==0)
    {
      trace("use one big screen\n");
      multiMonitorMode=ONE_BIG_SCREEN;
    } else if( tmp_string.compare(L"PagerOnlyOnPrimaryScreen")==0)
    {
      trace("pager on primary\n");
      multiMonitorMode=PAGER_ON_PRIMARY;
    } else if ( tmp_string.compare(L"ForceSingleMonitor")==0)
    {
      trace("single monitor\n");
      multiMonitorMode=SINGLE_MONITOR;
    }   
  }

  int panel_height=0;
  int panel_width=0;
    
  if (taskbarHorizontal)
  {
    RECT panel_rect;
    if (GetWindowRect(panel_window, &panel_rect))
    {
      panel_height =((panel_rect.bottom - panel_rect.top) / 10) * 10;
    }
    trace("panel_height system init: %d\n", panel_height);
  } else
  {
    RECT panel_rect;
    if (GetWindowRect(panel_window, &panel_rect))
    {
      panel_width =panel_rect.right - panel_rect.left;
    }
    trace("panel_width: %d\n", panel_width);
  }

  if (multiMonitorMode==SINGLE_MONITOR || multiMonitorMode==PAGER_ON_PRIMARY)
  {
   

    screen_width= GetSystemMetrics(SM_CXSCREEN) - panel_width;
    screen_height = GetSystemMetrics(SM_CYSCREEN)- panel_height;
  } else
  {
    screen_width= GetSystemMetrics(SM_CXVIRTUALSCREEN);
    screen_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  }

  trace("fullscreen widht: %d, fullscreen height: %d\n", screen_width, screen_height);

  SetRect(&fullscreen_rect, 0, 0, screen_width, screen_height);
 
  primary_width=GetSystemMetrics(SM_CXSCREEN);
  primary_height=GetSystemMetrics(SM_CYSCREEN);

  SetRect(&primary_rect, 0, 0, primary_width, primary_height);

  if (multiMonitorMode==ONE_BIG_SCREEN)
  {
    virtual_left=GetSystemMetrics(SM_XVIRTUALSCREEN);
    trace("left : %d\n", virtual_left);
    virtual_top=GetSystemMetrics(SM_YVIRTUALSCREEN);
    trace("top : %d\n", virtual_top);
  } else
  {
    virtual_left=0; virtual_top=0;
  }

  SetRect(&hidden_rect, virtual_left+15000, 0, virtual_left+15000+screen_width,
    screen_height);

  SetProp(panel_window, "PAGER_HIDDEN_RECT_VIRTUAL_LEFT", (void*) virtual_left);
  SetProp(panel_window, "PAGER_HIDDEN_RECT_SCREEN_WIDTH", (void*) screen_width);
  SetProp(panel_window, "PAGER_HIDDEN_RECT_SCREEN_HEIGHT", (void*) screen_height);
       

  xp_theme=System::_GetXPTheme();

  brush_framecolor = CreateSolidBrush(RGB(0, 0, 0));
  brush_red = CreateSolidBrush(RGB(255, 0, 0));
  brush_white = CreateSolidBrush(RGB(255, 255, 255));
  brush_black = CreateSolidBrush(RGB(0, 0, 0));
  pen_framecolor = CreatePen(PS_SOLID, 0, RGB(0, 0, 0));
  pen_null = CreatePen(PS_NULL, 0, RGB(34, 185, 229));
 
  progman_window=FindWindow("Progman", NULL);

  char cmd_buf[14];
  sprintf_s (cmd_buf, 14, "%s%s", "ReBar","Window32");
  list_window = FindWindowEx((HWND) panel_window, NULL, cmd_buf, NULL);

 

  taskbutton_window=FindWindowEx((HWND) list_window, NULL, "MSTaskSwWClass", NULL);

  trace("mstaskswclass: %x\n", taskbutton_window);
    

  tray_window = FindWindowEx((HWND) panel_window, NULL, "TrayNotifyWnd", NULL);
 

  border_width=GetSystemMetrics(SM_CXFRAME);
  caption_height=GetSystemMetrics(SM_CYCAPTION)+border_width;

  trace("border withd: %d\n", border_width);
  trace("capton height: %d\n", caption_height);
  

  default_wallpaper.clear();
  char buffer[MAX_PATH];
  SystemParametersInfo(SPI_GETDESKWALLPAPER, 260, buffer, 0);
  if (strlen(buffer) > 0)
  {
    default_wallpaper=string(buffer);
  } 
  trace("default wallpaper: %s\n", default_wallpaper.c_str());

  own_sid= GetOwnSID();

  CoCreateInstance(CLSID_TaskbarList, 0,
    CLSCTX_INPROC_SERVER, IID_ITaskbarList, (void **) &pTaskbar);
  pTaskbar->HrInit();
 
  is64bit=InternalIs64BitWindows();

  ShellExecutePointer=NULL;
  HMODULE hLib2 = ::LoadLibrary("shell32.dll");
  if (hLib2 != NULL)
  {
    ShellExecutePointer= (pShellExecute)::GetProcAddress(hLib2, "ShellExecuteA");
    FreeLibrary(hLib2);
  }

  System::Update();
}

bool System::Is64BitProcess(HANDLE hProcess)
{
  if (!is64bit)
  {
    return false;
  }

  if (IsWow64ProcessPointer==NULL)
  {
    return false;
  }

  BOOL bIsWow64 =false;
  if (!IsWow64ProcessPointer(hProcess, &bIsWow64))
  {
    return false;
  }

  return !bIsWow64; //no joke !
}

void System::Update(void)
{
  LogMethod;

  APPBARDATA abd;
  abd.cbSize=sizeof(APPBARDATA);
  abd.hWnd=panel_window;
    
  bool res=(bool) SHAppBarMessage(ABM_GETTASKBARPOS, &abd); 
  taskbarHorizontal=true;
  if (res==true)
  {
    taskbarEdge=abd.uEdge;
    if (taskbarEdge==ABE_LEFT || taskbarEdge==ABE_RIGHT)
    {
      taskbarHorizontal=false;
    }
  }

  RECT tray_window_screen;
  GetWindowRect(tray_window, &tray_window_screen);
  RECT list_window_screen;
  GetWindowRect(list_window, &list_window_screen);

  GetWindowRect(panel_window, &panel_window_rect);

  if (taskbarHorizontal)
  {
    int panel_height = (((panel_window_rect.bottom -
       panel_window_rect.top)) / 10) * 10;
    if (multiMonitorMode==SINGLE_MONITOR ||
        multiMonitorMode==PAGER_ON_PRIMARY)
    {
      int screen_height_tmp=GetSystemMetrics(SM_CYSCREEN)-panel_height;
      if (screen_height  > 0)
      {
        screen_height = screen_height_tmp;
      }
    }
  } else
  {
     int panel_width = panel_window_rect.right -
       panel_window_rect.left;
    if (multiMonitorMode==SINGLE_MONITOR ||
        multiMonitorMode==PAGER_ON_PRIMARY)
    {
      int screen_width_tmp = GetSystemMetrics(SM_CXSCREEN)-panel_width;
      if (screen_width_tmp > 0)
      {
        screen_width = screen_width_tmp;
      }
    }

  }

  SetRect(&tray_window_rect,
    tray_window_screen.left - panel_window_rect.left,
    tray_window_screen.top - panel_window_rect.top,
    tray_window_screen.right - panel_window_rect.left,
    tray_window_screen.bottom - panel_window_rect.top);

  SetRect(&list_window_rect,
    list_window_screen.left - panel_window_rect.left,
    list_window_screen.top - panel_window_rect.top,
    list_window_screen.right - panel_window_rect.left,
    list_window_screen.bottom - panel_window_rect.top);
}

System::~System()
{
  LogMethod;

  DeleteObject(brush_white);
  DeleteObject(brush_framecolor);
  DeleteObject(pen_framecolor);
  DeleteObject(brush_black);
  DeleteObject(brush_red);
  DeleteObject(pen_null);

  LocalFree(own_sid);

  pTaskbar->Release();

  delete IniFile;
}

int System::_GetXPTheme()
{
  LogMethod;

  HKEY hKey1;
  int theme=0;

  long lResult = RegOpenKeyEx(HKEY_CURRENT_USER, 
    "Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager", 0, KEY_ALL_ACCESS, &hKey1);
  if (lResult == ERROR_SUCCESS)
  {
    char buf0[256];
    long lResult0 = 0;
    DWORD dwValueLen1 = 256;
    lResult0 = RegQueryValueEx(hKey1, "ThemeActive", 0, REG_NONE, (LPBYTE) buf0,
      &dwValueLen1);

    if (lResult0 == ERROR_SUCCESS)
    {
      if (lstrcmpi(buf0, "0") == 0)
      {
        theme=4;
        RegCloseKey(hKey1);
        return theme;
      }
    }

    long lResult1 = 0;
    char buf[256];
    DWORD dwValueLen2 = 256;
    lResult1 = RegQueryValueEx(hKey1, "ColorName", 0, REG_NONE, (LPBYTE) buf,
      &dwValueLen2);

    if (lResult1 == ERROR_SUCCESS)
    {
      if (lstrcmpi(buf, "NormalColor") == 0)
      {
        theme=1;
      }

      if (lstrcmpi(buf, "HomeStead") == 0)
      {
        theme=2;
      }

      if (lstrcmpi(buf, "Metallic") == 0)
      {
        theme=3;
      }
    }
  }

  RegCloseKey(hKey1);
  return theme;
}

wstring  *System::GetWindowFileName(HWND hWnd)
{
  LogMethod;

  DWORD pId;
  DWORD res;
  wstring *filename=NULL;
  WCHAR lpFileName[MAX_PATH];

  if (!GetWindowThreadProcessId(hWnd, &pId))
  {
    return NULL;
  }

  shared_ptr<void> hProcess (OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ , FALSE, pId),
                             CloseHandle);
  if (hProcess.get()==NULL)
  {
    return NULL;
  }

  res = GetModuleFileNameExW(hProcess.get(), NULL, lpFileName, MAX_PATH);
  if (res!=0)
  {
    filename= new wstring(lpFileName);
  } 

  return filename;
}

LPSTR System::GetSIDbyProcess(HANDLE process)
{
  LogMethod;
  
  HANDLE token;
  DWORD len;

  if (!OpenProcessToken (process, TOKEN_QUERY, &token))
  {
    trace("error open token\n");
    return NULL;
  }

  GetTokenInformation(token, TokenUser, NULL, 0, &len);

  LPBYTE data = new BYTE[len];
  if (!GetTokenInformation(token, TokenUser, data, len, &len))
  { 
    CloseHandle(token);
    return NULL;
  }

  PSID SID = ((PTOKEN_USER)data)->User.Sid; 
  LPSTR pSid;

  if (!ConvertSidToStringSid(SID, &pSid))
  {
    CloseHandle(token);
    delete [] data;
    return NULL;
  }

  delete [] data;
  CloseHandle(token);
  // trace("sid: %s\n", pSid);
  return pSid;
}

LPSTR System::GetOwnSID(void)
{
  LogMethod;

  HANDLE handle = GetCurrentProcess();
  return GetSIDbyProcess(handle);
}

bool System::WindowIsOwnedbyMe(HWND window)
{
  LogMethod;


  DWORD pId;
  GetWindowThreadProcessId(window, &pId);
  DWORD access=(GetWinVersion->IsVista() || GetWinVersion->IsSeven())?PROCESS_QUERY_LIMITED_INFORMATION:
                                                              PROCESS_QUERY_INFORMATION;

  shared_ptr<void> hProcess (OpenProcess(access, FALSE, pId), CloseHandle);
  if (hProcess.get()==NULL)
  {
    trace("open process failed\n");
    return FALSE;
  }

  shared_ptr<char> process_sid(System::GetSIDbyProcess(hProcess.get()), LocalFree);
  if (process_sid.get()==NULL)
  {
    return FALSE;
  }

  if (strcmp(process_sid.get(), own_sid)==0)
  {
    return TRUE;
  } else
  {
    return FALSE;
  }

  return true;
}

HICON System::GetIconFromWindowHWND(HWND window)
{
  LogMethod;

  HICON hIcon = NULL;
  hIcon = (HICON) GetClassLongPtr(window, GCLP_HICON);
  if (hIcon == NULL)
  {
    hIcon = (HICON) SendMessage(window, WM_GETICON, ICON_BIG, 0);
  }
  if (hIcon == NULL)
  {
    hIcon = (HICON) SendMessage(window, WM_GETICON, ICON_SMALL, 0);
  }
  if (hIcon == NULL)
  {
    hIcon = (HICON) GetClassLongPtr(window, GCLP_HICONSM);
  }
  if (hIcon)
  {
    return CopyIcon(hIcon);
  }
  return LoadIcon(NULL, IDI_APPLICATION);
}

void System::GetMousePos(int &x, int &y)
{
  LogMethod;

  POINT pt;
  GetCursorPos(&pt); 
  trace ("mouse pos: %d, %d\n", pt.x, pt.y);
  x=pt.x; y=pt.y;
}

HWND System::GetRootWindow(void)
{
  LogMethod;

  return desktop_window;
}

void System::RectToGDIPlusRect(Gdiplus::Rect *rect, RECT _rect)
{
  LogMethod;

  rect->X=_rect.left;
  rect->Y=_rect.top;
  rect->Width=_rect.right-_rect.left;
  rect->Height=_rect.bottom-_rect.top;
}

bool System::Is64BitWindows(void)
{
  LogMethod;

  return is64bit;
}

RECT System::GetPrimaryRect(void)
{
  LogMethod;
  return primary_rect;
}

int System::GetPrimaryWidth(void)
{
  LogMethod;

  return primary_width;
}

int System::GetPrimaryHeight(void)
{
  LogMethod;

  return primary_height;
}

int System::GetMultiMonitorMode(void)
{
  LogMethod;

  return multiMonitorMode;
}

CIniReader* System::GetIniFile (void)
{
  LogMethod;

  return IniFile;
}

RECT System::RealToVirtualScreenRect (RECT real)
{
  LogMethod;

  RECT virt;

  virt.left=real.left-virtual_left;
  virt.top=real.top-virtual_top;

  virt.right=real.right-virtual_left;
  virt.bottom=real.bottom-virtual_top;

  return virt;
}

void System::VirtualToRealScreenXY (int &x, int &y)
{
  LogMethod;

  x=x+virtual_left;
  y=y+virtual_top;
}

int System::GetMonitor(HWND window)
{
  LogMethod;

  HMONITOR monitor=MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
  if (monitor==NULL)
  {
    return 0;
  }

  MONITORINFO info;
  info.cbSize=sizeof(MONITORINFO);
  if (!GetMonitorInfo(monitor, &info))
  {
    return 0;
  }

  if (info.dwFlags==MONITORINFOF_PRIMARY)
  {
    return PRIMARY_MONITOR;
  } else
  {
    return SECONDARY_MONITOR;
  }
}


RECT System::GetHiddenRect(void)
{
  LogMethod;

  return hidden_rect;
}

bool System::WindowIsVisibleOnAMonitor(HWND window)
{
  LogMethod;

  HMONITOR moni=MonitorFromWindow(window, MONITOR_DEFAULTTONULL);
  if (!moni)
  { 
    return FALSE;
  }

  RECT rect;
  if (IsIconic(window))
  {
    trace("is iconic: %x\n", window);
    WINDOWPLACEMENT placement;
    placement.length = sizeof(placement);

    if(!GetWindowPlacement(window, &placement))
    {
      return FALSE;
    }
    CopyRect(&rect, &placement.rcNormalPosition);
  } else
  {
    if (!GetWindowRect(window, &rect))
    {
      return FALSE;
    }
  }

  // trace("visiblie on monitor rect (window: %x): left: %d, top:%d, right: %d, bottom: %d\n",
  //   window, rect.left, rect.top, rect.right, rect.bottom);

  if (abs(rect.left-rect.right)<=1 && abs(rect.top - rect.bottom)<=1)
  {
    trace("but window too small\n");
    return FALSE;
  }

  return TRUE;
}

std::string System::GetDefaultWallpaper(void)
{
  LogMethod;

  return default_wallpaper;
}

bool System::WindowIsHungTimeoutCheck(HWND inhHWnd, UINT waitTime)
{
  LogMethod;

  if (SendMessageTimeout(inhHWnd,WM_NULL, 0x51842145, 0x5e7bdeba,
    SMTO_ABORTIFHUNG|SMTO_BLOCK, waitTime,NULL) == 0)
  {
    return true;
  }  else
  {
    return false;
  }
}

bool System::CheckIfWindowDoRespond(HWND inhHWnd, UINT waitTime)
{
  LogMethod;

  if (IsHungAppWindow(inhHWnd))
  {
    return true;
  }

  //from virtuawin code
  if (SendMessageTimeout(inhHWnd,WM_NULL, 0x51842145, 0x5e7bdeba,
    SMTO_ABORTIFHUNG|SMTO_BLOCK, waitTime,NULL) == 0)
  {
    return true;
  }  else
  {
    return false;
  }
}

bool System::CheckIfFileExists(std::wstring strFilename) 
{
  LogMethod;

  struct _stat stFileInfo;
  bool blnReturn;
  int intStat;

  intStat = _wstat(strFilename.c_str(),&stFileInfo);
  if(intStat == 0)
  {
    blnReturn = true;
  } else
  {
    blnReturn = false;
  }

  return(blnReturn);
}

int System::GetXPTheme(void)
{
  LogMethod;

  return xp_theme;
}

int System::GetCaptionHeight(void)
{
  LogMethod;

  return caption_height;
}

int System::GetBorderWidth(void)
{
  LogMethod;

  return border_width;
}

void System::ShowTaskButton(HWND window)
{
  LogMethod;

  pTaskbar->AddTab(window);
}

void System::HideTaskButton(HWND window)
{
  LogMethod;

  pTaskbar->DeleteTab(window);
}

bool System::PointIsInsideRect(RECT rect, POINT pt)
{
  LogMethod;

  rect.bottom++;
  rect.right++;
  return (bool) PtInRect(&rect, pt);
}

HWND System::GetPanelWindow(void)
{
  LogMethod;

  return panel_window;
}

HWND System::GetListWindow(void)
{
  LogMethod;

  return list_window;
}

HWND System::GetTrayWindow(void)
{
  LogMethod;

  return tray_window;
}

RECT System::GetTrayWindowRect(void)
{
  LogMethod;

  return tray_window_rect;
}

RECT System::GetListWindowRect(void)
{
  LogMethod;

  return list_window_rect;
}

RECT System::GetPanelWindowRect(void)
{
  LogMethod;

  return panel_window_rect;
}

int System::GetScreenHeight(void)
{
  LogMethod;

  return screen_height;
}

void System::SetScreenHeight(int height)
{
  LogMethod;

  screen_height=height;
}

int System::GetScreenWidth(void)
{
  LogMethod;

  return screen_width;
}

RECT System::GetFullScreenRect(void)
{
  LogMethod;

  return fullscreen_rect;
}

int System::Debuging(void)
{
  LogMethod;

  return debugmode;
}

int System::Round(double a)
{
  LogMethod;
  return int (a + 0.5);
}