
list(APPEND MONK_SOURCES
  ByteVector.cpp
  DataStack.cpp
  FileManager.cpp
  MFERData.cpp
  MFERDataCollection.cpp
  NihonKohdenData.cpp
)

# Add the library
add_library(corelib ${MONK_SOURCES})
target_include_directories(corelib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(PYTHON_BINDINGS)
 target_link_libraries(corelib PUBLIC pybind11::module pybind11::embed) # Link to pybind11
endif()

if(BUILD_TESTING)
  add_subdirectory(tests) # Add the tests
endif()
