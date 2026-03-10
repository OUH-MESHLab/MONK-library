pybind11_add_module(monklib pybind.cpp)
target_link_libraries(monklib PRIVATE monk)

install(TARGETS monklib
  LIBRARY DESTINATION ${Python3_SITEARCH}
)
