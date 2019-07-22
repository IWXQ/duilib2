#include "StdAfx.h"
#include "Utils/Task.h"

namespace DuiLib {
    bool IsInUIThread() {
        return GetCurrentThreadId() == CPaintManagerUI::GetUIThreadId();
    }

    void PostTaskToUIThread(ppx::base::Closure c) {
		assert(CPaintManagerUI::GetUIThreadId() > 0);
		if (CPaintManagerUI::GetUIThreadId() > 0) {
			ppx::base::Closure * p = new ppx::base::Closure;
			*p = c;
			PostThreadMessage(CPaintManagerUI::GetUIThreadId(), UIMSG_THREADMSG, (WPARAM)p, UIMSG_THREADMSG);
		}
    }
}