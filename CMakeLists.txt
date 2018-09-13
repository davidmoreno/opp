project(OPP)
cmake_minimum_required(VERSION 3.2)
add_definitions("-std=gnu++17 -rdynamic")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

include_directories(${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/src/core)

add_subdirectory(src)
add_subdirectory(test)
