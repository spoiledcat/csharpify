#include "config.h"
#include "main.h"
#include "managed_exports.h"
#include "managed_bridge.h"
#include "imgui.h"
#include "imgui_internal.h"


void MainLoop() {
    OnUpdate();
}

DNNE_EXTERN_C
int CallingBackToNativeLand(int number) {
    return number + 1;
}

int main(int argc, char** argv) {

    // the first call to a managed function will initialize the runtime, but it can also be initialized
    // ahead of time
    // load_managed_runtime();

//    CallingBackToNativeLand_ptr = CallingBackToNativeLand;
//    register_icall("Program::CallingBackToNativeLand", (const void*)CallingBackToNativeLand_ptr);

    //printf("Ready to attach");
    //while( !::IsDebuggerPresent() )
    //::Sleep( 100 );

    OnStart();

    auto data = new NativeData{42};
    //DeepClassNameCall(data);

    SomeCall(*data);

    return imgui_main(argc, argv);
}

