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

if(BF_BUILD_TESTS)
    # Active always-on core test set.
    bf_add_module_test(BitFlowCore core BF_AST_OpTypeHelpers LABELS unit)
    bf_add_module_test(BitFlowCore core BF_Dedup LABELS unit)
    bf_add_module_test(BitFlowCore core BF_ExprIntern_KeyBuilder LABELS unit)
    bf_add_module_test(BitFlowCore core BF_RuleEngine_InPlaceRewrite LABELS unit)
    bf_add_module_test(BitFlowCore core BF_ConstantEval LABELS unit)
    bf_add_module_test(BitFlowCore core BF_ConstantDetect LABELS unit)
    bf_add_module_test(BitFlowCore core BF_Codegen LABELS unit)
    bf_add_module_test(BitFlowCore core BF_SsaBuilder LABELS unit)
    bf_add_module_test(BitFlowCore core BF_PerfPass LABELS unit)
    bf_add_module_test(BitFlowCore core BF_Arithmetic_Integration LABELS unit)

    # Optional runtime/compiler-invoking tests (slow + toolchain dependent).
    if(BF_ENABLE_RUNTIME_COMPILE_TESTS)
        bf_add_module_test(BitFlowCore core BF_CodegenRuntime LABELS runtime_compile)
    endif()

    if(BF_ENABLE_FUZZ_PROPERTY_TESTS)
        bf_add_module_test(BitFlowCore core BF_FuzzEvalVsCodegen LABELS "fuzz;property")
    endif()

    bf_add_module_test(BitFlowCore core BF_CodegenMulti LABELS unit)
    bf_add_module_test(BitFlowCore core BF_CodegenStructuralCSE LABELS unit)
    bf_add_module_test(BitFlowCore core BF_BitVector LABELS unit)

    # Rules
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Order LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Flatten LABELS unit)

    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Fold LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Cancel LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Not LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Complement LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Zero LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Dominance LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Idempotent LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_AndXorReduction LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_XorAndReduction LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_XorNotReduction LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Absorb LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Cancel LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_CommonFactor LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Distribute LABELS unit)

    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Fold LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Zero LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_One LABELS unit)

    bf_add_module_test(BitFlowCore core/rules/simplify BF_Rules_CH LABELS unit)
    bf_add_module_test(BitFlowCore core/rules/simplify BF_Rules_MAJ LABELS unit)
endif()
