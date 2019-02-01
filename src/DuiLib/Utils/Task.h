#ifndef __THREAD_UI_TASK_H__
#define __THREAD_UI_TASK_H__
#pragma once
#include "base/bind.h"
#include "base/callback.h"

namespace DuiLib {
    UILIB_API bool IsInUIThread();
  
    UILIB_API void PostTaskToUIThread(ppx::base::Closure c);

    // Helper function.
    //
    UILIB_API bool PostTaskWhenNotInUIThread(ppx::base::Closure c);
}

#endif // !__THREAD_UI_TASK_H__