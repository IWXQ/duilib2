#include "Stdafx.h"
#include "Utils/SystemTrayIcon.h"
#include "ppxbase/process_util.h"
using namespace std;

namespace DuiLib {
#ifndef ASSERT
#include <assert.h>
#define ASSERT assert
#endif

#ifndef _countof
#define _countof(x) (sizeof(x)/sizeof(x[0]))
#endif

#define TRAYICON_CLASS _T("TrayIconClass")

	// The option here is to maintain a list of all TrayIcon windows,
	// and iterate through them, instead of only allowing a single 
	// TrayIcon per application
	CSystemTrayIcon* CSystemTrayIcon::m_pThis = NULL;

	const UINT CSystemTrayIcon::m_nTimerID = 4567;
	UINT CSystemTrayIcon::m_nMaxTooltipLength = 64;     // This may change...
	const UINT CSystemTrayIcon::m_nTaskbarCreatedMsg = ::RegisterWindowMessage(_T("TaskbarCreated"));
	HWND  CSystemTrayIcon::m_hWndInvisible;

	CSystemTrayIcon::CSystemTrayIcon() {
		Initialise();
	}

	CSystemTrayIcon::CSystemTrayIcon(HINSTANCE hInst,			// Handle to application instance
		HWND hParent,				// The window that will recieve tray notifications
		UINT uCallbackMessage,     // the callback message to send to parent
		LPCTSTR szToolTip,         // tray icon tooltip
		HICON icon,                // Handle to icon
		UINT uID,                  // Identifier of tray icon
		BOOL bHidden /*=FALSE*/,   // Hidden on creation?                  
		LPCTSTR szBalloonTip /*=NULL*/,
		LPCTSTR szBalloonTitle /*=NULL*/, 
		DWORD dwBalloonIcon /*=NIIF_NONE*/,
		UINT uBalloonTimeout /*=10*/)
	{
		Initialise();
		Create(hInst, hParent, uCallbackMessage, szToolTip, icon, uID, bHidden,
			szBalloonTip, szBalloonTitle, dwBalloonIcon, uBalloonTimeout);
	}

	void CSystemTrayIcon::Initialise() {
		// If maintaining a list of all TrayIcon windows (instead of
		// only allowing a single TrayIcon per application) then add
		// this TrayIcon to the list
		m_pThis = this;

		memset(&m_tnd, 0, sizeof(m_tnd));
		m_bHidden = TRUE;
		m_bRemoved = TRUE;

		m_DefaultMenuItemID = 0;
		m_DefaultMenuItemByPos = TRUE;

		m_bShowIconPending = FALSE;

		m_uIDTimer = 0;
		m_hSavedIcon = NULL;

		m_hTargetWnd = NULL;
		m_uCreationFlags = 0;
	}

	ATOM CSystemTrayIcon::RegisterClass(HINSTANCE hInstance) {
		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcex.lpfnWndProc = (WNDPROC)WindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = 0;
		wcex.hCursor = 0;
		wcex.hbrBackground = 0;
		wcex.lpszMenuName = 0;
		wcex.lpszClassName = TRAYICON_CLASS;
		wcex.hIconSm = 0;

		return RegisterClassEx(&wcex);
	}

