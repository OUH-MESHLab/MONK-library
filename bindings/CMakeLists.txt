pybind11_add_module(${PROJECT_NAME} pybind.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE corelib)
