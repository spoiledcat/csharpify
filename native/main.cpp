#include "config.h"
#include "main.h"
#include "managed_exports.h"
#include "managed_bridge.h"

void MainLoop()
{
}

int main(int argc, char** argv)
{
	load_managed_runtime();

	CallMe();
	CallMe2();

	return imgui_main(argc, argv);
}