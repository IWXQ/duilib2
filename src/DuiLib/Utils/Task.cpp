#include "StdAfx.h"
#include "Utils/Task.h"

namespace DuiLib {
    bool IsInUIThread() {
        return GetCurrentThreadId() == CPaintManagerUI::GetUIThreadId();
    }

    void PostTaskToUIThread(base::Closure c) {
        base::Closure * p = new base::Closure;
        *p = c;
        PostThreadMessage(CPaintManagerUI::GetUIThreadId(), UIMSG_THREADMSG, (WPARAM)p, UIMSG_THREADMSG);
    }

    bool PostTaskWhenNotInUIThread(base::Closure c) {
        if (IsInUIThread())
            return false;
        PostTaskToUIThread(c);
        return true;
    }
}