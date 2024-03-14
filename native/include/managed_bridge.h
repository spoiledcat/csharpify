#ifndef CSHARPIFY_MANAGED_BRIDGE_H_
#define CSHARPIFY_MANAGED_BRIDGE_H_

#include "common.h"

int load_managed_runtime();

int register_icall(const char* name, const void* fnptr);

#endif // CSHARPIFY_MANAGED_BRIDGE_H_