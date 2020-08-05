set(TESTDATA_PATHS "${CMAKE_CURRENT_SOURCE_DIR}")
string(APPEND TESTDATA_PATHS "/testsdata/")
add_compile_definitions( TESTDATA_PATH="${TESTDATA_PATHS}" )

find_package(Qt5 5.14 COMPONENTS Core Widgets Sql REQUIRED)
add_executable(test_parser test_parser.cpp ../src/parser.cpp)
target_link_libraries(test_parser PRIVATE project_warnings project_options Qt5::Core Qt5::Widgets Qt5::Sql)
