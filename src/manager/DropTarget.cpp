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

#include <ole2.h> 
#include "DropTarget.h"
#include "Pager.h"
#include "System.h"

DropTarget::DropTarget(void)
{
  LogMethod;
  m_refs=1;
}

HRESULT __stdcall DropTarget::QueryInterface (REFIID iid, void ** ppv)
{
  LogMethod;
  if(iid == IID_IUnknown || iid == IID_IDropTarget)
  {
    *ppv = this;
    AddRef();
    return NOERROR;
  }
  *ppv = NULL;
  return ResultFromScode(E_NOINTERFACE);
}

ULONG __stdcall DropTarget::AddRef(void)
{
  LogMethod;
  return ++m_refs;
}	

ULONG __stdcall DropTarget::Release(void)
{
  LogMethod;
   trace("release: m_ref: %d\n", m_refs);

  if(--m_refs == 0)
  {
    delete this;
    return 0;
  }
  return m_refs;
}

HRESULT __stdcall DropTarget::DragEnter(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
  LogMethod;
  *pdwEffect = DROPEFFECT_LINK;
  return NOERROR;
}

int CALLBACK DropTarget::HoverTimer(HWND hwnd, UINT message, UINT_PTR idTimer, DWORD dwTime)
{
  LogMethod;

  KillTimer(GetPager->GetPagerWindowHWND(), idTimer);

  if (GetPager->GetDragTimerDesktop()!=-1)
  {

    GetPager->SwitchToDesktop(GetPager->GetDesktopList()[GetPager->GetDragTimerDesktop()]);
  }

  GetPager->SetDragTimerDesktop(-1);

  return 1;
}

HRESULT __stdcall DropTarget::DragOver(DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
  LogMethod;

  ScreenToClient(GetPager->GetPagerWindowHWND(), (POINT *)&pt);
  //	trace ("pt.x: %d , pt.y: %d\n", pt.x, pt.y);

  POINT p;
  p.x=pt.x;
  p.y=pt.y;

  int timer_set_on_desk=GetPager->GetDragTimerDesktop();
  Desktop *new_desk=GetPager->GetDesktopFromPoint(p);
  int move_on_desk=new_desk->GetDesktopNumber();
  if (GetPager->CurrentDesktop()->GetDesktopNumber()==move_on_desk)
  {
    if (timer_set_on_desk!=-1)
    {
      KillTimer(GetPager->GetPagerWindowHWND(), TIMER_ID_DND);
      GetPager->SetDragTimerDesktop(-1);
    }
    return NOERROR;
  }

  if (timer_set_on_desk==move_on_desk)
  {
    return NOERROR;
  }

  if (timer_set_on_desk==-1)
  {
    SetTimer(GetPager->GetPagerWindowHWND(), TIMER_ID_DND, 500, (TIMERPROC) DropTarget::HoverTimer);
    GetPager->SetDragTimerDesktop(move_on_desk);
    return NOERROR;
  }

  KillTimer(GetPager->GetPagerWindowHWND(), TIMER_ID_DND);
  GetPager->SetDragTimerDesktop(-1);

  *pdwEffect = DROPEFFECT_LINK;

  return NOERROR;
}

HRESULT __stdcall DropTarget::DragLeave(void)
{
  LogMethod;

  KillTimer(GetPager->GetPagerWindowHWND(), TIMER_ID_DND);
  GetPager->SetDragTimerDesktop(-1);
  return NOERROR;
}

HRESULT __stdcall DropTarget::Drop(IDataObject * pDataObject, DWORD grfKeyState, POINTL pt, DWORD * pdwEffect)
{
  LogMethod;

  GetSys->ShowMessageBoxModless("You can not drop objects here. Hold the mouse for a short time to switch to the desktop you want and drop the object to the real desktop or application.", MB_ICONWARNING);

  return NOERROR;   
}