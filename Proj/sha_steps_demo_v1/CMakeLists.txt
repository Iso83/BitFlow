add_executable(ShaStepsDemoV1
    src/main.cpp
    src/ShaExprBuilder.h
)

set_property(TARGET ShaStepsDemoV1 PROPERTY FOLDER "SHA Steps")

target_include_directories(ShaStepsDemoV1 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ShaStepsDemoV1 PRIVATE BitFlowCore BitFlowIO)
