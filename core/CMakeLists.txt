add_library(monk
  ByteVector.cpp
  DataStack.cpp
  FileManager.cpp
  MFERData.cpp
  MFERDataCollection.cpp
  NihonKohdenData.cpp
)

# Namespaced alias for use within the build tree and by downstream projects
add_library(Monk::monk ALIAS monk)

target_include_directories(monk PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/monk>
)

install(TARGETS monk
  EXPORT MonkTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/monk
  FILES_MATCHING PATTERN "*.h"
)

install(EXPORT MonkTargets
  FILE MonkTargets.cmake
  NAMESPACE Monk::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/monk
)

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()
