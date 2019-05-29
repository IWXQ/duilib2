#ifndef __THREAD_UI_TASK_H__
#define __THREAD_UI_TASK_H__
#pragma once
#include "ppxbase/bind.h"
#include "ppxbase/callback.h"

#include <ppl.h>
#include <ppltasks.h>

namespace DuiLib {
    UILIB_API bool IsInUIThread();
  
    UILIB_API void PostTaskToUIThread(ppx::base::Closure c);
}

#endif // !__THREAD_UI_TASK_H__