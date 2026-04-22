cmake_minimum_required(VERSION 3.20)
project(BitFlowExprTreeExplorer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(imgui CONFIG REQUIRED)

add_executable(BitFlowExprTreeExplorer
    src/main.cpp
)

set_property(TARGET BitFlowExprTreeExplorer PROPERTY FOLDER "Proj/Tools")

target_link_libraries(BitFlowExprTreeExplorer PRIVATE
    BitFlowCore
    BitFlowIO
    imgui::imgui
    glfw
    OpenGL::GL
)

target_compile_definitions(BitFlowExprTreeExplorer PRIVATE IMGUI_HAS_DOCK=1)
