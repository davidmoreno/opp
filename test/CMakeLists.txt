add_definitions("-std=gnu++17 -rdynamic -I${CMAKE_SOURCE_DIR}/src")

add_executable(sbds serverboards.cpp sbds.cpp)
target_link_libraries(sbds ${CONAN_LIBS} opp -pthread)

add_executable(test_tcp test_tcp.cpp)
target_link_libraries(test_tcp ${CONAN_LIBS} opp -pthread)

add_executable(startstop startstop.cpp)
target_link_libraries(startstop ${CONAN_LIBS} opp -pthread)

add_executable(test_tcp_server test_tcp_server.cpp)
target_link_libraries(test_tcp_server ${CONAN_LIBS} opp -pthread)

add_executable(test_http_server test_http_server.cpp)
target_link_libraries(test_http_server ${CONAN_LIBS} opp -pthread)


add_executable(string_test string_test.cpp)
target_link_libraries(string_test ${CONAN_LIBS} opp -pthread)
