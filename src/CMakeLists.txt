add_library(opp
  serverboards.cpp
  opp.cpp process.cpp symbol.cpp vm.cpp io.cpp logger.cpp task.cpp
)

target_link_libraries(opp ${CONAN_LIBS})