	BOOL CSystemTrayIcon::Create(HINSTANCE hInst, HWND hParent, UINT uCallbackMessage,
		LPCTSTR szToolTip, HICON icon, UINT uID, BOOL bHidden /*=FALSE*/,
		LPCTSTR szBalloonTip /*=NULL*/,
		LPCTSTR szBalloonTitle /*=NULL*/,
		DWORD dwBalloonIcon /*=NIIF_NONE*/,
		UINT uBalloonTimeout /*=10*/) {

		m_nMaxTooltipLength = _countof(m_tnd.szTip);

		// Tray only supports tooltip text up to m_nMaxTooltipLength) characters
		ASSERT(_tcslen(szToolTip) <= m_nMaxTooltipLength);

		m_hInstance = hInst;

		RegisterClass(hInst);

		// Create an invisible window
		m_hWnd = ::CreateWindow(TRAYICON_CLASS, _T(""), WS_POPUP,
			CW_USEDEFAULT, CW_USEDEFAULT,
			CW_USEDEFAULT, CW_USEDEFAULT,
			NULL, 0,
			hInst, 0);

		ppx::base::UIPIMsgFilter(m_hWnd, m_nTaskbarCreatedMsg, TRUE);

		// load up the NOTIFYICONDATA structure
		m_tnd.cbSize = sizeof(NOTIFYICONDATA);
		m_tnd.hWnd = (hParent) ? hParent : m_hWnd;
		m_tnd.uID = uID;
		m_tnd.hIcon = icon;
		m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		m_tnd.uCallbackMessage = uCallbackMessage;

		wcsncpy_s(m_tnd.szTip, szToolTip, m_nMaxTooltipLength);

		if (szBalloonTip) {
			// The balloon tooltip text can be up to 255 chars long.
			ASSERT(lstrlen(szBalloonTip) < 256);

			// The balloon title text can be up to 63 chars long.
			if (szBalloonTitle) {
				ASSERT(lstrlen(szBalloonTitle) < 64);
			}

			// dwBalloonIcon must be valid.
			ASSERT(NIIF_NONE == dwBalloonIcon || NIIF_INFO == dwBalloonIcon ||
				NIIF_WARNING == dwBalloonIcon || NIIF_ERROR == dwBalloonIcon);

			// The timeout must be between 10 and 30 seconds.
			ASSERT(uBalloonTimeout >= 10 && uBalloonTimeout <= 30);

			m_tnd.uFlags |= NIF_INFO;

			_tcsncpy(m_tnd.szInfo, szBalloonTip, 255);
			if (szBalloonTitle)
				_tcsncpy(m_tnd.szInfoTitle, szBalloonTitle, 63);
			else
				m_tnd.szInfoTitle[0] = _T('\0');
			m_tnd.uTimeout = uBalloonTimeout * 1000; // convert time to ms
			m_tnd.dwInfoFlags = dwBalloonIcon;
		}


		m_bHidden = bHidden;
		m_hTargetWnd = m_tnd.hWnd;

		if (m_bHidden) {
			m_tnd.uFlags = NIF_STATE;
			m_tnd.dwState = NIS_HIDDEN;
			m_tnd.dwStateMask = NIS_HIDDEN;
		}

		m_uCreationFlags = m_tnd.uFlags;	// Store in case we need to recreate in OnTaskBarCreate

		BOOL bResult = TRUE;
		if (!m_bHidden) {
			bResult = Shell_NotifyIcon(NIM_ADD, &m_tnd);
			m_bShowIconPending = m_bHidden = m_bRemoved = !bResult;
		}

		if (szBalloonTip) {
			// Zero out the balloon text string so that later operations won't redisplay the balloon.
			m_tnd.szInfo[0] = _T('\0');
		}

		return bResult;
	}

	BOOL CSystemTrayIcon::Create(HWND hParent, UINT uCallbackMessage,
		LPCTSTR szTip, UINT icon, UINT uID, BOOL bHidden /*= FALSE*/, LPCTSTR szBalloonTip /*= NULL*/,
		LPCTSTR szBalloonTitle /*= NULL*/, DWORD dwBalloonIcon /*= NIIF_NONE*/, UINT uBalloonTimeout /*= 10*/) {
		HICON hIcon = (HICON) ::LoadImage(CPaintManagerUI::GetInstance(),
			MAKEINTRESOURCE(icon),
			IMAGE_ICON,
			0, 0,
			LR_DEFAULTCOLOR);

		BOOL bret = Create(CPaintManagerUI::GetInstance(), hParent, uCallbackMessage, szTip, hIcon, 
			uID, bHidden, szBalloonTip, szBalloonTitle, dwBalloonIcon, uBalloonTimeout);
		::DestroyIcon(hIcon);

		return bret;
	}

	CSystemTrayIcon::~CSystemTrayIcon() {
		RemoveIcon();
		m_IconList.clear();
		if (m_hWnd)
			::DestroyWindow(m_hWnd);
	}

	void CSystemTrayIcon::SetFocus() {
		Shell_NotifyIcon(NIM_SETFOCUS, &m_tnd);
	}


	BOOL CSystemTrayIcon::AddIcon() {
		if (!m_bRemoved)
			RemoveIcon();

		m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		if (!Shell_NotifyIcon(NIM_ADD, &m_tnd))
			m_bShowIconPending = TRUE;
		else
			m_bRemoved = m_bHidden = FALSE;

		return (m_bRemoved == FALSE);
	}

	BOOL CSystemTrayIcon::RemoveIcon() {
		m_bShowIconPending = FALSE;

		if (m_bRemoved)
			return TRUE;

		m_tnd.uFlags = 0;
		if (Shell_NotifyIcon(NIM_DELETE, &m_tnd))
			m_bRemoved = m_bHidden = TRUE;

		return (m_bRemoved == TRUE);
	}

