project(OPP)
cmake_minimum_required(VERSION 2.8.12)
add_definitions("-std=gnu++17 -rdynamic")

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_executable(sbds sbds.cpp serverboards.cpp opp.cpp process.cpp symbol.cpp vm.cpp io.cpp)
target_link_libraries(sbds ${CONAN_LIBS} -pthread)
