cmake_minimum_required(VERSION 3.20)
project(BitFlowExprTreeExplorer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

if(NOT TARGET glfw)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        bfexpr_glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.3.9
    )
    FetchContent_MakeAvailable(bfexpr_glfw)
endif()

FetchContent_Declare(
    bfexpr_imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG 60d7fb207eeb46d6363dd4bde10b35991bae0ce7
)
FetchContent_MakeAvailable(bfexpr_imgui)

add_library(BFExpr_ImGui STATIC
    ${bfexpr_imgui_SOURCE_DIR}/imgui.cpp
    ${bfexpr_imgui_SOURCE_DIR}/imgui_draw.cpp
    ${bfexpr_imgui_SOURCE_DIR}/imgui_tables.cpp
    ${bfexpr_imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${bfexpr_imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${bfexpr_imgui_SOURCE_DIR}/backends/imgui_impl_opengl2.cpp
)

target_include_directories(BFExpr_ImGui PUBLIC
    ${bfexpr_imgui_SOURCE_DIR}
    ${bfexpr_imgui_SOURCE_DIR}/backends
)

target_link_libraries(BFExpr_ImGui PUBLIC glfw)
set_property(TARGET BFExpr_ImGui PROPERTY FOLDER "Proj/third_party/imgui")

find_package(OpenGL REQUIRED)

add_executable(BitFlowExprTreeExplorer
    src/main.cpp
)

set_property(TARGET BitFlowExprTreeExplorer PROPERTY FOLDER "Proj/Tools")

target_link_libraries(BitFlowExprTreeExplorer PRIVATE
    BitFlowCore
    BitFlowIO
    BFExpr_ImGui
    glfw
    OpenGL::GL
)
