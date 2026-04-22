cmake_minimum_required(VERSION 3.20)
project(SHA_Steps_Demo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(ShaStepsDemo
    src/main.cpp
    src/ShaExprBuilder.h
    src/ShaDemoSupport.h
)

set_property(TARGET ShaStepsDemo PROPERTY FOLDER "SHA Steps")

target_include_directories(ShaStepsDemo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(ShaStepsDemo PRIVATE BitFlowCore BitFlowIO)

if (TARGET CLI11::CLI11)
    target_link_libraries(ShaStepsDemo PRIVATE CLI11::CLI11)
    target_compile_definitions(ShaStepsDemo PRIVATE SHA_DEMO_HAS_CLI11=1)
elseif(TARGET CLI11)
    target_link_libraries(ShaStepsDemo PRIVATE CLI11)
    target_compile_definitions(ShaStepsDemo PRIVATE SHA_DEMO_HAS_CLI11=1)
endif()
