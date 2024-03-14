#ifndef __CSHARPIFY_MONO_BRIDGE_H__
#define __CSHARPIFY_MONO_BRIDGE_H__

#include "config.h"

#if RUNTIME_MONO

#include "coreclr_delegates.h"
#include "coreclrhost.h"
#include "runtime.h"

static MonoCoreRuntimeProperties monovm_core_properties = {
        nullptr,
        nullptr,
        nullptr,
        nullptr
};

#endif

#endif