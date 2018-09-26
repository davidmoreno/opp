add_definitions("-std=gnu++17 -rdynamic -I${CMAKE_SOURCE_DIR}/src")

add_executable(sbds serverboards.cpp sbds.cpp)
target_link_libraries(sbds ${CONAN_LIBS} opp -pthread)

add_executable(test_tcp test_tcp.cpp)
target_link_libraries(test_tcp ${CONAN_LIBS} opp -pthread)

add_executable(startstop startstop.cpp)
target_link_libraries(startstop ${CONAN_LIBS} opp -pthread)
