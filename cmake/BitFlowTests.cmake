function(bf_add_module_test target folder name)
    set(options USE_CORE_PRINT)
    set(oneValueArgs SOURCE)
    set(multiValueArgs LABELS)
    cmake_parse_arguments(BF_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(BF_TEST_SOURCE)
        set(test_source ${BF_TEST_SOURCE})
    else()
        set(test_source tests/${name}.cpp)
    endif()

    add_executable(${name} ${test_source})

    target_link_libraries(${name} PRIVATE ${target})
    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests/common
    )

    if(BF_TEST_USE_CORE_PRINT)
        target_compile_definitions(${name} PRIVATE USE_CORE_PRINT)
    endif()

    add_test(NAME ${name} COMMAND ${name})

    if(BF_TEST_LABELS)
        set_tests_properties(${name} PROPERTIES LABELS "${BF_TEST_LABELS}")
    else()
        set_tests_properties(${name} PROPERTIES LABELS "unit")
    endif()

    set_property(TARGET ${name} PROPERTY FOLDER "tests/${folder}")
endfunction()
