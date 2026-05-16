function(bf_add_module_test target folder name)
    set(options USE_CORE_PRINT)
    set(oneValueArgs SOURCE)
    cmake_parse_arguments(BF_TEST "${options}" "${oneValueArgs}" "" ${ARGN})

    # source
    set(test_source "${BF_TEST_SOURCE}")
    if(NOT test_source)
        set(test_source "tests/${name}.cpp")
    endif()

    add_executable(${name} ${test_source})

    target_link_libraries(${name} PRIVATE ${target})

    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/tests/common
    )

    if(BF_TEST_USE_CORE_PRINT)
        target_compile_definitions(${name} PRIVATE USE_CORE_PRINT)
    endif()

    add_test(NAME ${name} COMMAND ${name})

    set_property(TARGET ${name} PROPERTY FOLDER "tests/${folder}")
endfunction()