# Including this cmake file will create a target for imgui
# Orion will use it's custom imgui backend.

# ImGui options
option(IMGUI_ENABLE_DEMO "Enable the demo functionality of imgui" ${ORION_ENGINE_DEVELOPER_MODE})

# Create library and add sources
add_library(imgui
        imgui/imconfig.h
        imgui/imgui.h imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_internal.h
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        imgui/misc/cpp/imgui_stdlib.h imgui/misc/cpp/imgui_stdlib.cpp
        $<IF:$<BOOL:${IMGUI_ENABLE_DEMO}>,imgui/imgui_demo.cpp,"">
)

# Add base component
target_link_libraries(imgui PUBLIC orion-base orion::math orion::utils)

# Set include directories
target_include_directories(imgui PUBLIC imgui)

# Create user imgui config file
set(imgui_config_file ${CMAKE_BINARY_DIR}/include/imconfig.h)
configure_file(imconfig.h.in ${imgui_config_file})
target_compile_definitions(imgui PUBLIC IMGUI_USER_CONFIG="${imgui_config_file}")
