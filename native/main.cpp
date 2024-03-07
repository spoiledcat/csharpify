#include "config.h"
#include "main.h"
#include "managed_exports.h"
#include "managed_bridge.h"
#include "imgui.h"
#include "imgui_internal.h"
// #include "cimgui.h"

// ImGuiContext *ctx;
// ImGuiMemAllocFunc alloc_func = nullptr;
// ImGuiMemFreeFunc free_func   = nullptr;
// void* user_data              = nullptr;

void MainLoop()
{
//    ctx = ImGui::GetCurrentContext();
//    ImGui::GetAllocatorFunctions(&alloc_func, &free_func, &user_data);

    OnUpdate();
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

	OnStart();
    auto data = new NativeData{42};
    //DeepClassNameCall(data);

    SomeCall(*data);

	return imgui_main(argc, argv);
}

