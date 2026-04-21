cmake_minimum_required(VERSION 3.20)
project(SHA_Demo LANGUAGES CXX)

add_executable(ShaStepsDemoV2
    src/main.cpp
    src/ShaExprBuilder.h
    src/ShaDemoSupport.h
)

set_property(TARGET ShaStepsDemoV2 PROPERTY FOLDER "SHA Steps")

target_include_directories(ShaStepsDemoV2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if (TARGET CLI11::CLI11)
    target_link_libraries(ShaStepsDemoV2 PRIVATE CLI11::CLI11)
elseif (TARGET CLI11)
    target_link_libraries(ShaStepsDemoV2 PRIVATE CLI11)
endif()

target_link_libraries(ShaStepsDemoV2 PRIVATE BitFlowCore BitFlowIO)

if (MSVC)
    target_compile_options(ShaStepsDemoV2 PRIVATE /W4 /permissive-)
else()
    target_compile_options(ShaStepsDemoV2 PRIVATE -Wall -Wextra -Wpedantic)
endif()
