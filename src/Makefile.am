IMGUI_DIR := $(top_srcdir)/imgui

BUILT_SOURCES = imgui.gen.cpp

bin_PROGRAMS = csharpify
csharpify_SOURCES = \
	main.cpp \
	imgui.gen.cpp \
	$(IMGUI_DIR)/imgui.cpp \
	$(IMGUI_DIR)/imgui_demo.cpp \
	$(IMGUI_DIR)/imgui_draw.cpp \
	$(IMGUI_DIR)/imgui_tables.cpp \
	$(IMGUI_DIR)/imgui_widgets.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_sdl2.cpp \
	$(IMGUI_DIR)/backends/imgui_impl_vulkan.cpp

AM_CPPFLAGS = -I$(IMGUI_DIR) -I$(IMGUI_DIR)/backends $(SDL2_CFLAGS) $(EXTRA1_CFLAGS)
AM_LDFLAGS = $(SDL2_LDFLAGS) $(MOLTENVK_LDFLAGS) $(EXTRA1_LDFLAGS) $(EXTRA2_LDFLAGS)

imgui.gen.cpp: $(IMGUI_DIR)/examples/example_sdl2_vulkan/main.cpp
	$(SED) 's/int main(int/int imgui_main(int/' $< > $@
	$(SED) -i '/ImGui::NewFrame();/,/ImGui::Render();/{/ImGui::NewFrame();/!{/ImGui::Render();/!d}}' $@
	$(SED) -i '/ImGui::NewFrame();/ a MainLoop();' $@
	$(SED) -i '1 i void MainLoop();' $@
		