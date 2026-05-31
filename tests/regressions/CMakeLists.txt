add_library(BitFlowRegressions    
    RegressionsMain.cpp
)

target_link_libraries(BitFlowRegressions PUBLIC 
    BitFlowCore    
    BitFlowIO
)

set_target_properties(BitFlowRegressions PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

set_property(TARGET BitFlowRegressions PROPERTY FOLDER "tests/regressions")

if(BF_BUILD_TESTS)
    bf_add_module_test(BitFlowRegressions regressions BF_Reg_RewriteStress)
    bf_add_module_test(BitFlowRegressions regressions BF_Reg_ExprPrinter)
    bf_add_module_test(BitFlowRegressions regressions BF_Reg_Arithmetic_Chains)
    bf_add_module_test(BitFlowRegressions regressions BF_Reg_RuleDocumentation)
endif()
