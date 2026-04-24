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
    bf_add_module_test(BitFlowCore core BF_AST_OpTypeHelpers)
    bf_add_module_test(BitFlowCore core BF_Dedup)
    bf_add_module_test(BitFlowCore core BF_ExprIntern_KeyBuilder)
    bf_add_module_test(BitFlowCore core BF_RuleEngine_InPlaceRewrite)
    bf_add_module_test(BitFlowCore core BF_ConstantEval)
    bf_add_module_test(BitFlowCore core BF_ConstantDetect)
    bf_add_module_test(BitFlowCore core BF_RewriteCost)
    bf_add_module_test(BitFlowCore core BF_Codegen)
    bf_add_module_test(BitFlowCore core BF_SsaBuilder)
    bf_add_module_test(BitFlowCore core BF_PerfPass)
    bf_add_module_test(BitFlowCore core BF_Arithmetic_Integration)
    bf_add_module_test(BitFlowCore core BF_ArithmeticFactorizeMultiplicity)
    bf_add_module_test(BitFlowCore core BF_Arithmetic_CanonicalForms)
    bf_add_module_test(BitFlowCore core BF_Factorize_Arithmetic_Guards)
    bf_add_module_test(BitFlowCore core BF_Factorize_Arithmetic_Split)

    # Optional runtime/compiler-invoking tests (slow + toolchain dependent).
    if(BF_ENABLE_RUNTIME_COMPILE_TESTS)
        bf_add_module_test(BitFlowCore core BF_CodegenRuntime LABELS runtime_compile)
    endif()

    if(BF_ENABLE_FUZZ_PROPERTY_TESTS)
        bf_add_module_test(BitFlowCore core BF_FuzzEvalVsCodegen LABELS "fuzz;property")
    endif()

    bf_add_module_test(BitFlowCore core BF_CodegenMulti)
    bf_add_module_test(BitFlowCore core BF_CodegenStructuralCSE)
    bf_add_module_test(BitFlowCore core BF_BitVector)

    # Rules
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Order)
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Flatten)
    bf_add_module_test(BitFlowCore core/rules BF_Rules_Isolated SOURCE tests/rules/Rule_Isolated.cpp)
    bf_add_module_test(BitFlowCore core/rules BF_Rules_Dependencies SOURCE tests/rules/Rule_Dependencies.cpp)
    bf_add_module_test(BitFlowCore core/rules BF_RuleEngine_RewriteStability)
    bf_add_module_test(BitFlowCore core/pipeline BF_Pipeline_Profiles SOURCE tests/pipeline/Pipeline_Profiles.cpp)
    bf_add_module_test(BitFlowCore core/pipeline BF_Pipeline_NoOscillation SOURCE tests/pipeline/Pipeline_NoOscillation.cpp)
    bf_add_module_test(BitFlowCore core/pipeline BF_Pipeline_Validation SOURCE tests/pipeline/Pipeline_Validation.cpp)

    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Fold)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Cancel)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Not)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Complement)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Zero)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Dominance)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Idempotent)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_AndXorReduction)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_XorAndReduction)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_XorNotReduction)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Absorb)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Cancel)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_CommonFactor)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Distribute)

    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Fold)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Zero)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_One)

    bf_add_module_test(BitFlowCore core/rules/simplify BF_Rules_CH)
    bf_add_module_test(BitFlowCore core/rules/simplify BF_Rules_MAJ)

    # SHA workflow helpers/examples
    bf_add_module_test(BitFlowCore core BF_SHA_SimplifyRegression)
    bf_add_module_test(BitFlowCore core BF_SHA_HelperCoverage)
    bf_add_module_test(BitFlowCore core BF_SHA_Fragments)
    bf_add_module_test(BitFlowCore core BF_SHA_Workflows)
    bf_add_module_test(BitFlowCore core BF_SHA_SigmaCanonical)
    bf_add_module_test(BitFlowCore core BF_SHA_CanonicalForms)
    bf_add_module_test(BitFlowCore core BF_SHA_RewriteTargets)
    bf_add_module_test(BitFlowCore core BF_SHA_EndToEnd)
    bf_add_module_test(BitFlowCore core BF_SHA_RoundFragments)
endif()
