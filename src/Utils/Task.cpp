#include "StdAfx.h"
#include "Utils/Task.h"

namespace DuiLib {
    bool IsInUIThread() {
        return GetCurrentThreadId() == CPaintManagerUI::GetUIThreadId();
    }

    void PostTaskToUIThread(ppx::base::Closure c) {
        ppx::base::Closure * p = new ppx::base::Closure;
        *p = c;
        PostThreadMessage(CPaintManagerUI::GetUIThreadId(), UIMSG_THREADMSG, (WPARAM)p, UIMSG_THREADMSG);
    }

    bool PostTaskWhenNotInUIThread(ppx::base::Closure c) {
        if (IsInUIThread())
            return false;
        PostTaskToUIThread(c);
        return true;
    }
}