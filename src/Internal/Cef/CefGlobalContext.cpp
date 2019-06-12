#include "CefGloablContext.h"


CefGlobalContext::CefGlobalContext() :
	m_bCefCache(false)
{

}

CefGlobalContext::~CefGlobalContext() {

}

void CefGlobalContext::SetWithCef(bool b) {
	m_bWithCef = b;
}


bool CefGlobalContext::GetWithCef() const {
	return m_bWithCef;
}

void CefGlobalContext::SetCefApp(CefRefPtr<CefApp> app) {
	m_pCefApp = app;
}

CefRefPtr<CefApp> CefGlobalContext::GetCefApp() const {
	return m_pCefApp;
}

void CefGlobalContext::SetCefCache(bool b) {
	m_bCefCache = b;
}

bool CefGlobalContext::GetCefCache() const {
	return m_bCefCache;
}