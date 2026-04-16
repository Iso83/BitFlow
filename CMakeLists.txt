cmake_minimum_required(VERSION 3.20)
project(BitFlow LANGUAGES C CXX)

option(BF_BUILD_TESTS "Build tests" ON)

if(BF_BUILD_TESTS)
    option(BF_ENABLE_CODEGEN_RUNTIME_TESTS "Enable slow runtime compile tests" OFF)

    enable_testing()
    include(cmake/BitFlowTests.cmake)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# =========================================================
# Dependencies
# =========================================================
option(BF_ENABLE_FETCH "Enable FetchContent dependencies" ON)

if(BF_ENABLE_FETCH)
    include(FetchContent)

    FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.11.3
    )

    FetchContent_MakeAvailable(nlohmann_json)
else()
    set(FETCHCONTENT_FULLY_DISCONNECTED ON)
endif()

# ---------- Organize third party targets ----------

if(TARGET nlohmann_json)
    set_property(TARGET nlohmann_json PROPERTY FOLDER "third_party/json")
endif()

add_subdirectory(engine/core)
add_subdirectory(engine/io)

option(BF_BUILD_TOOLS "Build standalone BitFlow tools" OFF)
if(BF_BUILD_TOOLS)
    add_executable(BitFlow_ExprEvalTool tools/ExprEvalTool.cpp)
    target_link_libraries(BitFlow_ExprEvalTool PRIVATE BitFlowCore BitFlowIO)
    set_property(TARGET BitFlow_ExprEvalTool PROPERTY FOLDER "tools")
endif()
