#include "StdAfx.h"
#include "TranparentWnd.h"
#include <process.h>


//////////////////////////////////////////////////////////////////////////
///

DUI_BEGIN_MESSAGE_MAP(CTranparentWnd, WindowImplBase)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK,OnClick)
DUI_END_MESSAGE_MAP()

CTranparentWnd::CTranparentWnd(void) : 
	m_pCef(NULL)
{
}

CTranparentWnd::~CTranparentWnd(void)
{
}

void CTranparentWnd::OnFinalMessage( HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CTranparentWnd::GetSkinFile()
{
	return _T("transparent.xml");
}

LPCTSTR CTranparentWnd::GetWindowClassName( void ) const
{
	return _T("TranparentWnd");
}

void CTranparentWnd::OnClick( TNotifyUI &msg )
{
	CDuiString sName = msg.pSender->GetName();
	sName.MakeLower();
}

LRESULT CTranparentWnd::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
 {
	 bHandled = FALSE;
	 return 0;
 }

LRESULT CTranparentWnd::ResponseDefaultKeyEvent(WPARAM wParam) {
	if (wParam == VK_ESCAPE) {
		Close();
		return TRUE;
	}
	else if (wParam == VK_RETURN) {
		return TRUE;
	}
	return FALSE;
}

LRESULT CTranparentWnd::OnSysCommand( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled )
{
	bHandled = FALSE;
	return 0L;
}

void CTranparentWnd::InitWindow()
{
	m_pCef = static_cast<CCefUI*>(m_PaintManager.FindControl(TEXT("cef")));

	if (m_pCef) {
		m_pCef->SetUrl((ppx::base::GetCurrentProcessDirectoryW() +  TEXT("../../../test-resource/lrc.html")).c_str());
	}
}
