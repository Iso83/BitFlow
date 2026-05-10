file(GLOB_RECURSE BF_CORE_SRC CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE BF_CORE_PRIVATE_HEADERS CONFIGURE_DEPENDS
    src/*.h
)

file(GLOB_RECURSE BF_CORE_PUBLIC_HEADERS CONFIGURE_DEPENDS
    include/*.h
)

add_library(BitFlowCore
    ${BF_CORE_SRC}
    ${BF_CORE_PRIVATE_HEADERS}
    ${BF_CORE_PUBLIC_HEADERS}
)

target_include_directories(BitFlowCore PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(BitFlowCore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_property(TARGET BitFlowCore PROPERTY FOLDER "engine/core")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/BitFlow/core
    PREFIX "Include"
    FILES ${BF_CORE_PUBLIC_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PREFIX "Source"
    FILES ${BF_CORE_SRC} ${BF_CORE_PRIVATE_HEADERS}
)

if(BF_EXPR_LIFETIME_CHECKS)
    target_compile_definitions(BitFlowCore PUBLIC BF_EXPR_LIFETIME_CHECKS)
endif()

if(BF_BUILD_TESTS)
    bf_add_module_test(BitFlowCore core BF_ExprRef USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core BF_Expr USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core BF_ExprUtils USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core BF_ExprStore USE_CORE_PRINT)

    # Active always-on core test set.
    bf_add_module_test(BitFlowCore core BF_OpType USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core BF_Dedup USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core BF_Evaluator USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core BF_Arithmetic_Integration USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core BF_ArithmeticFactorizeMultiplicity USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core BF_Arithmetic_CanonicalForms USE_CORE_PRINT)


    bf_add_module_test(BitFlowCore core BF_BitVector USE_CORE_PRINT)

    # Rules
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Order USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Flatten USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/rules/normalize/bitwize BF_Normalize_Bitwise_Rotate USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Fold USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Cancel USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Not USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Complement USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Zero USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Dominance USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Idempotent USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_AndXorReduction USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_XorAndReduction USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_XorNotReduction USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Absorb USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Cancel USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_CommonFactor USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Distribute USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Fold USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Zero USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_One USE_CORE_PRINT)
    
endif()
