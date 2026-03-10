cmake_minimum_required(VERSION 3.15)
project(monk VERSION 0.2.4 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

option(MONK_PYTHON_BINDINGS "Build Python extension module" OFF)

if(BUILD_TESTING)
  find_package(GTest REQUIRED)
  include(GoogleTest)
  enable_testing()
  set(MONK_TEST_DATA_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/test-file.MWF")
endif()

add_subdirectory(core)

if(MONK_PYTHON_BINDINGS)
  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 REQUIRED)
  find_package(Python3 REQUIRED COMPONENTS Development)
  add_subdirectory(bindings)
endif()

# Install CMake package config files so downstream projects can use find_package(monk)
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MonkConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/MonkConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/monk
)

write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/MonkConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/MonkConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/MonkConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/monk
)
