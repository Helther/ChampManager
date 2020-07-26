cmake_minimum_required(VERSION 3.1)

project(ChampManager LANGUAGES CXX)
include(cmake/StandardProjectSettings.cmake)
# TODO: use this lib
# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)

# TODO: fix sanitizers
# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" ON)

if(ENABLE_TESTING)
  enable_testing()
  message(
    "Building Tests."
  )
  add_subdirectory(tests)
endif()
add_subdirectory(src)
