#include "config.h"
#include "common.h"
#include "utils.h"

#include "managed_exports.h"
#include "bridge.h"


void MainLoop() {
    OnUpdate();
}

DNNE_EXTERN_C
int CallingBackToNativeLand(int number) {
    return number + 1;
}

DNNE_EXTERN_C
void send_utf16(uint16_t* str) {
	std::u16string u16{(char16_t*)str};
    auto ret = UnicodeToString(u16);
	printf("%s\n", ret.c_str());
}

DNNE_EXTERN_C void send_utf8(char* str) {
	printf("%s\n", str);
}


int main(int argc, char** argv) {

    // the first call to a managed function will initialize the runtime, but it can also be initialized
    // ahead of time
    // load_managed_runtime();

    OnStart();

    // auto data = new NativeData{42};

    // SomeCall(*data);

    //return imgui_main(argc, argv);
	return 0;
}

