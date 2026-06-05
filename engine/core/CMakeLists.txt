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

set_target_properties(BitFlowCore PROPERTIES
    POSITION_INDEPENDENT_CODE ON
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
if(BF_RULE_STRICT_REWRITE)
    target_compile_definitions(BitFlowCore PUBLIC BF_RULE_STRICT_REWRITE)
endif()

if(BF_TEST_CATCH_EXCEPTIONS)
    target_compile_definitions(BitFlowCore PUBLIC BF_TEST_CATCH_EXCEPTIONS)
endif()

if(BF_BUILD_TESTS)
    bf_add_module_test(BitFlowCore core/bitvector BF_BitVector USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/eval BF_Evaluator USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/expression BF_Expr USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/expression BF_ExprPrinter USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/expression BF_ExprRef USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/expression BF_ExprRefUtils USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/expression BF_ExprStore USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/expression BF_OpType USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/expression BF_ExprUtils USE_CORE_PRINT)


    bf_add_module_test(BitFlowCore core/rules BF_Rule USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules BF_RuleEngine USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules BF_RulePipeline USE_CORE_PRINT)
    
    
    bf_add_module_test(BitFlowCore core/rules/factorize/arithmetic BF_Factorize_Arithmetic_CommonFactor USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/arithmetic BF_Factorize_Arithmetic_PerfectSquare USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/arithmetic BF_Factorize_Arithmetic_DifferenceOfSquares USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/arithmetic BF_Factorize_Arithmetic_MulFractionNumerator USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/arithmetic BF_Factorize_Arithmetic_DivFractionDenominator USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Absorb USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_CommonFactor USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Distribute USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Normalize_Flatten USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Normalize_Order USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/rules/normalize/arithmetic BF_Rules_Normalize_AddNegToSub USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/normalize/arithmetic BF_Rules_Normalize_SubToNeg USE_CORE_PRINT)
    
    bf_add_module_test(BitFlowCore core/rules/normalize/bitwize BF_Normalize_Bitwise_Rotate USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_CombineConstants USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Fold USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_NegNeg USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_NegPowEven USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_One USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_SubNeg USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Zero USE_CORE_PRINT)

    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_AndXorReduction USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Cancel USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Cancel_Integration USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Complement USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Fold USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Idempotent USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Not USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_XorAndReduction USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_XorAndNotReduction USE_CORE_PRINT)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Zero USE_CORE_PRINT)

endif()


set(BITFLOW_DOC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/../../docs/core/core_architecture.md
    ${CMAKE_CURRENT_SOURCE_DIR}/../../docs/core/operator\ precedence.md
    ${CMAKE_CURRENT_SOURCE_DIR}/../../AGENTS.md
)

target_sources(BitFlowCore PRIVATE
    ${BITFLOW_DOC_FILES}
)

if(MSVC)
    source_group("doc"
        FILES ${BITFLOW_DOC_FILES}
    )
endif()