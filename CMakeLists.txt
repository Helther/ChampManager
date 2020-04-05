cmake_minimum_required(VERSION 3.10)

# Set the project name to your project name, my project isn't very descriptive
project(ChampManager LANGUAGES CXX)
include(cmake/StandardProjectSettings.cmake)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
#target_compile_features(project_options INTERFACE cxx_std_17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

option(BUILD_SHARED_LIBS "Enable compilation of shared libraries" OFF)
option(ENABLE_TESTING "Enable Test Builds" OFF)

if(ENABLE_TESTING)
  enable_testing()
  message(
    "Building Tests."
  )
  #add_subdirectory(tests)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5Core)
add_executable(ChampManager
  main.cpp
  parser.cpp
)
target_link_libraries(ChampManager Qt5::Core)
