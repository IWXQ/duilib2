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

#ifndef PPX_CEF_REQUEST_CONTEXT_HANDLER_H__
#define PPX_CEF_REQUEST_CONTEXT_HANDLER_H__
#pragma once

#include "include/cef_request_context.h"
#include "include/cef_request_context_handler.h"

namespace DuiLib {
	namespace Internal {

		class RequestContextHandler : public CefRequestContextHandler {
		public:
			RequestContextHandler();

			bool OnBeforePluginLoad(const CefString& mime_type,
				const CefString& plugin_url,
				const CefString& top_origin_url,
				CefRefPtr<CefWebPluginInfo> plugin_info,
				PluginPolicy* plugin_policy) OVERRIDE;

		private:

			IMPLEMENT_REFCOUNTING(RequestContextHandler);
			DISALLOW_COPY_AND_ASSIGN(RequestContextHandler);
		};
	}
}

#endif // !PPX_CEF_REQUEST_CONTEXT_HANDLER_H__