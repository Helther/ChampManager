cmake_minimum_required(VERSION 3.12)

set(project_name ChampManager)
project(${project_name} LANGUAGES CXX)
include(cmake/StandardProjectSettings.cmake)

#set(CMAKE_VERBOSE_MAKEFILE ON)

# Link this 'library' to set the c++ standard / compile-time options requested
add_library(project_options INTERFACE)
target_compile_features(project_options INTERFACE cxx_std_17)

# Link this 'library' to use the warnings specified in CompilerWarnings.cmake
add_library(project_warnings INTERFACE)

# standard compiler warnings
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

# sanitizer options if supported by compiler
include(cmake/Sanitizers.cmake)
enable_sanitizers(project_options)

find_package(Qt5 5.15 COMPONENTS Core Widgets Sql REQUIRED)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
include_directories(src)
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
file(GLOB_RECURSE HEADERS ${SRC_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${SRC_DIR}/*.cpp)
set(QT_CUSTOM src/mainwindow.ui
	rsc/cm.qrc)
add_executable(${project_name} ${SOURCES} ${HEADERS} ${QT_CUSTOM})
target_link_libraries(${project_name} PRIVATE project_options project_warnings Qt5::Core Qt5::Widgets Qt5::Sql)

option(ENABLE_TESTING "Enable Test Builds" ON)

if(ENABLE_TESTING)
  enable_testing()
  set(TESTDATA_PATHS "${CMAKE_CURRENT_SOURCE_DIR}")
  string(APPEND TESTDATA_PATHS "/tests/testsdata/")
  add_compile_definitions( TESTDATA_PATH="${TESTDATA_PATHS}" )
  message(
    "Building Tests."
  )
  add_subdirectory(tests)
  add_test(ParserTestXML tests/test_parser_xml 1)
  add_test(ParserTestConfig tests/test_parser_config 2)
endif()
