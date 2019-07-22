/*******************************************************************************
* Copyright (C) 2018 - 2020, winsoft666, <winsoft666@outlook.com>.
*
* THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
* EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.
*
* Expect bugs
*
* Please use and enjoy. Please let me know of any bugs/improvements
* that you have found/implemented and I will fix/incorporate them into this
* file.
*******************************************************************************/
#ifndef DUILIB_GLOBAL_CONTEXT_H_
#define DUILIB_GLOBAL_CONTEXT_H_
#pragma once
#ifdef UILIB_WITH_CEF
#include "ppxbase/singleton.h"
#include "include/base/cef_scoped_ptr.h"
#include "include/base/cef_thread_checker.h"
#include "include/cef_app.h"

class CefGlobalContext : public ppx::base::Singleton<CefGlobalContext> {
public:
	void SetCefApp(CefRefPtr<CefApp> app);
	CefRefPtr<CefApp> GetCefApp() const;

	void SetWithCef(bool b);
	bool GetWithCef() const;

	void SetCefCache(bool b);
	bool GetCefCache() const;

	void SetUsingProxyServer(bool b);
	bool GetUsingProxyServer() const;
private:
	bool m_bWithCef;
	bool m_bCefCache;
	bool m_bUsingProxyServer;
	CefRefPtr<CefApp> m_pCefApp;
private:
	CefGlobalContext();
	~CefGlobalContext();

	SINGLETON_CLASS_DECLARE(CefGlobalContext);
};

#endif

#endif // !DUILIB_GLOBAL_CONTEXT_H_