#include "managed_exports.h"
#include "main.h"

#include <cstdio>

void Example1()
{
	struct MyData d;
	d.field1 = 2;
	d.field2 = 8;
	printf("Before: %d", d.field1);
	UpdateStructField(&d);
	printf("After: %d", d.field1);
}


// Called from imgui.gen.cpp on every frame.
void MainLoop() {
	// Call the C# method
	OnUpdate();
}

// This is called from C#
DNNE_EXTERN_C
bool ReturningBool() {
	return false;
}


// We start here
int main(int argc, char** argv) {
	// the first call to a managed function will initialize the runtime, but it can also be initialized
	// ahead of time with this call.
	// load_managed_runtime();

	// This is a C# method.
	OnStart();

	// Start up the imgui loop in imgui.gen.cpp
	return imgui_main(argc, argv);
}


