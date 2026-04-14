file(GLOB_RECURSE BF_IO_SRC CONFIGURE_DEPENDS
    src/*.cpp
)

list(FILTER BF_IO_SRC EXCLUDE REGEX ".*/ExprConsoleTool\\.cpp$")

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
    bf_add_module_test(BitFlowIO io BF_ExprParser)
    bf_add_module_test(BitFlowIO io BF_ExprPrinter)
endif()

option(BF_BUILD_IO_TOOLS "Build BitFlow IO command line tools" OFF)
if(BF_BUILD_IO_TOOLS)
    add_executable(BitFlowIO_ExprConsoleTool src/ExprConsoleTool.cpp)
    target_link_libraries(BitFlowIO_ExprConsoleTool PRIVATE BitFlowIO)
    set_property(TARGET BitFlowIO_ExprConsoleTool PROPERTY FOLDER "tools/io")
endif()
