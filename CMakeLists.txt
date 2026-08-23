cmake_minimum_required(VERSION 4.0)

project(BitFlow
    VERSION 0.1.1
    LANGUAGES C CXX
)

# =========================================================
# Project configuration
# =========================================================

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

option(
    ${PROJECT_NAME}_EXPR_LIFETIME_CHECKS
    "Enable Expr lifetime validation instrumentation"
    OFF
)

option(
    ${PROJECT_NAME}_RULE_STRICT_REWRITE
    "Enable strict rule rewrite validation checks"
    OFF
)

option(
    ${PROJECT_NAME}_TESTS_CATCH_EXCEPTIONS
    "Catch exceptions inside BF_SAFE_REWRITE"
    ON
)

option(${PROJECT_NAME}_BUILD_CLI 
    "Build BitFlow CLI"
    OFF
)

# =========================================================
# CppCMake
# =========================================================

include(cmake/Dependencies.cmake)

dependencies_fetch_cppcmake()

cppcmake_project_initialize()

if(CPPCMAKE_PROJECT_ALREADY_REGISTERED)
    return()
endif()

cppcmake_githook_precommit()
cppcmake_version_generate()
cppcmake_test_setup()

# =========================================================
# Dependencies
# =========================================================

cppcmake_dependency_try_local_packages()

# =========================================================
# Modules
# =========================================================

if(${PROJECT_NAME}_BUILD_TESTS)
    cppcmake_module_add(tests/common)
    cppcmake_module_add(tests/regressions)
endif()

cppcmake_module_add(engine/core)
cppcmake_module_add(engine/io)
cppcmake_module_add(engine/functions)

if(${PROJECT_NAME}_BUILD_CLI)
    cppcmake_module_add(app/cli)
endif()