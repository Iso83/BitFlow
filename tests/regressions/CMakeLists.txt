# ============================================================================
# Target
# ============================================================================

string(REPLACE "/" "_" MODULE_NAME "regression")

set(MODULE_TARGET "${PROJECT_NAME}_${MODULE_NAME}")

# ============================================================================
# Tests
# ============================================================================

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_ArithmeticChains
    TARGET
        ${PROJECT_NAME}::engine_io
    MODULE
        "regressions"
    SOURCES
        tests/Arithmetic_Chains.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_ExprPrinter
    TARGET
        ${PROJECT_NAME}::engine_io
    MODULE
        "regressions"
    SOURCES
        tests/ExprPrinter.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_RewriteStress
    TARGET
        ${PROJECT_NAME}::engine_io
    MODULE
        "regressions"
    SOURCES
        tests/RewriteStress.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_RuleDocumentation
    TARGET
        ${PROJECT_NAME}::engine_io
    MODULE
        "regressions"
    SOURCES
        tests/RuleDocumentation.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
