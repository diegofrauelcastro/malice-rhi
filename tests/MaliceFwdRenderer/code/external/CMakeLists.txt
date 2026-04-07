cmake_minimum_required(VERSION 3.16)

project(imgui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(IMGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/imgui)

set(IMGUI_SOURCES
    # Base
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_demo.cpp

    # Backends
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
)

find_package(Vulkan REQUIRED)
# Link libraries.
target_link_libraries(${PROJECT_NAME}
    PUBLIC
    volk
    glfw
)

# Macro definitions.
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    GLFW_INCLUDE_NONE
    IMGUI_IMPL_VULKAN_USE_VOLK
)

add_library(imgui::imgui ALIAS imgui)
