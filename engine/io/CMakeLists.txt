file(GLOB_RECURSE BF_IO_SRC CONFIGURE_DEPENDS
    src/*.cpp
)


file(GLOB_RECURSE BF_IO_PRIVATE_HEADERS CONFIGURE_DEPENDS
    src/*.h
)

file(GLOB_RECURSE BF_IO_PUBLIC_HEADERS CONFIGURE_DEPENDS
    include/*.h
)

add_library(BitFlowIO
    ${BF_IO_SRC}
    ${BF_IO_PRIVATE_HEADERS}
    ${BF_IO_PUBLIC_HEADERS}
)

target_link_libraries(BitFlowIO PUBLIC 
    BitFlowCore
)

target_include_directories(BitFlowIO PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(BitFlowIO PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(BitFlowIO PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

set_property(TARGET BitFlowIO PROPERTY FOLDER "engine/io")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/BitFlow/io
    PREFIX "Include"
    FILES ${BF_IO_PUBLIC_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PREFIX "Source"
    FILES ${BF_IO_SRC} ${BF_IO_PRIVATE_HEADERS}
)

if(BF_BUILD_TESTS)
    bf_add_module_test(BitFlowIO io BF_Lexer)
    bf_add_module_test(BitFlowIO io BF_ExprParser)
    bf_add_module_test(BitFlowIO io BF_ExprLatex)
endif()
