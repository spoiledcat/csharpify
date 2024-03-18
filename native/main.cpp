#include "config.h"
#include "common.h"
#include "utils.h"

#include "managed_exports.h"
#include "bridge.h"
#include "main.h"


void MainLoop() {
    OnUpdate();
}

int main(int argc, char** argv) {

    // the first call to a managed function will initialize the runtime, but it can also be initialized
    // ahead of time
    // load_managed_runtime();

    OnStart();

    return imgui_main(argc, argv);
	//return 0;
}


DNNE_EXTERN_C
bool CallToNative() {
    return true;
}
