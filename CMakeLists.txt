cmake_minimum_required(VERSION 3.20)
project(BitFlow LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(BF_EXPR_LIFETIME_CHECKS
    "Enable Expr lifetime validation instrumentation"
    OFF)
option(BF_TEST_CATCH_EXCEPTIONS
    "Catch exceptions inside BF_SAFE_REWRITE"
    ON)
option(BF_BUILD_TESTS 
    "Build tests" 
    ON)
option(BF_BUILD_CLI "Build BitFlow CLI" OFF)

if(BF_BUILD_TESTS)
    enable_testing()
    include(cmake/BitFlowTests.cmake)

    add_subdirectory(tests/regressions)
endif()

add_subdirectory(engine/core)
add_subdirectory(engine/io)
add_subdirectory(engine/functions)

if(BF_BUILD_CLI)
    add_subdirectory(cli)
endif()