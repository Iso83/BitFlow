# ============================================================================
# Source files
# ============================================================================

file(GLOB_RECURSE MODULE_SRC CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE MODULE_PRIVATE_HEADERS CONFIGURE_DEPENDS
    src/*.h
)

file(GLOB_RECURSE MODULE_PUBLIC_HEADERS CONFIGURE_DEPENDS
    include/*.h
)

# ============================================================================
# Target
# ============================================================================

string(REPLACE "/" "_" MODULE_NAME "engine/core")

set(MODULE_TARGET "${PROJECT_NAME}_${MODULE_NAME}")

add_library(${MODULE_TARGET}
    ${MODULE_SRC}
    ${MODULE_PRIVATE_HEADERS}
    ${MODULE_PUBLIC_HEADERS}
)

add_library(${PROJECT_NAME}::${MODULE_NAME} ALIAS ${MODULE_TARGET})

# ============================================================================
# Build configuration
# ============================================================================

cppcmake_version_target(${MODULE_TARGET})
cppcmake_helper_include_target(${MODULE_TARGET})

target_include_directories(${MODULE_TARGET}
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
    PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_features(${MODULE_TARGET}
    PUBLIC
        cxx_std_20
)

if(${PROJECT_NAME}_EXPR_LIFETIME_CHECKS)
    target_compile_definitions(${MODULE_TARGET}
        PUBLIC ${PROJECT_NAME}_EXPR_LIFETIME_CHECKS
    )
endif()

if(${PROJECT_NAME}_RULE_STRICT_REWRITE)
    target_compile_definitions(${MODULE_TARGET}
        PUBLIC ${PROJECT_NAME}_RULE_STRICT_REWRITE
    )
endif()

if(${PROJECT_NAME}_TESTS_CATCH_EXCEPTIONS)
    target_compile_definitions(${MODULE_TARGET}
        PUBLIC ${PROJECT_NAME}_TESTS_CATCH_EXCEPTIONS
    )
endif()

# ============================================================================
# IDE organization
# ============================================================================

cppcmake_project_set_ide_folder(${MODULE_TARGET} "engine/core")

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}/engine/core"
    PREFIX "Include"
    FILES
        ${MODULE_PUBLIC_HEADERS}
)

source_group(
    TREE "${CMAKE_CURRENT_SOURCE_DIR}/src"
    PREFIX "Source"
    FILES
        ${MODULE_SRC}
        ${MODULE_PRIVATE_HEADERS}
)

# ============================================================================
# Dependencies
# ============================================================================

cppcmake_helper_include_target(${MODULE_TARGET})

# ============================================================================
# Tests
# ============================================================================

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Bitvector
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/bitvector"
    SOURCES
        tests/bitvector/bitvector.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Evaluator
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/eval"
    SOURCES
        tests/eval/Evaluator.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Expr
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/expression"
    SOURCES
        tests/expression/Expr.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_ExprPrinter
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/expression"
    SOURCES
        tests/expression/ExprPrinter.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_ExprRef
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/expression"
    SOURCES
        tests/expression/ExprRef.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_ExprRefUtils
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/expression"
    SOURCES
        tests/expression/ExprRefUtils.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_ExprStore
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/expression"
    SOURCES
        tests/expression/ExprStore.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_ExprUtils
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/expression"
    SOURCES
        tests/expression/ExprUtils.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_OpType
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/expression"
    SOURCES
        tests/expression/OpType.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_RuleEngine
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules"
    SOURCES
        tests/rules/RuleEngine.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_RulePipeline
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules"
    SOURCES
        tests/rules/RulePipeline.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_aCommonFactor
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/arithmetic"
    SOURCES
        tests/rules/factorize/arithmetic/CommonFactor.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_DifferenceOfSquares
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/arithmetic"
    SOURCES
        tests/rules/factorize/arithmetic/DifferenceOfSquares.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_DivFractionDenominator
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/arithmetic"
    SOURCES
        tests/rules/factorize/arithmetic/DivFractionDenominator.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_MulFractionNumerator
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/arithmetic"
    SOURCES
        tests/rules/factorize/arithmetic/MulFractionNumerator.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_PerfectSquare
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/arithmetic"
    SOURCES
        tests/rules/factorize/arithmetic/PerfectSquare.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Absorb
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/bitwise"
    SOURCES
        tests/rules/factorize/bitwise/Absorb.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_bCommonFactor
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/bitwise"
    SOURCES
        tests/rules/factorize/bitwise/CommonFactor.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_DeMorgan
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/bitwise"
    SOURCES
        tests/rules/factorize/bitwise/DeMorgan.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Distribute
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/factorize/bitwise"
    SOURCES
        tests/rules/factorize/bitwise/Distribute.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Flatten
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/normalize"
    SOURCES
        tests/rules/normalize/Flatten.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Order
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/normalize"
    SOURCES
        tests/rules/normalize/Order.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_AddNegToSub
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/normalize/arithmetic"
    SOURCES
        tests/rules/normalize/arithmetic/AddNegToSub.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_SubToNeg
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/normalize/arithmetic"
    SOURCES
        tests/rules/normalize/arithmetic/SubToNeg.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Rotate
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/normalize/bitwise"
    SOURCES
        tests/rules/normalize/bitwise/Rotate.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_CombineConstants
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/arithmetic"
    SOURCES
        tests/rules/simplify/arithmetic/CombineConstants.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_aFold
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/arithmetic"
    SOURCES
        tests/rules/simplify/arithmetic/Fold.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_NegNeg
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/arithmetic"
    SOURCES
        tests/rules/simplify/arithmetic/NegNeg.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_NegPowEven
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/arithmetic"
    SOURCES
        tests/rules/simplify/arithmetic/NegPowEven.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_One
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/arithmetic"
    SOURCES
        tests/rules/simplify/arithmetic/One.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_SubNeg
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/arithmetic"
    SOURCES
        tests/rules/simplify/arithmetic/SubNeg.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_aZero
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/arithmetic"
    SOURCES
        tests/rules/simplify/arithmetic/Zero.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_AndXorReduction
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/AndXorReduction.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Cancel_Integration
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/Cancel_Integration.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Cancel
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/Cancel.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Complement
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/Complement.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_bFold
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/Fold.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Idempotent
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/Idempotent.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_Not
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/Not.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_XorAndNotReduction
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/XorAndNotReduction.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_XorAndReduction
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/XorAndReduction.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)

cppcmake_test_add(
    NAME
        Test_${MODULE_TARGET}_bZero
    TARGET
        ${MODULE_TARGET}
    MODULE
        "engine/core/rules/simplify/bitwise"
    SOURCES
        tests/rules/simplify/bitwise/Zero.cpp
    LINK_LIBRARIES
        ${PROJECT_NAME}::tests_common
    INCLUDE_DIRECTORIES
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    COMPILE_DEFINITIONS
        "USE_CORE_PRINT"
)
