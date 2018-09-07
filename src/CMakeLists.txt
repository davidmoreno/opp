add_library(opp
  serverboards.cpp
  opp.cpp process.cpp symbol.cpp vm.cpp io.cpp logger.cpp task.cpp
)

# add_definitions(-D__MESSAGES_DEBUG__)

target_link_libraries(opp ${CONAN_LIBS})
