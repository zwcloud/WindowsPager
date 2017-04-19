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

#pragma once

#include "..\common\stdafx.h"

class CIniReader  
{
public:
  CIniReader();
  ~CIniReader() {}
  int GetIgnoreWindowsCount(void);
  bool GetIgnoreWindowsEntries(std::vector<std::wstring>& list);
  bool AddToIgnoreWindowList(int line, const std::wstring& value);
  void StringSplit(const std::wstring &str, std::vector<std::wstring> &tokens, const std::wstring& delimiters);
  UINT GetHotkeyScrolling(std::wstring scroll_key);
  UINT GetHotkeyMoveToDesktop(std::wstring scroll_key);
  UINT GetHotkeyModifier(std::wstring section);
  UINT GetHotkeyKey(int number, std::wstring section, std::wstring desk_key);
  void SetINIFileName(std::wstring strINIFile);
  std::wstring GetKeyValue(std::wstring strSection, std::wstring strKey);
  int GetKeyValueInt(std::wstring strSection, std::wstring strKey);

private:
  std::wstring strFileName;
};
