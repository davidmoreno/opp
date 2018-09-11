add_definitions("-std=gnu++17 -rdynamic -I${CMAKE_SOURCE_DIR}/src")
add_executable(sbds serverboards.cpp sbds.cpp)
target_link_libraries(sbds ${CONAN_LIBS} opp -pthread)
