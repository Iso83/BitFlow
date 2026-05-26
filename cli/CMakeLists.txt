file(GLOB_RECURSE BF_CLI_SRC CONFIGURE_DEPENDS
    *.cpp
)

file(GLOB_RECURSE BF_CLI_PRIVATE_HEADERS CONFIGURE_DEPENDS
    *.h
)

add_executable(BitFlow.CLI
    ${BF_CLI_SRC}
    ${BF_CLI_PRIVATE_HEADERS}
)

target_link_libraries(BitFlow.CLI PRIVATE 
    BitFlowCore
	BitFlowIO
)

set_target_properties(BitFlow.CLI PROPERTIES
    OUTPUT_NAME bitflow
    POSITION_INDEPENDENT_CODE ON
)

target_include_directories(BitFlow.CLI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set_target_properties(BitFlow.CLI PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

set_property(TARGET BitFlow.CLI PROPERTY FOLDER "cli")

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
    PREFIX "Source"
    FILES ${BF_CLI_SRC} ${BF_CLI_PRIVATE_HEADERS}
)


execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE BF_GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

if(NOT BF_GIT_HASH)
    set(BF_GIT_HASH "unknown")
endif()

target_compile_definitions(BitFlow.CLI PRIVATE
    BF_GIT_HASH="${BF_GIT_HASH}"
)
