add_library(opp
  opp.cpp process.cpp io.cpp
  symbol.cpp vm.cpp io.cpp logger.cpp task.cpp
)

# add_definitions(-D__MESSAGES_DEBUG__)

target_link_libraries(opp ${CONAN_LIBS})
