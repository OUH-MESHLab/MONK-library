cmake_minimum_required(VERSION 3.8...3.18)
cmake_policy(SET CMP0148 OLD)
project(monklib)


# Set C++ standard to 17
set(CMAKE_CXX_STANDARD 17 CACHE STRING "C++ version selection")

# Use RPATH for installed targets during build
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

# Enable position independent code
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Enable generation of python binding
option(PYTHON_BINDINGS "Enables the build" OFF)

# Enable testing
if(BUILD_TESTING)
  find_package(GTest REQUIRED)

  # Enable Google Test macros
  include(GoogleTest)

  # Define the test data file
  set(TEST_DATA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/test-file.MWF")

  enable_testing()
endif()

if(PYTHON_BINDINGS)
  find_package(pybind11 REQUIRED)
  find_package(Python3 REQUIRED COMPONENTS Development)
  add_subdirectory(bindings)
endif()

# Include core library
add_subdirectory(core)
