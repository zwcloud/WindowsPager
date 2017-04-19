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

#include "INI.h"
#include "System.h"
#include "Pager.h"


using namespace std;
using namespace boost;

CIniReader::CIniReader()
{
  strFileName.clear();
}

//from http://oopweb.com/CPP/Documents/CPPHOWTO/Volume/C++Programming-HOWTO-7.html
void CIniReader::StringSplit(const wstring &str,
                             vector<wstring> &tokens, const wstring& delimiters)
{
  LogMethod;

  // Skip delimiters at beginning.
  wstring::size_type lastPos = str.find_first_not_of(delimiters, 0);
  // Find first "non-delimiter".
  wstring::size_type pos     = str.find_first_of(delimiters, lastPos);

  while (wstring::npos != pos || wstring::npos != lastPos)
  {
    // Found a token, add it to the vector.
    tokens.push_back(str.substr(lastPos, pos - lastPos));
    // Skip delimiters.  Note the "not_of"
    lastPos = str.find_first_not_of(delimiters, pos);
    // Find next "non-delimiter"
    pos = str.find_first_of(delimiters, lastPos);
  }
}

int CIniReader::GetIgnoreWindowsCount(void)  
{
  LogMethod;

  const int nSize=10050;
  WCHAR ac_Result[nSize];

  DWORD anzahl=GetPrivateProfileSectionW(L"IgnoreWindows", ac_Result, nSize, strFileName.c_str());
  trace("anzal: %d\n", anzahl);
  if (anzahl==nSize-2)
  {
    return -1;
  }

  if (anzahl==0)
  {
    return 0;
  }

  int i=anzahl-2;
  trace("last: %c\n", ac_Result[i]);
  while (i > 0 && ac_Result[i]!='\0')
  {
    --i;
  }
  trace("i: %d\n", i);

  wstring last=wstring(&ac_Result[i+1]);

  int under_line=last.find_first_of(L"_");
  if (under_line==wstring::npos)
  {
    return -1;
  }

  int equal=last.find_first_of(L"=", under_line+1);
  if (equal==wstring::npos)
  {
    return -1;
  }
  wstring number=last.substr(under_line+1, equal-1-under_line);
  if (number.empty())
  {
    return -1;
  }

  try
  {
    return lexical_cast<int>(number);
  }
  catch(bad_lexical_cast &)
  {
    return -1;
  }

  return -1;
}

bool CIniReader::GetIgnoreWindowsEntries(vector<wstring>& list)  
{
  LogMethod;

  const int nSize=10050;
  WCHAR ac_Result[nSize];  //change size depending on needs

  DWORD anzahl=GetPrivateProfileSectionW(L"IgnoreWindows", ac_Result, nSize, strFileName.c_str());

  trace("anzal: %d\n", anzahl);
  if (anzahl==nSize-2 || anzahl==0)
  {
    return false;
  }

  list.clear();

  wstring strSectionData(L"");
  for(DWORD i=0; i<anzahl; i++)
  {
    if(ac_Result[i] != '\0') 
    {
      strSectionData = strSectionData + ac_Result[i];
    } else 
    {
      if(!strSectionData.empty()) 
      {
        list.push_back(strSectionData);
      }
      strSectionData.clear();
    }
  }

  return true;
}

bool CIniReader::AddToIgnoreWindowList(int line, const wstring& value)
{
  LogMethod;

  wstringstream s;
  s << line;
  wstring key=L"ignore_"+s.str();

  return WritePrivateProfileStringW (L"IgnoreWindows", key.c_str(), value.c_str(),
    strFileName.c_str());
}

UINT CIniReader::GetHotkeyModifier(wstring section)
{
  LogMethod;

  wstring modifier_string=CIniReader::GetKeyValue(section.c_str(), L"modifier");
  if (modifier_string.compare(L"error")==0)
  {
    char buf[256];
    sprintf_s(buf, 256, "Error reading key: '%s' in section '%s' in 'windowspager.ini'."\
      "\nPlease repair the 'windowspager.ini' file. Will quit now.", "modifier", "hotkeys");
    MessageBox(0, buf, "WindowsPager", MB_ICONHAND);
    GetPager->ResetListwindow();
    exit(1);
  }

  UINT modifier_id=0;
  if (modifier_string.find(L"control") != wstring::npos)
  {
    modifier_id=MOD_CONTROL;
  }

  if (modifier_string.find(L"alt") != wstring::npos)
  {
    modifier_id|=MOD_ALT;
  }

  if (modifier_string.find(L"win") != wstring::npos)
  {
    modifier_id|=MOD_WIN;
  }

  if (modifier_string.find(L"shift") != wstring::npos)
  {
    modifier_id|=MOD_SHIFT;
  }

  if (modifier_id==0)
  {
    MessageBox(0, "No valid value for key 'modifier' in 'windowspager.ini'. "\
      "\nPlease repair the 'windowspager.ini'. Will quit now", "WindowsPager", MB_ICONHAND);   
    GetPager->ResetListwindow();
    exit(1);
  }

  return modifier_id;
}

UINT CIniReader::GetHotkeyScrolling(wstring scroll_key)
{
  LogMethod;

  wstring key_value=CIniReader::GetKeyValue(L"HotkeysDesktopScrolling", scroll_key);
  if (key_value.compare(L"error")==0)
  {
    return -1;
  }

  UINT v=GetSys->GetVirtualKeyCodebyString(key_value);
  return v;
}

UINT CIniReader::GetHotkeyMoveToDesktop(wstring scroll_key)
{
  LogMethod;

  wstring key_value=CIniReader::GetKeyValue(L"HotkeysMoveToDesktop", scroll_key);
  if (key_value.compare(L"error")==0)
  {
    return -1;
  }

  UINT v=GetSys->GetVirtualKeyCodebyString(key_value);

  return v;
}

UINT CIniReader::GetHotkeyKey(int number, std::wstring section, wstring desk_key)
{
  LogMethod;

  wstring key_value=CIniReader::GetKeyValue(section, desk_key);
  if (key_value.compare(L"error")==0)
  {
    if (number > 9)
    { 
      return -1;
    } else
    {
      return VK_NUMPAD0+number;
    }
  }

  UINT v=GetSys->GetVirtualKeyCodebyString(key_value);
  return v;

}

wstring CIniReader::GetKeyValue(wstring strSection, wstring strKey)
{
  LogMethod;

  WCHAR ac_Result[256];
  long m_lRetValue = GetPrivateProfileStringW(strSection.c_str(),strKey.c_str(),
    L"error", ac_Result, 255, strFileName.c_str());	
  wstring strResult(ac_Result);

  return strResult;
}

int CIniReader::GetKeyValueInt(wstring strSection, wstring strKey)
{
  LogMethod;

  int res=GetPrivateProfileIntW(strSection.c_str(), strKey.c_str(), -1,
    strFileName.c_str());
  return res;        
}

void CIniReader::SetINIFileName(wstring strINIFile)
{
  LogMethod;

  if (!GetSys->CheckIfFileExists(strINIFile))
  {
    MessageBox(0, "Missing 'windowspager.ini' file! \nPlease make sure this file is"\
      " in the same directory as 'windowspager.exe'. Will quit now.",
      "WindowsPager", MB_ICONHAND);
    exit(1);
  }
  strFileName = strINIFile;
}

