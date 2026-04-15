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
    bf_add_module_test(BitFlowCore core BF_AST_OpTypeHelpers)
    bf_add_module_test(BitFlowCore core BF_Dedup)
    bf_add_module_test(BitFlowCore core BF_ExprIntern_KeyBuilder)
    bf_add_module_test(BitFlowCore core BF_RuleEngine_InPlaceRewrite)

    # Rules
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Order) 
    bf_add_module_test(BitFlowCore core/rules/normalize BF_Rules_Flatten)

    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Fold)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Cancel)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Not)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Complement)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Zero)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Dominance)
    bf_add_module_test(BitFlowCore core/rules/simplify/bitwise BF_Simplify_Bitwise_Idempotent)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Absorb)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Cancel)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_CommonFactor)
    bf_add_module_test(BitFlowCore core/rules/factorize/bitwise BF_Factorize_Bitwise_Distribute)

    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Fold)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_Zero)
    bf_add_module_test(BitFlowCore core/rules/simplify/arithmetic BF_Simplify_Arithmetic_One)

    bf_add_module_test(BitFlowCore core/rules/simplify BF_Rules_CH)
    bf_add_module_test(BitFlowCore core/rules/simplify BF_Rules_MAJ)
endif()
