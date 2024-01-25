SUBDIRS = managed
TOP=$(top_srcdir)

IMGUI_DIR := $(TOP)/imgui

BUILT_SOURCES = imgui.gen.cpp

bin_PROGRAMS = csharpify
csharpify_SOURCES = \
	main.cpp \
	managed/managed.g.c \
	imgui.gen.cpp \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_vulkan.cpp

AM_CPPFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends $(SDL2_CFLAGS) $(EXTRA1_CFLAGS) -DDNNE_COMPILE_AS_SOURCE -Imanaged
AM_LDFLAGS = $(SDL2_LDFLAGS) $(MOLTENVK_LDFLAGS) $(EXTRA1_LDFLAGS) $(EXTRA2_LDFLAGS)

if CORECLR_RUNTIME
AM_CPPFLAGS += -DCORECLR_RUNTIME
endif

AM_LDFLAGS += -L$(TOP)/packages/microsoft.netcore.app.runtime.osx-arm64/$(DOTNET_VERSION)/runtimes/osx-arm64/native -lcoreclr

imgui.gen.cpp: $(IMGUI_DIR)/examples/example_sdl2_vulkan/main.cpp
	$(SED) 's/int main(int/int imgui_main(int/' $< > $@
	$(SED) -i '/ImGui::NewFrame();/,/ImGui::Render();/{/ImGui::NewFrame();/!{/ImGui::Render();/!d}}' $@
	$(SED) -i '/ImGui::NewFrame();/ a MainLoop();' $@
	$(SED) -i '1 i void MainLoop();' $@

.stamp-rpath: $(bin_PROGRAMS)
	xcrun install_name_tool -add_rpath @executable_path/../packages/microsoft.netcore.app.runtime.osx-arm64/$(DOTNET_VERSION)/runtimes/osx-arm64/native $<
	$(Q) touch $@

all-local:: .stamp-rpath managed.dll

run: $(bin_PROGRAMS) managed.dll .stamp-rpath
	./$<

managed.dll: managed/bin/Debug/managed.dll
	+$(MAKE) -C $(SUBDIRS) $(MAKECMDGOALS)
	cp $< $@
