function(bf_add_module_test target folder name)
    add_executable(${name} tests/${name}.cpp)

    target_link_libraries(${name} PRIVATE ${target})
    target_include_directories(${name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests/common
    )

    add_test(NAME ${name} COMMAND ${name})

    set_property(TARGET ${name} PROPERTY FOLDER "tests/${folder}")
endfunction()