#include "config.h"
#include "main.h"
#include "managed_exports.h"
#include "managed_bridge.h"

void MainLoop()
{
}

//static int (DNNE_CALLTYPE* CallingBackToNativeLand_ptr)(int);

DNNE_EXTERN_C
int CallingBackToNativeLand(int number)
{
    return number+1;
}

int main(int argc, char** argv)
{
	load_managed_runtime();
//    CallingBackToNativeLand_ptr = CallingBackToNativeLand;
//    register_icall("Program::CallingBackToNativeLand", (const void*)CallingBackToNativeLand_ptr);

	CallMe();
    auto data = new NativeData{42};
    DeepClassNameCall(data);

    SomeCall(*data);

	return imgui_main(argc, argv);
}

