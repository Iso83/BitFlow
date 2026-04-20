function(bf_add_module_test target folder name)
    set(options)
    set(oneValueArgs)
    set(multiValueArgs LABELS)
    cmake_parse_arguments(BF_TEST "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    add_executable(${name} tests/${name}.cpp)

    target_link_libraries(${name} PRIVATE ${target})
    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests/common
    )

    add_test(NAME ${name} COMMAND ${name})
    if(BF_TEST_LABELS)
        set_tests_properties(${name} PROPERTIES LABELS "${BF_TEST_LABELS}")
    endif()

    set_property(TARGET ${name} PROPERTY FOLDER "tests/${folder}")
endfunction()
