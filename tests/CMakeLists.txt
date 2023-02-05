find_package(Qt5 5.15 COMPONENTS Core Widgets Sql REQUIRED)
add_executable(test_parser_xml test_parser_xml.cpp ../src/parser.cpp)
target_link_libraries(test_parser_xml PRIVATE project_warnings project_options Qt5::Core Qt5::Widgets Qt5::Sql)

add_executable(test_parser_config test_parser_config.cpp ../src/parser.cpp)
target_link_libraries(test_parser_config PRIVATE project_warnings project_options Qt5::Core Qt5::Widgets Qt5::Sql)
