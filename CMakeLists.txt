cmake_minimum_required(VERSION 3.8)
project(LMStudio)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_definitions(-DGLM_ENABLE_EXPERIMENTAL -DIMGUI_DEFINE_MATH_OPERATORS)

include_directories(lib/glm) #?
include_directories(lib/imgui)
add_subdirectory(lib)

file(GLOB_RECURSE LMMODELVIEW_SRC
    "lib/bStream/bstream.cpp"
    "src/*.cpp"
    "src/*.c"
    "include/*.hpp"
    "include/*.h"

    "lib/ImGuiFileDialog/ImGuiFileDialog/*.h"
    "lib/imgui/misc/cpp/*.cpp"
)

add_executable(LMStudio ${LMMODELVIEW_SRC})
target_include_directories(LMStudio PUBLIC include include/util lib/glm lib/glad/include lib/bStream lib/ImGuiFileDialog/ImGuiFileDialog lib/glfw/include lib/gctoolsplusplus/include lib/TriStripper/include)

target_link_libraries(LMStudio PUBLIC imgui ImGuiFileDialog glm glfw gctools++ TriStripper)
