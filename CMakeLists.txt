cmake_minimum_required(VERSION 3.20)
project(BitFlow LANGUAGES C CXX)

option(BF_BUILD_TESTS "Build tests" ON)
option(BF_EXPR_LIFETIME_CHECKS "Enable Expr lifetime validation instrumentation" OFF)

if(BF_BUILD_TESTS)
    enable_testing()
    include(cmake/BitFlowTests.cmake)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_subdirectory(engine/core)
add_subdirectory(engine/io)