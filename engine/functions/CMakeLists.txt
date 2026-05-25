file(GLOB_RECURSE BF_FUNCTIONS_SRC CONFIGURE_DEPENDS
    src/*.cpp
)


file(GLOB_RECURSE BF_FUNCTIONS_PRIVATE_HEADERS CONFIGURE_DEPENDS
    src/*.h
)

file(GLOB_RECURSE BF_FUNCTIONS_PUBLIC_HEADERS CONFIGURE_DEPENDS
    include/*.h
)

add_library(BitFlowFunctions
    ${BF_FUNCTIONS_SRC}
    ${BF_FUNCTIONS_PRIVATE_HEADERS}
    ${BF_FUNCTIONS_PUBLIC_HEADERS}
)

target_link_libraries(BitFlowFunctions PUBLIC 
    BitFlowCore
	BitFlowIO
)

target_include_directories(BitFlowFunctions PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(BitFlowFunctions PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(BitFlowFunctions PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

set_property(TARGET BitFlowFunctions PROPERTY FOLDER "engine/functions")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/BitFlow/functions
    PREFIX "Include"
    FILES ${BF_FUNCTIONS_PUBLIC_HEADERS}
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src
    PREFIX "Source"
    FILES ${BF_FUNCTIONS_SRC} ${BF_FUNCTIONS_PRIVATE_HEADERS}
)

if(BF_BUILD_TESTS)
    bf_add_module_test(BitFlowFunctions functions BF_Functions)
endif()
