add_library(BitFlowREGRESSIONS
    RegressionsMain.cpp
)

target_link_libraries(BitFlowREGRESSIONS PUBLIC 
    BitFlowIO
)

set_target_properties(BitFlowREGRESSIONS PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

set_property(TARGET BitFlowREGRESSIONS PROPERTY FOLDER "tests/regressions")

if(BF_BUILD_TESTS)
    bf_add_module_test(BitFlowREGRESSIONS regressions BF_Reg_RewriteStress)
    bf_add_module_test(BitFlowREGRESSIONS regressions BF_Reg_ExprPrinter)
    bf_add_module_test(BitFlowREGRESSIONS regressions BF_Reg_Arithmetic_Chains)
endif()
