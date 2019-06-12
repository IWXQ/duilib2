#ifndef DUILIB2_GLOBAL_CONTEXT_H_
#define DUILIB2_GLOBAL_CONTEXT_H_
#pragma once

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
private:
	bool m_bWithCef;
	bool m_bCefCache;
	CefRefPtr<CefApp> m_pCefApp;
private:
	CefGlobalContext();
	~CefGlobalContext();

	SINGLETON_CLASS_DECLARE(CefGlobalContext);
};



#endif // !DUILIB2_GLOBAL_CONTEXT_H_