add_library(opp SHARED
  # this adds the basic core processes and vm
  $<TARGET_OBJECTS:opp_core>
  # and now the opp. The core can not access the opp.
  io.cpp poller.cpp logger.cpp task.cpp supervisor.cpp
  tcp_client.cpp
)

add_subdirectory(core)

target_link_libraries(opp ${CONAN_LIBS})
