add_library(opp SHARED
  # this adds the basic core processes and vm
  $<TARGET_OBJECTS:opp_core>
  # and now the opp. The core can not access the opp.
  poller.cpp logger.cpp task.cpp supervisor.cpp
  io/io.cpp io/tcp/client.cpp io/tcp/server.cpp
)

add_subdirectory(core)

target_link_libraries(opp ${CONAN_LIBS})
