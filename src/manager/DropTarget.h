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
#include <ole2.h> 

class DropTarget: public IDropTarget
{
public:
  // IUnknown implementation
  HRESULT __stdcall QueryInterface (REFIID iid, void ** ppvObject);
  ULONG	__stdcall AddRef (void);
  ULONG	__stdcall Release (void);

  // IDropTarget implementation
  HRESULT __stdcall DragEnter (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
  HRESULT __stdcall DragOver (DWORD grfKeyState, POINTL pt, DWORD * pdwEffect);
  HRESULT __stdcall DragLeave (void);
  HRESULT __stdcall Drop (IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect); 
  static int CALLBACK HoverTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime);
  DropTarget();

private:
  ULONG	m_refs;
};
