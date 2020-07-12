set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Qt5 5.14 COMPONENTS Core Widgets Sql REQUIRED)
add_executable(ChampManager
  main.cpp
  parser.cpp
  dbhelper.cpp
  resultswindow.cpp
  resultswindow.ui
  )

target_compile_features(ChampManager PRIVATE cxx_std_17)
set_project_warnings(ChampManager)

target_link_libraries(ChampManager Qt5::Core Qt5::Widgets Qt5::Sql)