	BOOL CSystemTrayIcon::HideIcon() {
		if (m_bRemoved || m_bHidden)
			return TRUE;

		m_tnd.uFlags = NIF_STATE;
		m_tnd.dwState = NIS_HIDDEN;
		m_tnd.dwStateMask = NIS_HIDDEN;

		m_bHidden = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);

		return (m_bHidden == TRUE);
	}

	BOOL CSystemTrayIcon::ShowIcon() {
		if (m_bRemoved)
			return AddIcon();

		if (!m_bHidden)
			return TRUE;

		m_tnd.uFlags = NIF_STATE;
		m_tnd.dwState = 0;
		m_tnd.dwStateMask = NIS_HIDDEN;
		Shell_NotifyIcon(NIM_MODIFY, &m_tnd);

		return (m_bHidden == FALSE);
	}

	BOOL CSystemTrayIcon::SetIcon(HICON hIcon) {
		m_tnd.uFlags = NIF_ICON;
		m_tnd.hIcon = hIcon;

		if (m_bHidden)
			return TRUE;
		else
			return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
	}

	BOOL CSystemTrayIcon::SetIcon(LPCTSTR lpszIconName) {
		HICON hIcon = (HICON) ::LoadImage(m_hInstance,
			lpszIconName,
			IMAGE_ICON,
			0, 0,
			LR_LOADFROMFILE);

		if (!hIcon)
			return FALSE;
		BOOL returnCode = SetIcon(hIcon);
		::DestroyIcon(hIcon);
		return returnCode;
	}

	BOOL CSystemTrayIcon::SetIcon(UINT nIDResource) {
		HICON hIcon = (HICON) ::LoadImage(m_hInstance,
			MAKEINTRESOURCE(nIDResource),
			IMAGE_ICON,
			0, 0,
			LR_DEFAULTCOLOR);

		BOOL returnCode = SetIcon(hIcon);
		::DestroyIcon(hIcon);
		return returnCode;
	}

	BOOL CSystemTrayIcon::SetStandardIcon(LPCTSTR lpIconName) {
		HICON hIcon = ::LoadIcon(NULL, lpIconName);

		return SetIcon(hIcon);
	}

	BOOL CSystemTrayIcon::SetStandardIcon(UINT nIDResource) {
		HICON hIcon = ::LoadIcon(NULL, MAKEINTRESOURCE(nIDResource));

		return SetIcon(hIcon);
	}

	HICON CSystemTrayIcon::GetIcon() const {
		return m_tnd.hIcon;
	}

	BOOL CSystemTrayIcon::SetIconList(UINT uFirstIconID, UINT uLastIconID) {
		if (uFirstIconID > uLastIconID)
			return FALSE;

		UINT uIconArraySize = uLastIconID - uFirstIconID + 1;

		m_IconList.clear();
		try {
			for (UINT i = uFirstIconID; i <= uLastIconID; i++)
				m_IconList.push_back(::LoadIcon(m_hInstance, MAKEINTRESOURCE(i)));
		}
		catch (...) {
			m_IconList.clear();
			return FALSE;
		}

		return TRUE;
	}

	BOOL CSystemTrayIcon::SetIconList(HICON* pHIconList, UINT nNumIcons) {
		m_IconList.clear();

		try {
			for (UINT i = 0; i <= nNumIcons; i++)
				m_IconList.push_back(pHIconList[i]);
		}
		catch (...) {
			m_IconList.clear();
			return FALSE;
		}

		return TRUE;
	}

	BOOL CSystemTrayIcon::Animate(UINT nDelayMilliSeconds, int nNumSeconds /*=-1*/) {
		if (m_IconList.empty())
			return FALSE;

		StopAnimation();

		m_nCurrentIcon = 0;
		time(&m_StartTime);
		m_nAnimationPeriod = nNumSeconds;
		m_hSavedIcon = GetIcon();

		// Setup a timer for the animation
		m_uIDTimer = ::SetTimer(m_hWnd, m_nTimerID, nDelayMilliSeconds, NULL);
		return (m_uIDTimer != 0);
	}

	BOOL CSystemTrayIcon::StepAnimation() {
		if (!m_IconList.size())
			return FALSE;

		m_nCurrentIcon++;
		if (m_nCurrentIcon >= m_IconList.size())
			m_nCurrentIcon = 0;

		return SetIcon(m_IconList[m_nCurrentIcon]);
	}

	BOOL CSystemTrayIcon::StopAnimation() {
		BOOL bResult = FALSE;

		if (m_uIDTimer)
			bResult = ::KillTimer(m_hWnd, m_uIDTimer);
		m_uIDTimer = 0;

		if (m_hSavedIcon)
			SetIcon(m_hSavedIcon);
		m_hSavedIcon = NULL;

		return bResult;
	}

	/////////////////////////////////////////////////////////////////////////////
	// CSystemTray tooltip text manipulation

	BOOL CSystemTrayIcon::SetTooltipText(LPCTSTR pszTip) {
		ASSERT(_tcslen(pszTip) < m_nMaxTooltipLength);

		m_tnd.uFlags = NIF_TIP;
		_tcsncpy(m_tnd.szTip, pszTip, m_nMaxTooltipLength - 1);

		if (m_bHidden)
			return TRUE;
		else
			return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
	}

	BOOL CSystemTrayIcon::SetTooltipText(UINT nID) {
		TCHAR strBuffer[1024];
		ASSERT(1024 >= m_nMaxTooltipLength);

		if (!LoadString(m_hInstance, nID, strBuffer, m_nMaxTooltipLength - 1))
			return FALSE;

		return SetTooltipText(strBuffer);
	}

	LPTSTR CSystemTrayIcon::GetTooltipText() const {
		static TCHAR strBuffer[1024];
		ASSERT(1024 >= m_nMaxTooltipLength);

		wcsncpy_s(strBuffer, 1024, m_tnd.szTip, m_nMaxTooltipLength - 1);

		return strBuffer;
	}


	BOOL CSystemTrayIcon::ShowBalloon(LPCTSTR szText,
		LPCTSTR szTitle  /*=NULL*/,
		DWORD   dwIcon   /*=NIIF_NONE*/,
		UINT    uTimeout /*=10*/) {

		// The balloon tooltip text can be up to 255 chars long.
		ASSERT(lstrlen(szText) < 256);

		// The balloon title text can be up to 63 chars long.
		if (szTitle) {
			ASSERT(lstrlen(szTitle) < 64);
		}

		// dwBalloonIcon must be valid.
		ASSERT(NIIF_NONE == dwIcon || NIIF_INFO == dwIcon ||
			NIIF_WARNING == dwIcon || NIIF_ERROR == dwIcon);

		// The timeout must be between 10 and 30 seconds.
		//ASSERT(uTimeout >= 10 && uTimeout <= 30);


		m_tnd.uFlags = NIF_INFO;
		_tcsncpy(m_tnd.szInfo, szText, 256);
		if (szTitle)
			_tcsncpy(m_tnd.szInfoTitle, szTitle, 64);
		else
			m_tnd.szInfoTitle[0] = _T('\0');
		m_tnd.dwInfoFlags = dwIcon;
		m_tnd.uTimeout = uTimeout * 1000;   // convert time to ms

		BOOL bSuccess = Shell_NotifyIcon(NIM_MODIFY, &m_tnd);

		// Zero out the balloon text string so that later operations won't redisplay the balloon.
		m_tnd.szInfo[0] = _T('\0');

		return bSuccess;
	}

	void CSystemTrayIcon::RemoveBalloon()
	{
		m_tnd.uFlags = NIF_INFO;
		m_tnd.szInfo[0] = 0;
		m_tnd.szInfoTitle[0] = 0;

		m_tnd.dwInfoFlags = NIIF_NONE;

		Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
	}

	BOOL CSystemTrayIcon::SetNotificationWnd(HWND hNotifyWnd) {
		// Make sure Notification window is valid
		if (!hNotifyWnd || !::IsWindow(hNotifyWnd)) {
			ASSERT(FALSE);
			return FALSE;
		}

		m_tnd.hWnd = hNotifyWnd;
		m_tnd.uFlags = 0;

		if (m_bHidden)
			return TRUE;
		else
			return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
	}

	HWND CSystemTrayIcon::GetNotificationWnd() const {
		return m_tnd.hWnd;
	}

	// Change or retrive the window to send menu commands to
	BOOL CSystemTrayIcon::SetTargetWnd(HWND hTargetWnd) {
		m_hTargetWnd = hTargetWnd;
		return TRUE;
	}

	HWND CSystemTrayIcon::GetTargetWnd() const {
		if (m_hTargetWnd)
			return m_hTargetWnd;
		else
			return m_tnd.hWnd;
	} 


	BOOL CSystemTrayIcon::SetCallbackMessage(UINT uCallbackMessage) {
		// Make sure we avoid conflict with other messages
		ASSERT(uCallbackMessage >= WM_APP);

		m_tnd.uCallbackMessage = uCallbackMessage;
		m_tnd.uFlags = NIF_MESSAGE;

		if (m_bHidden)
			return TRUE;
		else
			return Shell_NotifyIcon(NIM_MODIFY, &m_tnd);
	}

	UINT CSystemTrayIcon::GetCallbackMessage() const {
		return m_tnd.uCallbackMessage;
	}


	BOOL CSystemTrayIcon::SetMenuDefaultItem(UINT uItem, BOOL bByPos) {
		if ((m_DefaultMenuItemID == uItem) && (m_DefaultMenuItemByPos == bByPos))
			return TRUE;

		m_DefaultMenuItemID = uItem;
		m_DefaultMenuItemByPos = bByPos;

		HMENU hMenu = ::LoadMenu(m_hInstance, MAKEINTRESOURCE(m_tnd.uID));
		if (!hMenu)
			return FALSE;

		HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
		if (!hSubMenu) {
			::DestroyMenu(hMenu);
			return FALSE;
		}

		::SetMenuDefaultItem(hSubMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);

		::DestroyMenu(hSubMenu);
		::DestroyMenu(hMenu);

		return TRUE;
	}

	void CSystemTrayIcon::GetMenuDefaultItem(UINT& uItem, BOOL& bByPos) {
		uItem = m_DefaultMenuItemID;
		bByPos = m_DefaultMenuItemByPos;
	}

	LRESULT CSystemTrayIcon::OnTimer(UINT nIDEvent) {
		if (nIDEvent != m_uIDTimer) {
			ASSERT(FALSE);
			return 0L;
		}

		time_t CurrentTime;
		time(&CurrentTime);

		time_t period = CurrentTime - m_StartTime;
		if (m_nAnimationPeriod > 0 && m_nAnimationPeriod < period) {
			StopAnimation();
			return 0L;
		}

		StepAnimation();

		return 0L;
	}

	// This is called whenever the taskbar is created (eg after explorer crashes
	// and restarts. Please note that the WM_TASKBARCREATED message is only passed
	// to TOP LEVEL windows (like WM_QUERYNEWPALETTE)
	LRESULT CSystemTrayIcon::OnTaskbarCreated(WPARAM wParam, LPARAM lParam) {
		InstallIconPending();
		return 0L;
	}


	LRESULT CSystemTrayIcon::OnSettingChange(UINT uFlags, LPCTSTR lpszSection) {
		if (uFlags == SPI_SETWORKAREA)
			InstallIconPending();
		return 0L;
	}

	LRESULT CSystemTrayIcon::OnTrayNotification(UINT wParam, LONG lParam) {
		//Return quickly if its not for this tray icon
		if (wParam != m_tnd.uID)
			return 0L;

		HWND hTargetWnd = GetTargetWnd();
		if (!hTargetWnd)
			return 0L;

		// Clicking with right button brings up a context menu
		if (LOWORD(lParam) == WM_RBUTTONUP)
		{
			HMENU hMenu = ::LoadMenu(m_hInstance, MAKEINTRESOURCE(m_tnd.uID));
			if (!hMenu)
				return 0;

			HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
			if (!hSubMenu) {
				::DestroyMenu(hMenu);        //Be sure to Destroy Menu Before Returning
				return 0;
			}


			// Make chosen menu item the default (bold font)
			::SetMenuDefaultItem(hSubMenu, m_DefaultMenuItemID, m_DefaultMenuItemByPos);

			CustomizeMenu(hSubMenu);

			// Display and track the popup menu
			POINT pos;
			GetCursorPos(&pos);

			::SetForegroundWindow(m_tnd.hWnd);
			::TrackPopupMenu(hSubMenu, 0, pos.x, pos.y, 0, hTargetWnd, NULL);

			// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
			::PostMessage(m_tnd.hWnd, WM_NULL, 0, 0);

			DestroyMenu(hMenu);
		}
		else if (LOWORD(lParam) == WM_LBUTTONDBLCLK)
		{
			// double click received, the default action is to execute default menu item
			::SetForegroundWindow(m_tnd.hWnd);

			UINT uItem;
			if (m_DefaultMenuItemByPos) {
				HMENU hMenu = ::LoadMenu(m_hInstance, MAKEINTRESOURCE(m_tnd.uID));
				if (!hMenu)
					return 0;

				HMENU hSubMenu = ::GetSubMenu(hMenu, 0);
				if (!hSubMenu)
					return 0;
				uItem = ::GetMenuItemID(hSubMenu, m_DefaultMenuItemID);

				DestroyMenu(hMenu);
			}
			else
				uItem = m_DefaultMenuItemID;

			::PostMessage(hTargetWnd, WM_COMMAND, uItem, 0);
		}

		return 1;
	}

	// This is the global (static) callback function for all TrayIcon windows
	LRESULT PASCAL CSystemTrayIcon::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
		// The option here is to maintain a list of all TrayIcon windows,
		// and iterate through them. If you do this, remove these 3 lines.
		CSystemTrayIcon* pTrayIcon = m_pThis;
		if (pTrayIcon->GetSafeHwnd() != hWnd)
			return ::DefWindowProc(hWnd, message, wParam, lParam);


		if (message == CSystemTrayIcon::m_nTaskbarCreatedMsg)
			return pTrayIcon->OnTaskbarCreated(wParam, lParam);

		// Animation timer
		if (message == WM_TIMER && wParam == pTrayIcon->GetTimerID())
			return pTrayIcon->OnTimer(wParam);

		// Settings changed
		if (message == WM_SETTINGCHANGE && wParam == pTrayIcon->GetTimerID())
			return pTrayIcon->OnSettingChange(wParam, (LPCTSTR)lParam);

		// Is the message from the icon for this TrayIcon?
		if (message == pTrayIcon->GetCallbackMessage())
			return pTrayIcon->OnTrayNotification(wParam, lParam);

		// Message has not been processed, so default.
		return ::DefWindowProc(hWnd, message, wParam, lParam);
	}

	void CSystemTrayIcon::InstallIconPending() {
		// Is the icon display pending, and it's not been set as "hidden"?
		if (!m_bShowIconPending || m_bHidden)
			return;

		// Reset the flags to what was used at creation
		m_tnd.uFlags = m_uCreationFlags;

		// Try and recreate the icon
		m_bHidden = !Shell_NotifyIcon(NIM_ADD, &m_tnd);

		// If it's STILL hidden, then have another go next time...
		m_bShowIconPending = !m_bHidden;

		ASSERT(m_bHidden == FALSE);
	}

	BOOL CALLBACK FindTrayWnd(HWND hwnd, LPARAM lParam) {
		TCHAR szClassName[256];
		GetClassName(hwnd, szClassName, 255);

		// Did we find the Main System Tray? If so, then get its size and keep going
		if (_tcscmp(szClassName, _T("TrayNotifyWnd")) == 0) {
			LPRECT lpRect = (LPRECT)lParam;
			::GetWindowRect(hwnd, lpRect);
			return TRUE;
		}

		// Did we find the System Clock? If so, then adjust the size of the rectangle
		// we have and quit (clock will be found after the system tray)
		if (_tcscmp(szClassName, _T("TrayClockWClass")) == 0) {
			LPRECT lpRect = (LPRECT)lParam;
			RECT rectClock;
			::GetWindowRect(hwnd, &rectClock);
			// if clock is above system tray adjust accordingly
			if (rectClock.bottom < lpRect->bottom - 5) // 10 = random fudge factor.
				lpRect->top = rectClock.bottom;
			else
				lpRect->right = rectClock.left;
			return FALSE;
		}

		return TRUE;
	}

	// Check to see if the animation has been disabled (Matthew Ellis <m.t.ellis@bigfoot.com>)
	BOOL CSystemTrayIcon::GetDoWndAnimation() {
		ANIMATIONINFO ai;

		ai.cbSize = sizeof(ai);
		SystemParametersInfo(SPI_GETANIMATION, sizeof(ai), &ai, 0);

		return ai.iMinAnimate ? TRUE : FALSE;
	}

	BOOL CSystemTrayIcon::RemoveTaskbarIcon(HWND hWnd) {
		if (!::IsWindow(m_hWndInvisible)) {
			m_hWndInvisible = CreateWindowEx(0, _T("Static"), _T(""), WS_POPUP,
				CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
				NULL, 0, NULL, 0);

			if (!m_hWndInvisible)
				return FALSE;
		}

		SetParent(hWnd, m_hWndInvisible);

		return TRUE;
	}
}